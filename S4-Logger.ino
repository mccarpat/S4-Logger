
#define VERSION "0.1.5"


/*
 *  Walrus
 *  
 *  --- Information -----------------
 *  EGR 699 Project Code
 *  Logs S4 data
 *  Patrick McCarthy (PJM)
 *  Sofia Fanourakis
 *  
 *  --- Changelog -------------------
 *  v0.1.5 - PJM -> Setting up states
 *  v0.1.3 - PJM -> Skeleton Structure
 *  v0.1.0 - PJM -> Initial Creation
 * 
 *  --- Pins ------------------------
 *  D12 / PB4 (MISO)    -> SDcard MISO
 *  D11 / PB3 (MOSI)    -> SDcard MOSI
 *  D13 / PB5  (SCK)    -> SDcard SCK
 *  PD4 /       (SS)    -> SDcard CS                  <- could/should change to D10/PB2, both code and circuit
 *  //PC4/A4 (SDA)  -> RTC SDA
 *  //PC5/A5 (SCL)  -> RTC SCL
 *  //D2/PD2        -> G of 33N10 (N-channel)
 *  //A3/PC3        -> Voltage at top of AA battery.
 *  //A2/PC2        -> Voltage at GND of AA battery.
 *  //D9 / PB1       -> Red LED
 *  D7 / PD7 / PCINT23       -> Red LED
 *  D3 / PD3       -> Green LED
 *  //PD6       -> Blue LED
 *  //D7 / PD7 / PCINT23       -> Button 1 
 *  D9 / PB1 / PCINT1       -> Button 1
 *  D8 / PB0 / PCINT0      -> Button 2
 *  A0  / PC0      -> Potentiometer
 *  
 *  
 *  
 *  --- Notes -----------------------
 *  - I don't know why I named it walrus.
 *  - Need to find two pins that can be pulled up/down to identify unique logging units
 * 
 * 
 */


// Initial definitions to set up environment
#define F_CPU 16000000UL // Set the CPU frequency of the board (16MHz board @ 5V)
#define VREF 5 // 5V operating voltage
//#define FACTORY_RESET // Not yet implemented
//#define LIGHTS_ON_BUTTONS // LEDS light up, red on buton 1, green on button 2 press.

// Options
#define START_PROGRAM_IN_THIS_STATE none
#define BOTH_PRESSED_IF_WITHIN 250 // 250 ms of each other, both buttons are considered pressed simultaneously if this is true.
#define EVENT_TEST_INTERVAL 50  // 50 ms
#define EVENT_BLINKLEDS_INTERVAL 500 // 500 ms
#define FLASH_LED_DURATION 500 // 500 ms
#define CHANGE_STATE_AFTER_THIS_DELAY 1000
#define MAIN_LOOP_INTERVAL 5 // 5ms in main interval
#define BUTTON_DEBOUNCE_INTERVAL 5 // repeat times * (main loop interval + 1ms between), so 30ms total for a debounced "press"
#define BUTTON_DEBOUNCE_REPEAT_TIMES 5
#define READ_DATA_INTERVAL 1000 // 1 second between successive reading of new data from the connected device (charge controller)
#define WDPS_4S     (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(0<<WDP0)
#define watchdog_clear_status()    MCUSR = 0  // Reset all statuses in the control register of the MCU
#define watchdog_feed()            wdt_reset()  // This entertains me


// Measured Variables
#define LoadResistance 0.010 // [Ohms]





// Pins
#define PIN_SPI_MISO B,4
#define PIN_SPI_MOSI B,3
#define PIN_SPI_SCK B,5
#define PIN_SPI_SS_SDCARD D,4  // Should change to B,2 eventually
//#define PIN_I2C_SDA C,4
//#define PIN_I2C_SCL C,5
//#define PIN_LED_Red B,1
#define PIN_LED_Red D,7
#define PIN_LED_Green D,3
//#define PIN_Button_1 D,7
#define PIN_Button_1 B,1
#define PIN_Button_2 B,0
#define PIN_Potentiometer C,0


// Functions via definition
#define LED_RED_OFF PIN_SET_LOW(PIN_LED_Red)
#define LED_RED_ON PIN_SET_HIGH(PIN_LED_Red)
#define LED_GREEN_OFF PIN_SET_LOW(PIN_LED_Green)
#define LED_GREEN_ON PIN_SET_HIGH(PIN_LED_Green)

//unsigned short test = 10000;

// Includes
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <util/delay.h>
#include <SPI.h>
#include <SD.h>
#include "lemtils/Pin.h"
#include "lemtils/Timer.h" // Includes functions for using the on-board timer. Allows delay(ms). REQUIRES: Timer_Timer1_Initialize(); (Or similar)
#include <avr/wdt.h>
#include "lemtils/ADC.h" // Includes functions for using the ADC. REQUIRES: ADC_Initialize(); ADC_SetAsInput(pin);
#include "lemtils/EventHandler.c"
#include <avr/interrupt.h>

//#include "lemtils/Terminal.h" // Facilitates terminal communication ( e.g. printf(), scanf(), puts() ) (BAUD 9600). REQUIRES: Terminal_Initialize();
//#include <errno.h>
//#include <avr/pgmspace.h>
//#include <avr/eeprom.h>





// DECLARATIONS:  States
typedef enum aSTATE {
  none,             //
  home,             //
  recording,
  sleep_until_next_recording,
  stop_recording,
  format_card
  //recording_sd_card_check,
  //recording_
  //error,             //
  //confirmed             //
} STATE_t;
STATE_t state, last_state, next_state;


// DECLARATIONS: Functions
void events_Tick();
void events_Handler( void ); // Handles the events; If an event is ready, perform appropriate actions.
void events_Handler_HighPriority( void ); // Handles high priority events (This is ran with the 1ms timer interrupt)
void state_Handler( void );
void watchdog_set( void );
void state_SetNext(STATE_t newstate);
void watchdog_entertain(void);
void events_Handler( void ); // Handles the events; If an event is ready, perform appropriate actions.
void events_Handler_HighPriority( void ); // Handles high priority events (This is ran with the 1ms timer interrupt)
void state_Home( void );
void state_Transition( void );
void state_Recording( void );
void event_Test_function( void );
void ReadDataFromDevice( void );
void StartRecording( void );
void ReadToConsoleFromFile( void );
void AppendToFile( void );
void InitializeSDCard( void );
void OpenAndWaitForSerialPort( void );
void ButtonHandler( void );
void BlinkLEDs( void );
void Green_LED_Start_Blinking( void );
void Red_LED_Start_Blinking( void );
void Green_LED_Stop_Blinking( void );
void Red_LED_Stop_Blinking( void );
void state_Sleep_until_next_recording( void );
void state_Stop_recording( void );
void Red_LED_Flash( void );
void Green_LED_Flash( void );
void Both_LED_Flash( void );
void ReadSDCardToConsole( void );
void state_Format_card( void );
//void event_RedLEDOff( void );
//void event_GreenLEDOff( void );


// DECLARATIONS: Variables
typedef uint8_t byte;
File myFile;
bool This_Is_A_Variable = true;   // This is nothing
uint8_t variable = 42;           // This is nothing
bool status_change = false;
volatile bool state_is_fresh = true;
bool Red_Is_On = false;
bool Green_Is_On = false;
volatile bool Button1_Was_Pressed_With_Interrupt = false;
volatile bool Button2_Was_Pressed_With_Interrupt = false;
byte Button1_Debounce_Times = 0;
byte Button2_Debounce_Times = 0;
bool Button1 = false; // Was Button 1 pressed and debounced? (1 = Pressed) (Note: Stays true after a press, must be manually set to false when the press is registered)
bool Button2 = false; // Was Button 2 pressed and debounced? (1 = Pressed) (Note: Stays true after a press, must be manually set to false when the press is registered)
bool Button2_Is_Actively_Pressed = false;
bool Button1_Is_Actively_Pressed = false;
//bool Button1_Press_Has_Been_Used = false;
//bool Button2_Press_Has_Been_Used = false;
unsigned long logentry = 0;
unsigned long time;
unsigned long Button1_HeldTime = 0;         // Resets to zero on button up
unsigned long Button2_HeldTime = 0;         // Resets to zero on button up
unsigned long Button1_HeldTime_Latest = 0;  // Does not reset on button up, will always be larger than 0
unsigned long Button2_HeldTime_Latest = 0;  // Does not reset on button up, will always be larger than 0
volatile bool ButtonActivity = false;
bool Button1P = false;
bool Button2P = false;
bool Red_LED_Blink_On = false;
bool Green_LED_Blink_On = false;
bool Should_I_Be_Sleeping = false;
int Red_FlashCountdown = 0;
int Green_FlashCountdown = 0;


// Create events and set up event ticker
struct an_event event_Test ;
struct an_event event_ChangeStateAfterDelay ;
struct an_event event_ReadDataFromDevice ;
struct an_event event_ButtonDebounce ;
struct an_event event_BlinkLEDs ;
struct an_event event_RedLEDOff ;
struct an_event event_GreenLEDOff ;



void setup() {

  watchdog_clear_status();
  watchdog_set();

  // Initialize events and start relevant ones
  event_Initialize(&event_Test,EVENT_TEST_INTERVAL);
  event_Initialize(&event_ChangeStateAfterDelay, CHANGE_STATE_AFTER_THIS_DELAY);
  event_Initialize(&event_ReadDataFromDevice,READ_DATA_INTERVAL);
  event_Initialize(&event_ButtonDebounce,BUTTON_DEBOUNCE_INTERVAL);
  event_Initialize(&event_BlinkLEDs,EVENT_BLINKLEDS_INTERVAL);
  event_Initialize(&event_RedLEDOff,FLASH_LED_DURATION);
  event_Initialize(&event_GreenLEDOff,FLASH_LED_DURATION);
 
  event_StartNow(&event_ReadDataFromDevice);
  event_StartNow(&event_Test); // Sets it at a count of zero and "is_planned" to true
  event_StartNow(&event_ButtonDebounce);
  event_StartNow(&event_BlinkLEDs);

  PIN_SET_AS_OUTPUT(PIN_SPI_SCK);
  PIN_SET_HIGH(PIN_SPI_SCK);
  PIN_SET_AS_OUTPUT(PIN_SPI_MOSI);
  PIN_SET_HIGH(PIN_SPI_MOSI);
  
  PIN_SET_AS_OUTPUT(PIN_LED_Red);
  PIN_SET_AS_OUTPUT(PIN_LED_Green);

  PIN_SET_AS_INPUT(PIN_Potentiometer);
  PIN_SET_AS_INPUT(PIN_Button_1);
  PIN_SET_AS_INPUT(PIN_Button_2);
  
  LED_RED_OFF;
  Red_Is_On = false;
  LED_GREEN_OFF;
  Green_Is_On = false;


  state = START_PROGRAM_IN_THIS_STATE;
  last_state = START_PROGRAM_IN_THIS_STATE;


  OpenAndWaitForSerialPort();
  //Terminal_Initialize();
  //ADC_Initialize();

  // Initialize the 1ms timer
  timer2_ctc(0.001, true); // Set up the 1ms timer to 1 millisecond

  //#ifdef FACTORY_RESET
  //WriteSettingsToEEPROM();
  //#endif
   
  //GetSettingsFromEEPROM();
  
  //Timer_Timer1_Initialize();

  // Initialize interrupts
  //EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
  //EIMSK |= (1 << INT0);     // Turns on INT0
  //#define BUTTON_ONE_INT_VECTOR PCINT23  // If changed, manually change ISR
  //#define BUTTON_ONE_INT_VECTOR_REGISTER PCMSK2
  //BUTTON_ONE_INT_VECTOR_REGISTER |= ( 1<< BUTTON_ONE_INT_VECTOR);   // Initialize interrupt on Button 1
  //#define BUTTON_TWO_INT_VECTOR PCINT0  // If changed, manually change ISR
  //#define BUTTON_TWO_INT_VECTOR_REGISTER PCMSK0
  //BUTTON_TWO_INT_VECTOR_REGISTER |= ( 1<< BUTTON_TWO_INT_VECTOR);   // Initialize interrupt on Button 2

  //PCICR |= (1<<PCIE2) | (1<<PCIE0);    // Enable PCINT0 and PCINT2 vector (PCINT23 pin / Button 1) (Had these as defines but it gets screwy because of PCINT23)
  //PCMSK0 |= (1<<PCINT0);   // Enable the mask bits for PCINT0
  //PCMSK2 |= (1<<PCINT23);   // Enable the mask bit for PCINT23

  PCICR |= (1<<PCIE0);    // Enable PCINT0 and PCINT2 vector (PCINT23 pin / Button 1) (Had these as defines but it gets screwy because of PCINT23)
  PCMSK0 |= (1<<PCINT0) | (1<<PCINT1);   // Enable the mask bits for PCINT0
  //PCMSK2 |= (1<<PCINT23);   // Enable the mask bit for PCINT23
 
  // Enable global interrupts (starts the 1ms timer)
  sei();          // Enable interrupts

  //state_SetNext(none); // actually handled above by the state = START_PROGRAM_IN_THIS_STATE;
  watchdog_feed();
  //logentry++; Serial.print(logentry); Serial.println("%d - Entering Main Loop...",logentry);
  //logentry++; Serial.print(logentry); Serial.println(

  //Serial.print("Time: ");
  //time = millis();
  //prints time since program started
  //Serial.println(time);

  Serial.println(VERSION);
  InitializeSDCard();
  
  while(1)
  {
    // Primary loop that the program cycles through
    events_Handler();   // Check and act on any events
    state_Handler();    // Call relevant functions for the current state
    state_Transition();   // Transition to a new state if relevant
    watchdog_feed();    // Watchdog timer resets (but not the count)
    watchdog_entertain(); // Re-enables the interrupt bit (so it doesn't reset but instead interrupts)
    _delay_ms(MAIN_LOOP_INTERVAL);     // 5ms keeps the program from cycling too fast but isn't necessarily what it is set to
  }
}





void OpenAndWaitForSerialPort( void )
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}


void InitializeSDCard( void )
{
   
  Serial.print("Init SD card... ");
  if (!SD.begin(4)) {
    Serial.println("init failed!");
    Red_LED_Flash();
    return;
  }
  Serial.println("init done.");
  Green_LED_Flash();
}


void AppendToFile( void )
{
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("test 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
    Green_LED_Flash();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    Red_LED_Flash();
  }
}


void ReadToConsoleFromFile( void )
{
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}


void StartRecording( void )
{
  OpenAndWaitForSerialPort();
  
  AppendToFile();
  ReadToConsoleFromFile();

}


void loop() {
  // nothing happens after setup
}







// These events tick down each time this event is called
void events_Tick(){
    event_Tick(&event_Test);
    event_Tick(&event_ChangeStateAfterDelay);
    event_Tick(&event_ReadDataFromDevice);
    event_Tick(&event_ButtonDebounce);
    event_Tick(&event_BlinkLEDs);
    event_Tick(&event_RedLEDOff);
    event_Tick(&event_GreenLEDOff);
}

void LogCCDataToSDCard( void );

void LogCCDataToSDCard( void )
{
  
  //_delay_ms(500);
  Serial.println("CC Data retrieved. [ <- Sim ]"); // Put here something that gets the data (have to have error check everywhere so this is likely to be split up)
  //_delay_ms(500);
  InitializeSDCard();
  AppendToFile();
  //_delay_ms(500);
  Serial.println("CC data written to SD Card. [ <- Sim ]"); // Put here something that writes it, including compression if the compression flagi s on. make that flag.
  //_delay_ms(500);
  
}

void state_Recording( void )
{
  logentry++; Serial.print(logentry); Serial.println(" - In state_Recording( void ). ");
  // REDO THIS WITH ERROR HANDLING
  LogCCDataToSDCard();
  Serial.println("Setting sleeping state... [ <- Sim ]");
  state_SetNext(sleep_until_next_recording);
  //Green_LED_Flash();
  
  
}



// State handler to call the relevant function for the program's state
void state_Handler()
{  
   if (state_is_fresh || status_change){
    state_is_fresh = false;
    logentry++; Serial.print(logentry); Serial.print(" - In state_Handler() for: ");
     switch(state) {
      case home:
        Serial.println("home");
        state_Home(); break;
      case recording:
        Serial.println("record");
        state_Recording(); break;
      case stop_recording:
        Serial.println("stop_recording");
        state_Stop_recording(); break;
      case sleep_until_next_recording:
        Serial.println("sleep_until_next_recording");
        state_Sleep_until_next_recording(); break;
      case format_card:
        Serial.println("format_card");
        state_Format_card(); break;
      case none:
        Serial.println("none");
        break;
      default: break;
     }
    status_change = false;
   }
}

bool Next_Button_Set_Is_Valid = true;
bool Both_Buttons_Were_Held = false;
// Handles the transitions between states based upon current system inputs and states
void state_Transition()
{
  if((Button1_Is_Actively_Pressed == false) && (Button2_Is_Actively_Pressed == false))// && (Both_Buttons_Were_Held == true))
    {
      Next_Button_Set_Is_Valid = true;
      Both_Buttons_Were_Held = false;
      //return;
    }
    
   if((Button1_Is_Actively_Pressed == true) && (Button2_Is_Actively_Pressed == true))// && (Both_Buttons_Were_Held == true))
    {
      Both_Buttons_Were_Held = true;
      //Next_Button_Set_Is_Valid = false;
      //return;
    }
  
  if ((state == none) && (Button1_Is_Actively_Pressed == true) && (Button1_HeldTime > BOTH_PRESSED_IF_WITHIN) && (Button2_Is_Actively_Pressed == false) && (Next_Button_Set_Is_Valid == true) && (Both_Buttons_Were_Held == false))
    {
      state_SetNext(recording);
      //Button1 = false; // If you only check if actively pressed, you can press both and then keep holding one to trigger antoher state. This forces unpressing a button before triggering another state.
      //Button2 = false;
      logentry++; Serial.print(logentry); Serial.println(" - State 'none' -> 'recording'.");
      Next_Button_Set_Is_Valid = false;
      return;
    }

  if ((state == sleep_until_next_recording) && (Button2_Is_Actively_Pressed == true) && (Button2_HeldTime > BOTH_PRESSED_IF_WITHIN) && (Button1_Is_Actively_Pressed == false) && (Next_Button_Set_Is_Valid == true) && (Both_Buttons_Were_Held == false))
    {
      state_SetNext(stop_recording);
      //Button1 = false; // If you only check if actively pressed, you can press both and then keep holding one to trigger antoher state. This forces unpressing a button before triggering another state.
      //Button2 = false;
      logentry++; Serial.print(logentry); Serial.println(" - State 'sleep_until_next_recording' -> 'stop_recording'.");
      Next_Button_Set_Is_Valid = false;
      return;
    }

  long heldtime_difference = Button1_HeldTime > Button2_HeldTime ? Button1_HeldTime - Button2_HeldTime : Button2_HeldTime - Button1_HeldTime; // basically absolute value of difference between held times.
  if ((state == none) && (Button1_Is_Actively_Pressed == true) && (Button2_Is_Actively_Pressed == true) && (heldtime_difference <= BOTH_PRESSED_IF_WITHIN) && (Button1_HeldTime > BOTH_PRESSED_IF_WITHIN) && (Button2_HeldTime > BOTH_PRESSED_IF_WITHIN) && (Next_Button_Set_Is_Valid == true))
    {
      //Button1 = false; // If you only check if actively pressed, you can press both and then keep holding one to trigger antoher state. This forces unpressing a button before triggering another state.
      //Button2 = false;
      logentry++; Serial.print(logentry); Serial.println(" - Both buttons pressed.");
      Next_Button_Set_Is_Valid = false;
      state_SetNext(format_card);
      //Both_Buttons_Were_Held = true;
      return;
    }


  
//  if ((state == home) && (System_Is_Armed)) // Main screen menu position 1
//  {
//    //MenuChoose(none,home_motion,none);
//    if(key == CHAR_BUTTON_STAR){
//      if (EnterPIN(home)){
//        DisarmSystem();
//      } else {
//        state_is_fresh = true;
//      }
//    }
//    return;
//  }
//  
//  if ((state == home) && (This_Is_A_Variable)) // Main screen menu position 1
//  {
//    //MenuChoose(home_alarmhistory,home_motion,none);
//    //if(key == CHAR_BUTTON_STAR){
//     // if (EnterPIN(home)){
//        //Fresh_Countdown = true;
//        //state_SetNext(countdown_main);
//      //} else {
//      //state_is_fresh = true;
//      //}
//    }
//    return;
//  }
//  
//  //if ((state == home_motion)  && (!System_Is_Armed)) // Main screen menu position 2
//  //{
//  //  //MenuChoose(home,home_latch,none);
//  //  return;
//  //}
}


void events_Handler_HighPriority( void )
{
   // if (event_IsReady(&event_PiezoBeepOn))
   // {
        // Turn on the Piezo buzzer if necessary and start the "Off" event
       // if(!piezo_is_on){
            //PIN_SET_HIGH(PIN_PIEZO);
        //    piezo_is_on = true;
        //    event_Start(&event_PiezoBeepOff);
        //}
   // }
}

void events_Handler( void ){
  
    if (event_IsReady(&event_Test)){
        event_Start(&event_Test); // Start the event count over (So it can start to count down while the remaining code is executed)
        event_Test_function();
    }

    if (event_IsReady(&event_ReadDataFromDevice)){
        event_Start(&event_ReadDataFromDevice); // Start the event count over (So it can start to count down while the remaining code is executed)
        ReadDataFromDevice();
    }

    if (event_IsReady(&event_ButtonDebounce)){
        event_Start(&event_ButtonDebounce);
        ButtonHandler();
    }

    if (event_IsReady(&event_BlinkLEDs)){
        event_Start(&event_BlinkLEDs);
        BlinkLEDs();
    }

    if (event_IsReady(&event_RedLEDOff)){
        LED_RED_OFF;
        Red_Is_On = false;
    }

    if (event_IsReady(&event_GreenLEDOff)){
        LED_GREEN_OFF;
        Green_Is_On = false;
    }


    if (event_IsReady(&event_ChangeStateAfterDelay)){
        state_SetNext(next_state);
        next_state = none;
    }
}

// Sets the next state
void state_SetNext(STATE_t newstate)
{
  last_state = state;
  state = newstate;
  state_is_fresh = true;
}


void state_SetNextAndNexter(STATE_t next, STATE_t nexter)
{
  state_SetNext(next);
  event_Start(&event_ChangeStateAfterDelay);
  next_state = nexter;
}


void state_Home()
{
//   LCD_clear();
//   if (System_Is_Armed) {
//    LCD_print(2,T_HM_ALARM_ON);
//    LCD_print(5,T_HM_LINETODISARM);
//   } else {
//     LCD_print(2,T_HM_ALARM_OFF);
//     LCD_print(5,T_HM_LINETOARM);
//   }
}


void watchdog_set() {
  WDTCSR |= (1<<WDCE) | (1<<WDE);   // Set Change Enable bit and Enable Watchdog System Reset Mode.
  WDTCSR = (1<<WDE) | (1<<WDIE) | WDPS_4S;
  // If frozen, will go for 4 seconds, trigger WDT_vect and go another 4 seconds then reset all.
}


void watchdog_entertain(void) {
  WDTCSR |= _BV(WDIE);
}


void WriteSettingsToEEPROM()
{
  //  eeprom_update_byte(&piezo,piezo_sound);
  //  eeprom_update_byte(&armcountdown,Arm_Countdown_Time);
  //  eeprom_update_byte(&mem_backlightmax,BacklightMax);
  //  eeprom_update_byte(&mem_backlightmin,BacklightMin);
  //  eeprom_update_byte(&mem_DISPLAY_IN_FAHRENHEIT,TEMP_DISPLAY_IN_FAHRENHEIT);
  //  eeprom_update_byte(&mem_kc1,kc1);
  //  eeprom_update_byte(&mem_kc2,kc2);
  //  eeprom_update_byte(&mem_kc3,kc3);
  //  eeprom_update_byte(&mem_kc4,kc4); 
}

 
void GetSettingsFromEEPROM()
{
  //  piezo_sound = eeprom_read_byte(&piezo);
  //  SetPiezoBeep();
  //  Arm_Countdown_Time = eeprom_read_byte(&armcountdown);
  //  BacklightMax = eeprom_read_byte(&mem_backlightmax);
  //  BacklightMin = eeprom_read_byte(&mem_backlightmin);
  //  TEMP_DISPLAY_IN_FAHRENHEIT = eeprom_read_byte(&mem_DISPLAY_IN_FAHRENHEIT);
  //  kc1 = eeprom_read_byte(&mem_kc1);
  //  kc2 = eeprom_read_byte(&mem_kc2);
  //  kc3 = eeprom_read_byte(&mem_kc3);
  //  kc4 = eeprom_read_byte(&mem_kc4);
}


// TIMER2 is set up as a 1ms timer
ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) {
    events_Tick();
    events_Handler_HighPriority();
    if(Button1_Is_Actively_Pressed == true) { Button1_HeldTime++; Button1_HeldTime_Latest = Button1_HeldTime; }
    if(Button2_Is_Actively_Pressed == true) { Button2_HeldTime++; Button2_HeldTime_Latest = Button2_HeldTime; }
}


ISR(WDT_vect) {
  // PJM 06-30-16 - Verified that this is called in 0.1.3
  //#ifdef WATCHDOG_TURNS_LIGHT_ON
  //  LED_red(TURN_ON);
  //  LED_green(TURN_ON);
  //#endif
  //Serial.println("Watchdog Timer Triggered");
}

// PCINT23_vect is an interrupt on PD7 (Button 1)
//ISR(PCINT23_vect)
// PCINT1_vect is an interrupt on PB1 (Button 1)
ISR(PCINT1_vect)
{
    Button1_Was_Pressed_With_Interrupt = true;
    ButtonActivity = true;
}

// PCINT0_vect is an interrupt on PB0 (Button 2)
ISR(PCINT0_vect)
{
    Button2_Was_Pressed_With_Interrupt = true;
    ButtonActivity = true;
}


void ReadDataFromDevice( void )
{
  // Read data from device
  // Write data to card
  // Be done with it all
  //logentry++; Serial.print(logentry); Serial.println(" - Tick - One Second\n");
  //logentry++; Serial.print(logentry); //Serial.println(" - Tick - One Second\n");
  //Serial.print("\tButton 1 - ");
  //Serial.print(Button1_Was_Pressed_With_Interrupt);
  //Serial.print("\t\t\tButton 2 - ");
  //Serial.println(Button2_Was_Pressed_With_Interrupt);
  
}



void ButtonHandler( void )
{

  Button1P = false;
  Button2P = false;

    if(PIN_READ(PIN_Button_1) == 1){
      Button1P = true;
    }
    if(PIN_READ(PIN_Button_2) == 1){
      Button2P = true;
    }

  // Button 1
  if(Button1P == true && Button1_Is_Actively_Pressed == false){
    Button1_Debounce_Times++;
    //StopDebouncing = false;
    if(Button1_Debounce_Times >= BUTTON_DEBOUNCE_REPEAT_TIMES){
      Button1 = true;
      //Button1_Press_Has_Been_Used = false;
      Button1_Is_Actively_Pressed = true;
      Button1_Debounce_Times = 0;
    }
  }
  if(Button1P == false && Button1_Is_Actively_Pressed == true){
    Button1_Debounce_Times++;
    if(Button1_Debounce_Times >= BUTTON_DEBOUNCE_REPEAT_TIMES){
      Button1_Is_Actively_Pressed = false;
      Button1_Debounce_Times = 0;
    }
  }  
  if(Button1P == true && Button1_Is_Actively_Pressed == true){
    Button1_Debounce_Times = 0;
    //Button1_HeldTime++;
  }
  if(Button1P == false && Button1_Is_Actively_Pressed == false){
    Button1_Debounce_Times = 0;
    if(Button1_HeldTime > 0){
      logentry++; Serial.print(logentry); Serial.print(" - B1 millis = "); Serial.println(Button1_HeldTime);
      Button1_HeldTime_Latest = Button1_HeldTime;
    }
    Button1_HeldTime = 0;
  } 

  // Button 2
  if(Button2P == true && Button2_Is_Actively_Pressed == false){
    Button2_Debounce_Times++;
    if(Button2_Debounce_Times >= BUTTON_DEBOUNCE_REPEAT_TIMES){
      Button2 = true;     // Are these used? they have to be manually unused.
      //Button2_Press_Has_Been_Used = false;
      Button2_Is_Actively_Pressed = true;
      Button2_Debounce_Times = 0;
    }
  }
  if(Button2P == false && Button2_Is_Actively_Pressed == true){
    Button2_Debounce_Times++;
    if(Button2_Debounce_Times >= BUTTON_DEBOUNCE_REPEAT_TIMES){
      Button2_Is_Actively_Pressed = false;
      Button2_Debounce_Times = 0;
    }
  }  
  if(Button2P == true && Button2_Is_Actively_Pressed == true){
    Button2_Debounce_Times = 0;
    //Button2_HeldTime++;
  }
  if(Button2P == false && Button2_Is_Actively_Pressed == false){
    Button2_Debounce_Times = 0;
    if(Button2_HeldTime > 0){
      logentry++; Serial.print(logentry); Serial.print(" - B2 millis = "); Serial.println(Button2_HeldTime);
      Button2_HeldTime_Latest = Button2_HeldTime;
    }
    Button2_HeldTime = 0;
  }

  Button1_Was_Pressed_With_Interrupt = false;
  Button2_Was_Pressed_With_Interrupt = false;

//  if(Button1_Is_Actively_Pressed == true && Button2_Is_Actively_Pressed == true)
//  {
//    logentry++; Serial.print(logentry); Serial.println(" - 1 & 2 Debounced and Pressed...");
//    return;
//  } 
//  if(Button1_Is_Actively_Pressed == true){
//    logentry++; Serial.print(logentry); Serial.println(" - 1 Debounced and Pressed...");
//  }
//  if(Button2_Is_Actively_Pressed == true){
//    logentry++; Serial.print(logentry); Serial.println(" - 2 Debounced and Pressed...");
//  }

}




void event_Test_function( void )
{
  #ifdef LIGHTS_ON_BUTTONS
  if(Button1_Is_Actively_Pressed == true)
  {
    LED_RED_ON; Red_Is_On = true;
  } else {
    if(Red_Is_On == true) { LED_RED_OFF; Red_Is_On = false; }
  }

  if(Button2_Is_Actively_Pressed == true)
  {
    LED_GREEN_ON; Green_Is_On = true;
  } else {
    if(Green_Is_On == true) { LED_GREEN_OFF; Green_Is_On = false; }
  }
  #endif
}



void BlinkLEDs( void )
{

  if(Red_LED_Blink_On == true)
  {
    if(Red_Is_On == true){
        LED_RED_OFF;
        Red_Is_On = false;
      } else {
        LED_RED_ON;
        Red_Is_On = true;
      }
  }

  if(Green_LED_Blink_On == true)
  {
    if(Green_Is_On == true){
        LED_GREEN_OFF;
        Green_Is_On = false;
      } else {
        LED_GREEN_ON;
        Green_Is_On = true;
      }
  }

  if(Green_LED_Blink_On == false && Green_Is_On == true){
    LED_GREEN_OFF;
  }

  if(Red_LED_Blink_On == false && Red_Is_On == true){
    LED_RED_OFF;
  }

  if(Green_LED_Blink_On == false && Red_LED_Blink_On == false){
    event_Cancel(&event_BlinkLEDs);
  }
}


void Red_LED_Start_Blinking( void )
{
  Red_LED_Blink_On = true;
  event_StartNow(&event_BlinkLEDs);
}


void Green_LED_Start_Blinking( void )
{
  Red_LED_Blink_On = true;
  event_StartNow(&event_BlinkLEDs);
}


void Red_LED_Stop_Blinking( void )
{
  Red_LED_Blink_On = false;
  event_StartNow(&event_BlinkLEDs);
}


void Green_LED_Stop_Blinking( void )
{
  Green_LED_Blink_On = false;
  event_StartNow(&event_BlinkLEDs);
}


void state_Sleep_until_next_recording( void )
{
  logentry++; Serial.print(logentry); Serial.println(" - In state_Sleep_until_next_recording(). ");
}


void state_Stop_recording( void )
{
  logentry++; Serial.print(logentry); Serial.println(" - In state_Stop_recording(). Resetting to state 'none'.");
  state_SetNext(none);
}

void Red_LED_Flash( void ) 
{
  //Red_LED_Start_Blinking();
  //_delay_ms(FLASH_LED_DURATION);
  //Red_LED_Stop_Blinking();
  LED_RED_ON; Red_Is_On = true; event_Start(&event_RedLEDOff);
}


void Green_LED_Flash( void ) // You should make this better, use an event with a timer that fires once.
{
  //Green_LED_Start_Blinking();
  //_delay_ms(FLASH_LED_DURATION);
  //Green_LED_Stop_Blinking();
  LED_GREEN_ON; Green_Is_On = true; event_Start(&event_GreenLEDOff);
}


void Both_LED_Flash( void )  // You should make this better, use an event with a timer that fires once.
{
  Green_LED_Start_Blinking();
  Red_LED_Start_Blinking();
  _delay_ms(FLASH_LED_DURATION);
  Green_LED_Stop_Blinking();
  Red_LED_Stop_Blinking();
}


void ReadSDCardToConsole( void )
{
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void state_Format_card( void )
{
  logentry++; Serial.print(logentry); Serial.println(" - In state_Format_card(). Resetting to state 'none'.");
  state_SetNext(none);
}

