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

#define B_U 3 // Up    button
#define B_D 4 // Down  button
#define B_M 5 // Mid   button
#define B_L 6 // Left  button
#define B_R 7 // Right button
#define JUMP_X 25
#define JUMP_Y 21
#define X_WIDTH 17
#define X_HEIGHT 13
#define O_GAP_X 8
#define O_GAP_Y 6
#define O_RADIUS 8
#define CURSOR_HEIGHT 3
#define DEBOUNCE_DELAY 100

struct packet{
  uint8_t start;
  uint8_t app_id;
  union{
    struct{
      uint8_t cell_states[9];
      uint8_t increase;
      uint8_t position;
    }xox;
    uint8_t stub[30];
  }data;
};

Adafruit_SSD1306 display;  // Create displayw
SoftwareSerial BTserial(8, 9); // RX | TX

const long baudRate = 9600; 
char curr_cursor_x=0;
char curr_cursor_y=0;
char curr_player=1;
char first_x = 28;
char first_y = 4;
char char_count=0;
char prev_char=' ';

char u_state = LOW;
char d_state = LOW;
char m_state = LOW;
char l_state = LOW;
char r_state = LOW;

char u_pressed = LOW;
char d_pressed = LOW;
char m_pressed = LOW;
char l_pressed = LOW;
char r_pressed = LOW;

unsigned long last_debounce_u = 0;
unsigned long last_debounce_d = 0;
unsigned long last_debounce_m = 0;
unsigned long last_debounce_l = 0;
unsigned long last_debounce_r = 0;

struct packet test_pkt;

char tic_tac_state[3][3]={
  {0,0,0},
  {0,0,0},
  {0,0,0}
};

void send_package(struct packet pkt){
  uint8_t i, *p = (uint8_t *)&pkt;
  for(i=0;i<sizeof(pkt);++i){
    BTserial.write(*p);
    ++p;
  }
}

void drawX(uint8_t x, uint8_t y){
  display.drawLine(first_x + x * JUMP_X, first_y + y * JUMP_Y, first_x + X_WIDTH + x * JUMP_X , first_y + X_HEIGHT + y * JUMP_Y, WHITE);
  display.drawLine(first_x + x * JUMP_X + X_WIDTH, first_y + y * JUMP_Y, first_x  + x * JUMP_X, first_y + X_HEIGHT + y * JUMP_Y, WHITE);
}
void drawO(uint8_t x, uint8_t y){
  display.drawCircle(first_x + O_GAP_X + x * JUMP_X, first_y + O_GAP_Y + y * JUMP_Y, O_RADIUS, WHITE);  
}
void drawCursor(uint8_t x, uint8_t y){
  display.fillRect(first_x + x * JUMP_X, first_y + y * JUMP_Y + X_HEIGHT - 3, X_WIDTH, CURSOR_HEIGHT, WHITE);
}

void drawTicTacBoard(){
  char i, j;
  display.drawFastHLine(24,21,75,WHITE);    
  display.drawFastHLine(24,42,75,WHITE);
  display.drawFastVLine(49,2,60,WHITE);    
  display.drawFastVLine(74,2,60,WHITE);
  for(i=0;i<3;++i){
    for(j=0;j<3;++j){
      if(tic_tac_state[i][j] == 1){
        drawX(i,j);
      }else if(tic_tac_state[i][j] == 2){
        drawO(i,j);
      }
    }
  }
    
}

void setup() 
{
  Serial.begin(9600);    
  BTserial.begin(baudRate);  
  
  delay(100);  // This delay is needed to let the display to initialize
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize display with the I2C address of 0x3C 
  display.clearDisplay();  // Clear the buffer
  display.setTextColor(WHITE);  // Set color of the text
  display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3
  display.setTextWrap(false);  // By default, long lines of text are set to automatically “wrap” back to the leftmost column.
  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0                             
  display.dim(0);  //Set brightness (0 is maximun and 1 is a little dim)
  
  //drawTicTacBoard();  
  pinMode(B_U, INPUT_PULLUP);
  pinMode(B_D, INPUT_PULLUP); 
  pinMode(B_M, INPUT_PULLUP); 
  pinMode(B_L, INPUT_PULLUP);
  pinMode(B_R, INPUT_PULLUP);
}
 
void loop()
{
  char i;
  
  display.clearDisplay();  // Clear the display so we can refresh   
  
  u_state = digitalRead(B_U);
  d_state = digitalRead(B_D);
  m_state = digitalRead(B_M);
  l_state = digitalRead(B_L);
  r_state = digitalRead(B_R);
  

  if( (millis() - last_debounce_u) > DEBOUNCE_DELAY) {
    if (( u_state == LOW)) {
      if(u_pressed == LOW){
        if(curr_cursor_y>0){
          curr_cursor_y--;
        }else{
          curr_cursor_y = 2;
        }
        u_pressed = HIGH;
        memset(&test_pkt, 0, sizeof(test_pkt));
        test_pkt.start = 127;
        test_pkt.app_id = 1;
        for(i=0;i<9;++i){
          switch (tic_tac_state[i%3][i/3]){
            case 0:
              test_pkt.data.xox.cell_states[i] = ' ';
              break;
            case 1:
              test_pkt.data.xox.cell_states[i] = 'X';
              break;
            case 2:
              test_pkt.data.xox.cell_states[i] = 'O';
              break;                          
            default:
              break;
          }
        }
        test_pkt.data.xox.position = curr_cursor_x + curr_cursor_y * 3;
        
        send_package(test_pkt);
      }
      last_debounce_u = millis(); //set the current time
    }else{
        u_pressed = LOW;
    }
  }
  if( (millis() - last_debounce_d) > DEBOUNCE_DELAY) {
    if (( d_state == LOW)) {
      if(d_pressed == LOW){
        if(curr_cursor_y < 2){
          curr_cursor_y++;
        }else{
          curr_cursor_y = 0;
        }
        d_pressed = HIGH;
        memset(&test_pkt, 0, sizeof(test_pkt));
        test_pkt.start = 127;
        test_pkt.app_id = 1;
        for(i=0;i<9;++i){
          switch (tic_tac_state[i%3][i/3]){
            case 0:
              test_pkt.data.xox.cell_states[i] = ' ';
              break;
            case 1:
              test_pkt.data.xox.cell_states[i] = 'X';
              break;
            case 2:
              test_pkt.data.xox.cell_states[i] = 'O';
              break;                          
            default:
              break;
          }
        }
        test_pkt.data.xox.position = curr_cursor_x + curr_cursor_y * 3;
        
        send_package(test_pkt);
      }
      last_debounce_d = millis(); //set the current time
    }else{
        d_pressed = LOW;
    }
  }
  if( (millis() - last_debounce_m) > DEBOUNCE_DELAY) {
    if (( m_state == LOW)) {
      if(m_pressed == LOW){
        if(tic_tac_state[curr_cursor_x][curr_cursor_y] == 0){
          tic_tac_state[curr_cursor_x][curr_cursor_y] = curr_player;
          curr_player = 3 - curr_player;
        }
                
        memset(&test_pkt, 0, sizeof(test_pkt));
        test_pkt.start = 127;
        test_pkt.app_id = 1;
        for(i=0;i<9;++i){
          switch (tic_tac_state[i%3][i/3]){
            case 0:
              test_pkt.data.xox.cell_states[i] = ' ';
              break;
            case 1:
              test_pkt.data.xox.cell_states[i] = 'X';
              break;
            case 2:
              test_pkt.data.xox.cell_states[i] = 'O';
              break;                          
            default:
              break;
          }
        }
        test_pkt.data.xox.position = curr_cursor_x + curr_cursor_y * 3;
        
        send_package(test_pkt);
        m_pressed = HIGH;
      }
      last_debounce_m = millis(); //set the current time
    }else{
        m_pressed = LOW;
    }
  }
  if( (millis() - last_debounce_l) > DEBOUNCE_DELAY) {
    if (( l_state == LOW)) {
      if(l_pressed == LOW){
        if(curr_cursor_x > 0){
          curr_cursor_x--;
        }else{
          curr_cursor_x = 2;
        }
        l_pressed = HIGH;
        memset(&test_pkt, 0, sizeof(test_pkt));
        test_pkt.start = 127;
        test_pkt.app_id = 1;
        for(i=0;i<9;++i){
          switch (tic_tac_state[i%3][i/3]){
            case 0:
              test_pkt.data.xox.cell_states[i] = ' ';
              break;
            case 1:
              test_pkt.data.xox.cell_states[i] = 'X';
              break;
            case 2:
              test_pkt.data.xox.cell_states[i] = 'O';
              break;                          
            default:
              break;
          }
        }
        test_pkt.data.xox.position = curr_cursor_x + curr_cursor_y * 3;
        
        send_package(test_pkt);
      }
      last_debounce_l = millis(); //set the current time
    }else{
        l_pressed = LOW;
    }
  }
  if( (millis() - last_debounce_r) > DEBOUNCE_DELAY) {
    if (( r_state == LOW)) {
      if(r_pressed == LOW){
        if(curr_cursor_x < 2){
          curr_cursor_x++;
        }else{
          curr_cursor_x = 0;
        }
        r_pressed = HIGH;
        memset(&test_pkt, 0, sizeof(test_pkt));
        test_pkt.start = 127;
        test_pkt.app_id = 1;
        for(i=0;i<9;++i){
          switch (tic_tac_state[i%3][i/3]){
            case 0:
              test_pkt.data.xox.cell_states[i] = ' ';
              break;
            case 1:
              test_pkt.data.xox.cell_states[i] = 'X';
              break;
            case 2:
              test_pkt.data.xox.cell_states[i] = 'O';
              break;                          
            default:
              break;
          }
        }
        test_pkt.data.xox.position = curr_cursor_x + curr_cursor_y * 3;
        
        send_package(test_pkt);
      }
      last_debounce_r = millis(); //set the current time
    }else{
        r_pressed = LOW;
    }
  }
  
  drawTicTacBoard();
  drawCursor(curr_cursor_x,curr_cursor_y);
  
  
  display.display();
}
