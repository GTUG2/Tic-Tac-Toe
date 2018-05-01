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

 
/*************************** INCLUDES *************************/
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>  // Include core graphics library for the display
#include <Adafruit_SSD1306.h>  // Include Adafruit_SSD1306 library to drive the display

/*************************** DEFINES *************************/
#define B_U 3 // Up    button
#define B_D 4 // Down  button
#define B_M 5 // Mid   button
#define B_L 6 // Left  button
#define B_R 7 // Right button
#define BTN_COUNT 5 
#define JUMP_X 25
#define JUMP_Y 21
#define X_WIDTH 17
#define X_HEIGHT 13
#define FIRST_X 28
#define FIRST_Y 4
#define O_GAP_X 8
#define O_GAP_Y 6
#define O_RADIUS 8
#define CURSOR_HEIGHT 3
#define DEBOUNCE_DELAY 100
#define START_BYTE 127
#define MID_HOLD_MAX 3000
#define MENU_STATE 0
#define XOX_STATE 1
#define CONNECT4_STATE 2
#define PINGBOSS_STATE 3
#define INFMSG_STATE 6
#define REQMSG_STATE 7

/*************************** INSTANCES *************************/
Adafruit_SSD1306 display;  // Create displayw
SoftwareSerial BTserial(8, 9); // RX | TX

/*************************** TYPEDEF ETC.*************************/
struct packet{
  uint8_t start;
  uint8_t app_id;
  union{
    struct{
      uint8_t player_id;
    }initial;
    struct{
      uint8_t curr_item;
      uint8_t is_selected;
      uint8_t accepted;
    }menu;
    struct{
      uint8_t cell_states[9];
      uint8_t increase;
      uint8_t pos;
    }xox;
    uint8_t _[30];
  }data;
};

typedef enum btn_type{
  UP,
  DOWN,
  MID,
  LEFT,
  RIGHT
}btn_type_T;

const char  menu_item_name[6][13]={
  "",
  "Tic-Tac-Toe",
  " Connect 4",
  " Ping-Boss",
  "Adam Asmaca",
  "   Taiko"
};


/************************* GLOBAL VARIABLES ***********************/
char tic_tac_state[3][3]={
  {0,0,0},
  {0,0,0},
  {0,0,0}
};
const long baudRate = 9600; 
char general_state = 1; /*0=Menu, 1=TicTacToe, 2=Connect4, 3=Hangman, 4=PingBoss, 5=Taiko, 6=InfoMessage, 7=RequestMessage*/
char prev_state = 0;
char curr_cursor_x=0;
char curr_cursor_y=0;
char curr_player=1;
char score_x=0;
char score_y=0;

char player_id=1;
char char_count=0;
char prev_char=' ';
char curr_menu_item = 1;

char received_c=-1, received_c_count = 0;
uint8_t *rec_ptr;
char reqMsgSelection = 1;

char btn_state_arr[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
char btn_pressed_arr[5] = {0, 0, 0, 0, 0};
unsigned long last_dbn_arr[5] = {0, 0, 0, 0, 0};
unsigned long mid_pressed_ms = 0;
char mid_pressed_flag=0;

struct packet test_pkt;
struct packet received_pkt;

/*************************** FUNCTIONS *************************/
void send_packet(struct packet pkt){
  uint8_t i, *p = (uint8_t *)&pkt;
  for(i=0;i<sizeof(pkt);++i){
    BTserial.write(*p);
    ++p;
  }
}

void drawX(uint8_t x, uint8_t y){
  display.drawLine(FIRST_X + x * JUMP_X, FIRST_Y + y * JUMP_Y, FIRST_X + X_WIDTH + x * JUMP_X , FIRST_Y + X_HEIGHT + y * JUMP_Y, WHITE);
  display.drawLine(FIRST_X + x * JUMP_X + X_WIDTH, FIRST_Y + y * JUMP_Y, FIRST_X  + x * JUMP_X, FIRST_Y + X_HEIGHT + y * JUMP_Y, WHITE);
}
void drawO(uint8_t x, uint8_t y){
  display.drawCircle(FIRST_X + O_GAP_X + x * JUMP_X, FIRST_Y + O_GAP_Y + y * JUMP_Y, O_RADIUS, WHITE);  
}
void drawCursorTicTac(uint8_t x, uint8_t y){
  display.fillRect(FIRST_X + x * JUMP_X, FIRST_Y + y * JUMP_Y + X_HEIGHT - 3, X_WIDTH, CURSOR_HEIGHT, WHITE);
}

void calculateCursorTicTac(uint8_t curr_loop){
  switch(curr_loop){
    case 0:
      if(curr_cursor_y>0){
        curr_cursor_y--;
      }else{
        curr_cursor_y = 2;
      }
      break;
    case 1:
      if(curr_cursor_y<2){
        curr_cursor_y++;
      }else{
        curr_cursor_y = 0;
      }
      break;
    case 2:
      break;
    case 3:
      if(curr_cursor_x>0){
        curr_cursor_x--;
      }else{
        curr_cursor_x = 2;
      }
      break;
    case 4:
      if(curr_cursor_x<2){
        curr_cursor_x++;
      }else{
        curr_cursor_x = 0;
      }
      break;
    default:
      break;    
  }
}
void calculateMenuSelection(uint8_t curr_loop){
  switch(curr_loop){
    case 3:
      if(curr_menu_item>1){
        curr_menu_item--;
      }else{
        curr_menu_item = 5;
      }   
      break;
    case 4:
      if(curr_menu_item<5){
        curr_menu_item++;
      }else{
        curr_menu_item = 1;
      }        
      break;    
    default:
      break;    
  }
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
  display.drawFastVLine(16,0,31,WHITE);    
  display.drawFastHLine(0,10,16,WHITE);
  display.drawFastHLine(0,20,16,WHITE);
  display.drawFastHLine(0,31,16,WHITE);
  
  display.drawFastVLine(111,0,31,WHITE);    
  display.drawFastHLine(111,10,16,WHITE);
  display.drawFastHLine(111,20,16,WHITE);
  display.drawFastHLine(111,31,16,WHITE);

  display.setCursor(1,1);
  display.write('P');
  display.write(48+player_id);

  display.setCursor(114,1);
  display.write('P');
  display.write(48+3-player_id);

  display.setCursor(5,12); 
  display.write(player_id==1?'X':'O');

  display.setCursor(117,12); 
  display.write(player_id==2?'X':'O');

  display.setCursor(5,23);
  display.print((int)score_x);
  
  display.setCursor(117,23);
  display.print((int)score_y);
  
}

void drawMenu(){
  display.drawFastHLine(0,10,16,WHITE);
  display.drawFastVLine(16,0,10,WHITE);
  display.drawRoundRect(21,23,81, 19, 6, WHITE);
  display.fillTriangle(10, 33, 16, 28, 16, 38, WHITE);
  display.fillTriangle(113, 33, 106, 28, 106, 38, WHITE);
  display.setCursor(1,1);
  display.write('P');
  display.write(48+player_id);
  display.setCursor(26,30);
  display.print(menu_item_name[curr_menu_item]);
}

void drawInfMsg(){
  display.setTextSize(1);
  display.setCursor(9,30);
  display.print(F("Rakip bekleniyor..."));
  display.setTextSize(0);
}

void drawReqMsg(){
  display.setTextSize(1);
  display.setCursor(38,10);
  display.print(F("Oyuncu "));
  display.print((int)(3-player_id));

  //received_pkt.data.menu.curr_item = 2;
  Serial.println((int)received_pkt.data.menu.curr_item);
  display.setCursor(30,23);
  display.print(menu_item_name[received_pkt.data.menu.curr_item]);

  display.setCursor(20,35);
  display.print(F("Oynamak Istiyor"));

  display.setTextSize(0);
  display.setCursor(12,51);
  display.print(F("Kabul Et"));
  
  display.setCursor(86,51);
  display.print(F("Reddet"));
  
}

void drawReqMsgSelection(){
  if(reqMsgSelection == 1){
    display.fillTriangle(4,51,4,57,9,54,WHITE);
  }else if(reqMsgSelection == 2){
    display.fillTriangle(78,51,78,57,83,54,WHITE);
  }
  
}


/*************************** SETUP *************************/
void setup() 
{
  Serial.begin(9600);    
  BTserial.begin(baudRate);

  rec_ptr = (uint8_t *)&received_pkt;

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

  received_pkt.data.menu.curr_item = 1;
}

/*************************** LOOP *************************/
void loop()
{
  char i=0, counter=0;

  received_c = BTserial.read();
  while(received_c != -1 && received_c_count < 32){
    if(received_c == 127){
      received_c_count = 0;
      rec_ptr = (uint8_t *)&received_pkt;
    }
//    Serial.print((int)received_c_count);
    *rec_ptr = (uint8_t)received_c;
    rec_ptr++;
    received_c_count++;
    received_c = BTserial.read();
  }
  if(received_c_count < 32 && received_c == -1){
    received_c_count = 0;
    rec_ptr = (uint8_t *)&received_pkt;
  }else if(received_c_count == 32){
    rec_ptr = (uint8_t *)&received_pkt;
    for(i=0;i<32;++i){
      rec_ptr++;
    }
    rec_ptr = (uint8_t *)&received_pkt;

    switch(general_state){
      case MENU_STATE:
        general_state = received_pkt.app_id;
        break;
      case XOX_STATE:
        break;
      case INFMSG_STATE:
        if(received_pkt.data.menu.accepted == 1){
          general_state = received_pkt.data.menu.curr_item;
        }else if(received_pkt.data.menu.accepted == 2){
          general_state = MENU_STATE;        
        }
        break;
      case REQMSG_STATE:
        if(received_pkt.data.menu.accepted == 1){
          general_state = received_pkt.data.menu.curr_item;
        }else if(received_pkt.data.menu.accepted == 2){
          general_state = MENU_STATE;        
        }
      default:
        break;      
    }
    received_c_count = 0;
  }
  
  display.clearDisplay();  // Clear the display so we can refresh   

  for(i=0;i<BTN_COUNT;++i){
    btn_state_arr[i] = digitalRead(B_U + i);
  }

  /*
    i = 0 -> up     button
    i = 1 -> down   button
    i = 2 -> mid    button
    i = 3 -> left   button
    i = 4 -> right  button
  */
  for(i=0;i<BTN_COUNT;++i){
    if( (millis() - last_dbn_arr[i]) > DEBOUNCE_DELAY) {
      if (btn_state_arr[i] == LOW) {
        if(i == 2 && !mid_pressed_flag){
          mid_pressed_ms = millis();
          mid_pressed_flag  = 1;          
        }
        if(i == 2 && millis() - mid_pressed_ms >= MID_HOLD_MAX){
          Serial.println(F("GG WP."));
          mid_pressed_flag  = 0;          
        }
        if(btn_pressed_arr[i] == 0){          
          btn_pressed_arr[i] = 1;
          switch(general_state){
            case 0:
              calculateMenuSelection(i);
              memset(&test_pkt, 0, sizeof(test_pkt));
              test_pkt.start = 127;
              test_pkt.app_id = 0;
              test_pkt.data.menu.curr_item = curr_menu_item;
              if(i == 2){
                test_pkt.data.menu.is_selected = 1;
              }
              send_packet(test_pkt);
              break;             
            case 1:
              calculateCursorTicTac(i);
              if(i== 2){
                if(tic_tac_state[curr_cursor_x][curr_cursor_y] == 0){
                  tic_tac_state[curr_cursor_x][curr_cursor_y] = curr_player;
                  curr_player = 3 - curr_player;
                }
              }
              memset(&test_pkt, 0, sizeof(test_pkt));
              test_pkt.start = 127;
              test_pkt.app_id = 1;
              for(counter=0;counter<9;++counter){
                switch (tic_tac_state[counter%3][counter/3]){
                  case 0:
                    test_pkt.data.xox.cell_states[counter] = ' ';
                    break;
                  case 1:
                    test_pkt.data.xox.cell_states[counter] = 'X';
                    break;
                  case 2:
                    test_pkt.data.xox.cell_states[counter] = 'O';
                    break;                          
                  default:
                    break;
                }
              }
              test_pkt.data.xox.pos = curr_cursor_x + curr_cursor_y * 3;
              send_packet(test_pkt);
              break;
            case 7:
              if(i == 3){
                reqMsgSelection = 1;
              }else if(i == 4){
                reqMsgSelection = 2;
              }
              memset(&test_pkt, 0, sizeof(test_pkt));
              test_pkt.start = 127;
              test_pkt.app_id = 0;
              test_pkt.data.menu.curr_item = reqMsgSelection;
              if(i == 2){
                test_pkt.data.menu.accepted = reqMsgSelection;
              }
              send_packet(test_pkt);
              break;             
            default:
              break; 
          }          
        }        
        last_dbn_arr[i] = millis(); //set the current time
      }else{
        if(i == 2 && millis() - mid_pressed_ms < MID_HOLD_MAX){
          mid_pressed_flag  = 0;          
        }
        btn_pressed_arr[i] = LOW;
      }
    }    
  } 

  switch(general_state){
    case MENU_STATE:
      drawMenu();
      break;
    case XOX_STATE:
      drawTicTacBoard();
      drawCursorTicTac(curr_cursor_x,curr_cursor_y);// TODO: draw only if that player's state
      break;
    case INFMSG_STATE:
      drawInfMsg();
      break;
    case REQMSG_STATE:
      drawReqMsgSelection();
      drawReqMsg();
      break;
    case 4:
      break;
    default:
      break;
  }
  
  display.display();
}
