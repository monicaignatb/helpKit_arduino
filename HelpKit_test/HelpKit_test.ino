#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AD9833.h"
#include <Encoder.h>
// set up encoder object

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define ICON_SIZE 15
// Pin definitions.
// - enc_a is ENC Signal A line (Arduino analog pin A0)
// - enc_b is ENC Signal B line (Arduino analog pin A1)
#define ENC_A  A0
#define ENC_B  A1
//#define ENC_BTN  0
Encoder myEnc(ENC_A, ENC_B);
//MD_REncoder R = MD_REncoder(A0, A1);
AD9833 AD(10, 11, 13);
int freq = 1000;
bool up = true;
char str[10];
//encoder parameters
int ENC_BTN = 0;
int voltageValue = 1; // Initial voltage value
int frequencyValue = 50; // Initial frequency value
int dcValue = 0; // Initial DC value
long oldPosition  = -999;
//minimum and maximum values for voltage, frequency and DC
int minVoltage = 0;
int maxVoltage = 10;
int maxFreq = 2000000;
int minFreq = 10;
int maxDC = 10;
int minDC = -10;

bool buttonPressed = false;
int mainMenuIndex = 0;
int subMenuIndex = 0;
int signalTypeIndex = 0;
float incStepVoltage = 1;
float incStepFreq=10;
//menu items
const char* mainMenuItems[] = {"SIGNAL", "AMP", "FREQ", "DC", "ON/OFF"};
const char* signalTypes[] = {"OFF", "Sine", "Square1", "Square2","Triangle"};
enum MenuState { SIGNAL_TYPE_MENU, VOLTAGE_MENU, FREQ_MENU, DC_MENU };
MenuState currentState = SIGNAL_TYPE_MENU;
enum MenuOption { SIGNAL_TYPE, VOLTAGE, FREQUENCY, DC_VALUE, MENU_SIZE };
MenuOption currentOption = SIGNAL_TYPE;
int signalType = 0;
int currentSignalIndex = 0;
const int numSignalTypes = sizeof(signalTypes) / sizeof(signalTypes[0]);
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void drawPoint(uint8_t x, uint8_t y)
{
  display.drawPixel(x, y, SSD1306_WHITE);
  display.drawPixel(x, y + 1, SSD1306_WHITE);
  display.drawPixel(x + 1, y, SSD1306_WHITE);
  display.drawPixel(x + 1, y + 1, SSD1306_WHITE);
}

// x should start at 10 since the width of an icon is 10 pixels
void drawText(uint8_t x, uint8_t y, const char text[])
{
  uint8_t current_position = x;
  for (uint8_t i = 0; text[i] != '\0'; i++)
  {
    if (text[i] == '.'){
      drawPoint(current_position, y + 12);
      current_position += 2;
    }
    else {
      display.drawChar(current_position, y, text[i], SSD1306_WHITE, SSD1306_BLACK, 2);
      current_position += 11;
    }
    
  }
}

void setup() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  display.display();
  delay(1500);
  //itoa(freq, str, 10);
  /*drawText(ICON_SIZE + 3, 0, str);//"900.0KHz");
  display.drawBitmap(0, 0, sine_wave, ICON_SIZE, ICON_SIZE, SSD1306_WHITE);
  // display.drawFastVLine(67, 0, 14, SSD1306_WHITE);
  // display.drawFastVLine(68, 0, 14, SSD1306_WHITE);
  display.drawBitmap(0, 15, amplitude, ICON_SIZE, ICON_SIZE, SSD1306_WHITE);
  drawText(ICON_SIZE + 3, 15, "0.1V");

  display.drawBitmap(0, 30, dc, ICON_SIZE, ICON_SIZE, SSD1306_WHITE);
  drawText(ICON_SIZE + 3, 30, "0.0V");

  display.drawBitmap(0, 45, out, ICON_SIZE, ICON_SIZE, SSD1306_WHITE);
  drawText(ICON_SIZE + 3, 45, "OFF");
  */
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
  pinMode(ENC_BTN, INPUT_PULLUP);
  Serial.begin(57600);
  SPI.begin();
  AD.begin();
  //AD.setWave(AD9833_SQUARE2);
}

void loop() {

  //AD.setFrequency(frequencyValue);
long newPosition = myEnc.read()/4;
  if (newPosition != oldPosition) {
    if (buttonPressed) {
      adjustValue(newPosition);
    } else {
      navigateMenu(newPosition);
    }
    oldPosition = newPosition;
  }
 // delay(10);  //  to simulate other tasks
/*long newPosition = myEnc.read() / 4;
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    switch (currentState) {
      case MAIN_MENU:
        mainMenuIndex = newPosition % 4;
        if (mainMenuIndex < 0) mainMenuIndex += 4;
        break;
      case SUB_MENU:
      values[mainMenuIndex] = constrain(values[mainMenuIndex] + (newPosition - oldPosition), 0, 100); // Adjust value
        break;
    }
    displayMenu();
  }
*/
  if (digitalRead(ENC_BTN) == LOW) {
   buttonPressed = !buttonPressed;
    delay(500); // Debounce delay
  }
}
void navigateMenu(long position) {
    position = (position % MENU_SIZE + MENU_SIZE) % MENU_SIZE;
    currentOption = static_cast<MenuOption>(position);
    updateMenu();
}

//void navigateMenu(long position) {
 // currentOption = static_cast<MenuOption>(position % MENU_SIZE);
 // updateMenu();
//}
void updateMenu() {
  display.clearDisplay();
  display.setCursor(0,0);
  for (int i = 0; i < MENU_SIZE; i++) {
    if (i == currentOption) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Highlight
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    switch (i) {
      case SIGNAL_TYPE:
        display.print("Signal Type: ");
        display.println(signalTypes[signalTypeIndex]); // "OFF", "Sine", "Square1", "Square2","Triangle"
        break;
      case VOLTAGE:
        display.print("Voltage: ");
        display.print(voltageValue);
        display.println(" V");
        break;
      case FREQUENCY:
        display.print("Frequency: ");
        display.print(frequencyValue);
        display.println(" Hz");
        break;
      case DC_VALUE:
        display.print("DC Value: ");
        display.print(dcValue);
        display.println(" V");
        break;
    }
  }
  display.display();
}

void adjustValue(long newPosition) {
  switch (currentOption) {
    case SIGNAL_TYPE:
    {
      if (oldPosition == -999) {
        // Handle the first run or reset condition
        oldPosition = newPosition;
      }
      signalTypeIndex = newPosition % 5; // 5 signal types
      if (signalTypeIndex < 0) signalTypeIndex += 5;
      AD.setWave(signalTypeIndex);
      //oldPosition = newPosition;
      break;
    }
    case VOLTAGE:
    {
      if (oldPosition == -999) {
        // Handle the first run or reset condition
        oldPosition = newPosition;
      }
      //voltageValue = constrain(voltageValue + (newPosition - oldPosition), 0, 5); // Adjust voltage value
      int positionChange = newPosition - oldPosition; // Calculate the change in position
      Serial.print("old position ");
        Serial.println(oldPosition); //
        Serial.print("new position ");
        Serial.println(newPosition); //
        Serial.print("pos change ");
        Serial.println(positionChange); //
        if (positionChange !=0) {
      voltageValue += positionChange * incStepVoltage; // Increment voltage based on position change
      if (voltageValue > maxVoltage) voltageValue = maxVoltage;
      if (voltageValue < minVoltage) voltageValue = minVoltage;
        }
      oldPosition = newPosition;
      break;
    }
    case FREQUENCY:
    {
    if (oldPosition == -999) {
        // Handle the first run or reset condition
        oldPosition = newPosition;
      }
      //voltageValue = constrain(voltageValue + (newPosition - oldPosition), 0, 5); // Adjust voltage value
      int positionChange = newPosition - oldPosition; // Calculate the change in position
      Serial.print("old position ");
        Serial.println(oldPosition); //
        Serial.print("new position ");
        Serial.println(newPosition); //
        Serial.print("pos change ");
        Serial.println(positionChange); //
        if (positionChange !=0) {
      frequencyValue += positionChange*incStepFreq;
      if (frequencyValue > maxFreq) frequencyValue = maxFreq;
      if (frequencyValue < minFreq) frequencyValue = minFreq;
        }
        oldPosition = newPosition;
        AD.setFrequency(frequencyValue);
      break;
    }
    case DC_VALUE:
    {
      if (oldPosition == -999) {
        // Handle the first run or reset condition
        oldPosition = newPosition;
      }
      //voltageValue = constrain(voltageValue + (newPosition - oldPosition), 0, 5); // Adjust voltage value
      int positionChange = newPosition - oldPosition; // Calculate the change in position
      Serial.print("old position ");
        Serial.println(oldPosition); //
        Serial.print("new position ");
        Serial.println(newPosition); //
        Serial.print("pos change ");
        Serial.println(positionChange); //
        if (positionChange !=0) {
      dcValue = newPosition;
      if (dcValue > maxDC) dcValue = maxDC;
      if (dcValue < minDC) dcValue = minDC;
        }
        oldPosition = newPosition;
      break;
    }
  }
  updateMenu();
}
 /* void handleEncoder(long newPosition) {
    switch (currentState) {
    //case MAIN_MENU:
      //mainMenuIndex = newPosition % 4; // four items in main menu
      //if (mainMenuIndex < 0) mainMenuIndex += 4;
      //break;
    case SIGNAL_TYPE_MENU:
      signalTypeIndex = newPosition % 5; // 5 signal types
      if (signalTypeIndex < 0) signalTypeIndex += 5;
      AD.setWave(signalTypeIndex);
      oldPosition = newPosition;
      break;
    case VOLTAGE_MENU:
    {
      if (oldPosition == -999) {
        // Handle the first run or reset condition
        oldPosition = newPosition;
      }
      //voltageValue = constrain(voltageValue + (newPosition - oldPosition), 0, 5); // Adjust voltage value
      int positionChange = newPosition - oldPosition; // Calculate the change in position
      Serial.print("old position ");
        Serial.println(oldPosition); //
        Serial.print("new position ");
        Serial.println(newPosition); //
        Serial.print("pos change ");
        Serial.println(positionChange); //
        if (positionChange !=0) {
      voltageValue += positionChange * incStepVoltage; // Increment voltage based on position change
      if (voltageValue > maxVoltage) voltageValue = maxVoltage;
        if (voltageValue < minVoltage) voltageValue = minVoltage;
      Serial.print("voltage ");
        Serial.println(voltageValue); //
        }
        oldPosition = newPosition;
    }
      break;
    case FREQ_MENU:
      frequencyValue = constrain(frequencyValue + (newPosition - oldPosition), 0, 100000); // Adjust frequency value
      if (newPosition != oldPosition) {
        if (newPosition > oldPosition) {
        frequencyValue++;
      } else {
        frequencyValue--;
      }
      oldPosition = newPosition;
      }
      AD.setFrequency(frequencyValue);
      break;
    case DC_MENU:
      dcValue = constrain(dcValue + (newPosition - oldPosition), 0, 5); // Adjust DC value
      oldPosition = newPosition;
      break;
  }
}
*/
/*void handleButtonPress() {
  if (currentState == SIGNAL_TYPE_MENU) {
    switch (mainMenuIndex) {
      case 0:
        currentState = ;
        break;
      case 1:
        currentState = VOLTAGE_MENU;
        break;
      case 2:
        currentState = FREQ_MENU;
        break;
      case 3:
        currentState = DC_MENU;
        break;
    }
  } else {
    currentState = MAIN_MENU;
  }
}
*/


  /*for (int i = 0; i < numItems; i++) {
    if (i == menuIndex) {
      display.setTextColor(SSD1306_BLACK, SSD1306_INVERSE);//SSD1306_BLACK); // Highlight selected item
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(20, i * 16);
    display.print(menuItems[i]);
    display.drawBitmap(0, i * 16, menuBitmaps[i], 16, 16, SSD1306_WHITE);
    //display.println(menuItems[i]);
  }
 */   
   
/*void selectMenuItem() {
  Serial.print("Selected: ");
  Serial.println(menuItems[menuIndex]);
}
*/


