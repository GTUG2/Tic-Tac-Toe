//  Sketc: basicSerialWithNL_001
// 
//  Uses hardware serial to talk to the host computer and software serial 
//  for communication with the Bluetooth module
//  Intended for Bluetooth devices that require line end characters "\r\n"
//
//  Pins
//  Arduino 5V out TO BT VCC
//  Arduino GND to BT GND
//  Arduino D9 to BT RX through a voltage divider
//  Arduino D8 BT TX (no need voltage divider)
//
//  When a command is entered in the serial monitor on the computer 
//  the Arduino will relay it to the bluetooth module and display the result.
//
 
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>  // Include core graphics library for the display
#include <Adafruit_SSD1306.h>  // Include Adafruit_SSD1306 library to drive the display
Adafruit_SSD1306 display;  // Create displayw

SoftwareSerial BTserial(8, 9); // RX | TX
 
const long baudRate = 9600; 
char c=' ';
boolean NL = true;
char curr_cursor_x=0;
char curr_cursor_y=0;
char curr_player=0;
char first_x = 49;
char first_y = 15;
char char_count=0;
char prev_char=' ';


void setup() 
{
    Serial.begin(9600);    
    BTserial.begin(baudRate);  
    delay(100);  // This delay is needed to let the display to initialize
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize display with the I2C address of 0x3C 
    display.clearDisplay();  // Clear the buffer
    display.setTextColor(WHITE);  // Set color of the text
    display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3
    display.setTextWrap(true);  // By default, long lines of text are set to automatically “wrap” back to the leftmost column.
                                 // To override this behavior (so text will run off the right side of the display - useful for
                                 // scrolling marquee effects), use setTextWrap(false). The normal wrapping behavior is restored
                                 // with setTextWrap(true).
                                 
    display.dim(0);  //Set brightness (0 is maximun and 1 is a little dim)
    
    display.display();
}
 
void loop()
{
    //display.clearDisplay();  // Clear the display so we can refresh    
    display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

 
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    if (BTserial.available())
    {
        c = BTserial.read();
        if(c=='q'){
          curr_cursor_x=0;
          curr_cursor_y=0;
          display.clearDisplay();  // Clear the display so we can refresh
          display.setCursor(0,0);
          //display.drawBitmap(0, 0, xox_board, 128, 64, WHITE);
          display.display();
        }else{
          display.print(c);
          /*if(char_count!=1){
            prev_char = c; 
          }
          char_count++;
          if(char_count==2){
            curr_player=1-curr_player;
            if(curr_player==0){
              display.drawCircle(first_x+(prev_char-'a')*19,first_y+(c-'0')*19,5,WHITE);
            }else{
              display.drawLine(first_x-7+(prev_char-'a')*19,first_y+7+(c-'0')*19,first_x+7+(prev_char-'a')*19,first_y-7+(c-'0')*19,WHITE);
              display.drawLine(first_x-7+(prev_char-'a')*19,first_y-7+(c-'0')*19,first_x+7+(prev_char-'a')*19,first_y+7+(c-'0')*19,WHITE);
            }
            char_count=0;
            display.display();
          }
        }*/
          Serial.write(c);
          display.display();
        }
    }
 
 
    // Read from the Serial Monitor and send to the Bluetooth module
    if (Serial.available())
    {
        c = Serial.read();
        BTserial.write(c);       
 
        // Echo the user input to the main window. The ">" character indicates the user entered text.
        if (NL) { Serial.print(F(">"));  NL = false; }
        Serial.write(c);
        if (c==10) { NL = true; }
    }

 
}
