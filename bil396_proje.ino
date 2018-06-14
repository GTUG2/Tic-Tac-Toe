/*  Pins
   Arduino 5V out TO BT VCC
   Arduino GND to BT GND
   Arduino D9 to BT RX through a voltage divider
   Arduino D8 BT TX (no need voltage divider)
*/
/*************************** INCLUDES *************************/
#include <SoftwareSerial.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <avr/pgmspace.h>

/*************************** DEFINES *************************/
#define B_U 3 // Up    button
#define B_D 4 // Down  button
#define B_M 5 // Mid   button
#define B_L 6 // Left  button
#define B_R 7 // Right button
#define BTN_COUNT 5
#define JUMP_X 25
#define JUMP_Y 21
#define CONNECT4_JUMP_X 10
#define CONNECT4_JUMP_Y 8
#define X_WIDTH 17
#define X_HEIGHT 13
#define FIRST_X 28
#define FIRST_Y 4
#define O_GAP_X 8
#define O_GAP_Y 6
#define O_RADIUS 8
#define CURSOR_HEIGHT 3
#define DEBOUNCE_DELAY 50
#define START_BYTE 127
#define MID_HOLD_MAX 3000

#define MENU_STATE 0
#define XOX_STATE 1
#define CONNECT4_STATE 2
#define PINGBOSS_STATE 3
#define INFMSG_STATE 6
#define REQMSG_STATE 7
#define INITIAL_STATE 8
#define WIN_STATE 9
#define LOSE_STATE 10
#define DRAW_STATE 11
#define PING_WIN_STATE 12
#define PING_LOSE_STATE 13
#define RESET_REQUEST_ID 99
#define olsun_artik2_width 128
#define olsun_artik2_height 64

#define boss_width 30
#define boss_height 20
#define left_player_width 15
#define left_player_height 15
#define right_player_width 15
#define right_player_height 15
#define BOSS_MAX_HP 6

static const uint8_t right_player_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x0e, 0x00, 0x0a, 0xe0, 0x0e, 0x40, 0x04, 0xfc, 0x0f,
  0x90, 0x0f, 0x00, 0x04, 0x80, 0x0f, 0x80, 0x08, 0x80, 0x08, 0x80, 0x38,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t left_player_bits[] PROGMEM = {
  0x00, 0x00, 0x38, 0x00, 0x28, 0x00, 0xb8, 0x03, 0x10, 0x01, 0xf8, 0x1f,
  0xf8, 0x04, 0x10, 0x00, 0xf8, 0x00, 0x88, 0x00, 0x88, 0x00, 0x8e, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t boss_bits[] PROGMEM = {
  0x80, 0xff, 0x7f, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x60, 0x00, 0x80, 0x01,
  0x20, 0x03, 0x30, 0x01, 0x20, 0x06, 0x18, 0x01, 0x20, 0x0c, 0x0c, 0x01,
  0x20, 0x1f, 0x3e, 0x01, 0x20, 0x1b, 0x36, 0x01, 0x20, 0x1b, 0x36, 0x01,
  0x2f, 0x1f, 0x3e, 0x3d, 0x2f, 0x0e, 0x1c, 0x3d, 0x2c, 0x00, 0x00, 0x0d,
  0x38, 0x00, 0x00, 0x07, 0x20, 0x00, 0x00, 0x01, 0x20, 0xfe, 0x1f, 0x01,
  0x20, 0xfe, 0x1f, 0x01, 0x20, 0x00, 0x00, 0x01, 0x60, 0x00, 0x80, 0x01,
  0xc0, 0x00, 0xc0, 0x00, 0x80, 0xff, 0x7f, 0x00
};

static const uint8_t olsun_artik2_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x0f, 0xfc, 0xfc, 0xe0, 0x3f, 0xf8, 0x01, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0xff, 0xfc, 0xe3, 0x3f, 0xfe,
  0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x03,
  0x04, 0x06, 0x30, 0x06, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x30, 0x80, 0x01, 0x04, 0x0c, 0x30, 0x03, 0x00, 0x00, 0x00, 0x38,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x01, 0x04, 0x0c, 0x18, 0x03,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x80, 0x01,
  0x04, 0x04, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x18, 0x80, 0x01, 0x0c, 0x07, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x1a,
  0x00, 0x00, 0x06, 0x00, 0x00, 0x18, 0x80, 0xff, 0xfc, 0x03, 0x06, 0xff,
  0x01, 0x00, 0x00, 0x0f, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x18, 0x98, 0x01,
  0x0c, 0x06, 0x03, 0x03, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x78, 0x00, 0x00,
  0x00, 0x18, 0x98, 0x01, 0x04, 0x84, 0x03, 0x03, 0x00, 0x00, 0x80, 0x1b,
  0x00, 0x1f, 0x00, 0x00, 0x00, 0x18, 0x98, 0x01, 0x04, 0x8c, 0x01, 0x03,
  0x00, 0x00, 0xc0, 0x0a, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x30, 0x98, 0x01,
  0x04, 0xcc, 0x00, 0x03, 0x00, 0x00, 0x80, 0x0f, 0xf0, 0x07, 0x00, 0x00,
  0x00, 0x30, 0x98, 0x01, 0x04, 0x66, 0x00, 0x03, 0x00, 0x00, 0x80, 0x0e,
  0xfc, 0x03, 0x00, 0x00, 0x00, 0xe0, 0x18, 0x07, 0x0c, 0xe7, 0x7f, 0x0e,
  0x00, 0x00, 0xf0, 0x02, 0xff, 0x01, 0x00, 0x00, 0x00, 0xc0, 0x1f, 0xfe,
  0xfc, 0xe3, 0x7f, 0xfc, 0x01, 0x00, 0xe0, 0xc0, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xe0,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xfc, 0x1f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xff,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x05, 0x10,
  0x00, 0x00, 0x80, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x7d, 0x51, 0xa8, 0x08, 0x20, 0x14, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0xc9, 0xa8, 0x84, 0x68, 0x14,
  0x00, 0x00, 0x19, 0xd8, 0x05, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0xc5,
  0xa9, 0x82, 0xe8, 0x14, 0x00, 0x00, 0x84, 0xdf, 0x05, 0x00, 0x00, 0x00,
  0x00, 0x40, 0x04, 0x43, 0xa9, 0x81, 0xa8, 0x14, 0x00, 0x00, 0xf2, 0xdf,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x3c, 0x43, 0xab, 0x81, 0xa8, 0x15,
  0x00, 0x00, 0xf4, 0xdf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x04, 0x45,
  0xae, 0x82, 0x28, 0x17, 0x00, 0x40, 0xef, 0x73, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x40, 0x04, 0x49, 0xac, 0x84, 0x28, 0x16, 0x00, 0x20, 0x9f, 0xb3,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x40, 0x7c, 0x51, 0xa8, 0x08, 0x27, 0x54,
  0x00, 0x20, 0x7e, 0xde, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xf8, 0xe1, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xe2, 0xdb,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x60, 0xc4, 0xde, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x08, 0xff, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x39, 0xec,
  0x01, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x91, 0x00, 0xc7, 0x18, 0x00, 0x00,
  0x00, 0x40, 0x73, 0xf0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x20, 0x92, 0x00,
  0x28, 0x25, 0x00, 0x00, 0x00, 0x40, 0xe2, 0x81, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x92, 0x00, 0x28, 0x05, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x0f,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x91, 0x00, 0xc6, 0x1d, 0x00, 0x00,
  0x00, 0x80, 0x8c, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x20, 0x92, 0x00,
  0x08, 0x25, 0x00, 0x00, 0x00, 0x80, 0x1c, 0xff, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x92, 0x00, 0x28, 0x25, 0x00, 0x00, 0x00, 0x00, 0x78, 0xfc,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x91, 0x07, 0xc7, 0x18, 0x00, 0x00,
  0x00, 0x00, 0xf9, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf1, 0x1f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf2, 0xe1,
  0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x81, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x7f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x73, 0x44,
  0x1f, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x01, 0x00, 0x00,
  0x00, 0x20, 0x90, 0x44, 0x11, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0xc0, 0x07, 0x00, 0x00, 0x00, 0x20, 0x90, 0x44, 0x11, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x06, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x20, 0x73, 0x44,
  0x1f, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x1c, 0x00, 0x00,
  0x00, 0x20, 0x52, 0x44, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
  0x00, 0x30, 0x00, 0x00, 0x00, 0x20, 0x92, 0x44, 0x01, 0x08, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x10, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x93, 0x38,
  0x01, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};
/*************************** INSTANCES *************************/
//U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_MIRROR, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
SoftwareSerial BTserial(8, 9); // RX | TX

/*************************** TYPEDEF ETC.*************************/
struct packet {
  uint8_t start;
  uint8_t app_id;
  union {
    struct {
      uint8_t player_id;
    } initial;
    struct {
      uint8_t curr_item;
      uint8_t is_selected;
      uint8_t accepted;
      uint8_t cursor_;
    } menu;
    struct {
      uint8_t cell_states[9];
      uint8_t scoreP1;
      uint8_t scoreP2;
      uint8_t pos;
      uint8_t is_clicked;
      uint8_t turn;
      uint8_t win_round;
      uint8_t win_game;
    } xox;
    struct {
      uint8_t p1_cells[6];
      uint8_t p2_cells[6];
      uint8_t scoreP1;
      uint8_t scoreP2;
      uint8_t pos;
      uint8_t is_clicked;
      uint8_t turn;
      uint8_t roundWon;
      uint8_t x1;
      uint8_t y1;
      uint8_t x2;
      uint8_t y2;
    } connect_four;
    struct {
      uint8_t p1pos;
      uint8_t p2pos;
      uint8_t bossPos;
      uint8_t bossHp;
      uint8_t pressedButton;

      uint8_t bossBullets[10];  //x koordinati 0 ise dizi elemani bos
      uint8_t p1Bullets[5];   //x koordinati 0 ise dizi elemani bos
      uint8_t p2Bullets[5];    //x koordinati 0 ise dizi elemani bos
    } ping_boss;
    uint8_t _[30];
  } data;
};

const char menu_item_name[4][13] = {
  "",
  "Tic-Tac-Toe",
  " Connect 4",
  " Ping-Boss"
};


/************************* GLOBAL VARIABLES ***********************/
char tic_tac_state[9] = {
  0, 0, 0,
  0, 0, 0,
  0, 0, 0
};

char connect_four_state[6][7] = {
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0}
};

/*0=Menu, 1=TicTacToe, 2=Connect4, 3=PingBoss, 6=InfoMessage, 7=RequestMessage, 8=Initial, 9=Win, 10=Lose, 11=Draw, 12=PingBossWin, 13=PingBossLose */

char general_state = INITIAL_STATE;

const long baudRate = 9600;
char prev_state = 0;
char curr_cursor_x = 0;
char curr_cursor_y = 0;
char curr_player = 1;
char score_x = 0;
char score_o = 0;

char bossHp = BOSS_MAX_HP;
char bossY = 32;
char p1pos = 32;
char p2pos = 32;

char player_id = 1;
char char_count = 0;
char prev_char = ' ';
char curr_menu_item = 1;
char will_display = 1;

char received_c = -1, received_c_count = 0;
uint8_t *rec_ptr;
char reqMsgSelection = 1;

char btn_state_arr[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
char btn_pressed_arr[5] = {0, 0, 0, 0, 0};
unsigned long last_dbn_arr[5] = {0, 0, 0, 0, 0};
unsigned long mid_pressed_ms = 0;
char mid_pressed_flag = 0;

static struct packet to_send_pkt = {};
static struct packet received_pkt = {};

/*************************** FUNCTIONS *************************/
void send_packet(struct packet pkt) {
  uint8_t i, *p = (uint8_t *)&pkt;
  for (i = 0; i < sizeof(pkt); ++i) {
    BTserial.write(*p);
    ++p;
  }
}

void drawX(uint8_t x, uint8_t y, uint8_t inverse) {
  u8g2.setColorIndex(1 - inverse);
  u8g2.drawLine(FIRST_X + x * JUMP_X, FIRST_Y + y * JUMP_Y, FIRST_X + X_WIDTH + x * JUMP_X , FIRST_Y + X_HEIGHT + y * JUMP_Y);
  u8g2.drawLine(FIRST_X + x * JUMP_X + X_WIDTH, FIRST_Y + y * JUMP_Y, FIRST_X  + x * JUMP_X, FIRST_Y + X_HEIGHT + y * JUMP_Y);
  u8g2.setColorIndex(1);
}

void drawO(uint8_t x, uint8_t y, uint8_t inverse) {
  u8g2.setColorIndex(1 - inverse);
  u8g2.drawCircle(FIRST_X + O_GAP_X + x * JUMP_X, FIRST_Y + O_GAP_Y + y * JUMP_Y, O_RADIUS);
  u8g2.setColorIndex(1);
}

void drawXcf(uint8_t x, uint8_t y) {
  char xval = 32 + x * 10, yval = 8 + y * 8;
  drawChar(xval, yval, 'x');
  /*
    u8g2.drawLine(31 + x * 10,      10 + y * 8,   31 + 5 + x * 10 ,   10 + 5 + y * 8);
    u8g2.drawLine(31 + x * 10 + 5,  10 + y * 8,   31  + x * 10,       10 + 5 + y * 8);*/
}

void drawOcf(uint8_t x, uint8_t y) {
  char xval = 32 + x * 10, yval = 8 + y * 8;
  drawChar(xval, yval, 'o');
  /*
    u8g2.drawCircle(31 + 10 + x * 10, 10 + 8 + y * 8, 3);*/
}


void fillRectangle(uint8_t x, uint8_t y) {
  u8g2.drawBox(24 + x * JUMP_X, 2 + y * JUMP_Y, 24, 18);
}

void drawCursorTicTac(uint8_t x, uint8_t y) {
  if (!(tic_tac_state[x + 3 * y] == 3 || tic_tac_state[x + 3 * y] == 4)) {
    u8g2.drawBox(FIRST_X + x * JUMP_X, FIRST_Y + y * JUMP_Y + X_HEIGHT - 3, X_WIDTH, CURSOR_HEIGHT);
  }
}

void drawCursorConnectFour(uint8_t x) {
  u8g2.drawTriangle(31 + x * CONNECT4_JUMP_X, 3, 37 + x * CONNECT4_JUMP_X, 3, 34 + x * CONNECT4_JUMP_X, 6);
}

void calculateCursorTicTac(uint8_t curr_loop) {
  switch (curr_loop) {
    case 0:
      if (curr_cursor_y > 0) {
        curr_cursor_y--;
      } else {
        curr_cursor_y = 2;
      }
      break;
    case 1:
      if (curr_cursor_y < 2) {
        curr_cursor_y++;
      } else {
        curr_cursor_y = 0;
      }
      break;
    case 2:
      break;
    case 3:
      if (curr_cursor_x > 0) {
        curr_cursor_x--;
      } else {
        curr_cursor_x = 2;
      }
      break;
    case 4:
      if (curr_cursor_x < 2) {
        curr_cursor_x++;
      } else {
        curr_cursor_x = 0;
      }
      break;
    default:
      break;
  }
}
void calculateCursorConnectFour(uint8_t curr_loop) {
  switch (curr_loop) {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      if (curr_cursor_x > 0) {
        curr_cursor_x--;
      } else {
        curr_cursor_x = 6;
      }
      break;
    case 4:
      if (curr_cursor_x < 6) {
        curr_cursor_x++;
      } else {
        curr_cursor_x = 0;
      }
      break;

    default:
      break;

  }

}
void calculateMenuSelection(uint8_t curr_loop) {
  switch (curr_loop) {
    case 3:
      if (curr_menu_item > 1) {
        curr_menu_item--;
      } else {
        curr_menu_item = 3;
      }
      break;
    case 4:
      if (curr_menu_item < 3) {
        curr_menu_item++;
      } else {
        curr_menu_item = 1;
      }
      break;
    default:
      break;
  }
}
void resetTicTac() {
  memset(tic_tac_state, 0, sizeof(tic_tac_state));
  curr_cursor_x = 0;
  curr_cursor_y = 0;
  curr_player = 1;
  score_x = 0;
  score_o = 0;
  will_display = 1;
}
void resetCF() {
  memset(connect_four_state, 0, sizeof(connect_four_state));
  curr_cursor_x = 0;
  curr_cursor_y = 0;
  curr_player = 1;
  score_x = 0;
  score_o = 0;
  will_display = 1;
}
void resetPingBoss() {
  bossHp = BOSS_MAX_HP;
  bossY = 32;
  p1pos = 32;
  p2pos = 32;

  will_display = 1;
}

void resetALL() {
  //Serial.println("RESET REQUESTED");
  general_state = MENU_STATE;
  prev_state = 0;
  curr_cursor_x = 0;
  curr_cursor_y = 0;
  curr_player = 1;
  score_x = 0;
  score_o = 0;

  char_count = 0;
  prev_char = ' ';
  curr_menu_item = 1;
  will_display = 1;

  received_c = -1, received_c_count = 0;
  reqMsgSelection = 1;

  memset(tic_tac_state, 0, sizeof(tic_tac_state));
  memset(connect_four_state, 0, sizeof(connect_four_state));
  memset(&to_send_pkt, 0, sizeof(to_send_pkt));
  memset(&received_pkt, 0, sizeof(received_pkt));
}

void drawTicTacBoard() {
  char i, j;

  u8g2.drawHLine(24, 21, 75);
  u8g2.drawHLine(24, 42, 75);
  u8g2.drawVLine(49, 2, 60);
  u8g2.drawVLine(74, 2, 60);
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 3; ++j) {
      if (tic_tac_state[i + 3 * j] == 1) {
        drawX(i, j, 0);
      } else if (tic_tac_state[i + 3 * j] == 2) {
        drawO(i, j, 0);
      } else if (tic_tac_state[i + 3 * j] == 3) {
        fillRectangle(i, j);
        drawX(i, j, 1);
      } else if (tic_tac_state[i + 3 * j] == 4) {
        fillRectangle(i, j);
        drawO(i, j, 1);
      }
    }
  }
  u8g2.drawVLine(16, 0, 31);
  u8g2.drawHLine(0, 10, 16);
  u8g2.drawHLine(0, 20, 16);
  u8g2.drawHLine(0, 31, 16);

  u8g2.drawVLine(111, 0, 31);
  u8g2.drawHLine(111, 10, 16);
  u8g2.drawHLine(111, 20, 16);
  u8g2.drawHLine(111, 31, 16);

  drawChar(1, 1, 'P');
  drawChar(8, 1, player_id + '0');
  drawChar(5, 12, player_id == 1 ? 'X' : 'O');
  drawChar(5, 23, (player_id == 1 ? score_x + '0' : score_o + '0' ));

  drawChar(114, 1, 'P');
  drawChar(121, 1, 3 - player_id + '0');
  drawChar(117, 12, player_id == 2 ? 'X' : 'O');
  drawChar(117, 23, (player_id == 2 ? score_x + '0' : score_o + '0'));

}

void drawChar(uint8_t x, uint8_t y, char c) {
  char toPrint[2] = "";
  toPrint[0] = c;
  toPrint[1] = '\0';
  u8g2.drawStr(x, y, toPrint);
}

void drawConnectFourBoard() {
  char i, j;

  /* Left side of player's info */
  u8g2.drawVLine(15, 0, 30);
  u8g2.drawHLine(0, 10, 15);
  u8g2.drawHLine(0, 20, 15);
  u8g2.drawHLine(0, 30, 15);
  /* Right side of player's info */
  u8g2.drawVLine(113, 0, 30);
  u8g2.drawHLine(113, 10, 14);
  u8g2.drawHLine(113, 20, 14);
  u8g2.drawHLine(113, 30, 14);
  /* Connect Four Board */
  u8g2.drawVLine(29, 8, 48);
  u8g2.drawVLine(39, 8, 48);
  u8g2.drawVLine(49, 8, 48);
  u8g2.drawVLine(59, 8, 48);
  u8g2.drawVLine(69, 8, 48);
  u8g2.drawVLine(79, 8, 48);
  u8g2.drawVLine(89, 8, 48);
  u8g2.drawVLine(99, 8, 48);

  u8g2.drawHLine(29, 8, 70);
  u8g2.drawHLine(29, 16, 70);
  u8g2.drawHLine(29, 24, 70);
  u8g2.drawHLine(29, 32, 70);
  u8g2.drawHLine(29, 40, 70);
  u8g2.drawHLine(29, 48, 70);
  u8g2.drawHLine(29, 56, 70);

  for (i = 0; i < 6; ++i) {
    for (j = 0; j < 7; ++j) {
      if (connect_four_state[i][j] == 1) {
        drawXcf(j, i);
      } else if (connect_four_state[i][j] == 2) {
        drawOcf(j, i);
      }
    }
  }
  drawChar(1, 1, 'P');
  drawChar(8, 1, player_id + '0');
  drawChar(5, 12, player_id == 1 ? 'X' : 'O');
  drawChar(5, 23, player_id == 1 ? score_x + '0' : score_o + '0');

  drawChar(116, 1, 'P');
  drawChar(122, 1, 3 - player_id + '0');
  drawChar(119, 12, player_id == 2 ? 'X' : 'O');
  drawChar(119, 23, player_id == 2 ? score_x + '0' : score_o + '0');

  char xval1 = 34 + received_pkt.data.connect_four.x1 * 10, yval1 = 11 + received_pkt.data.connect_four.y1 * 8;
  char xval2 = 34 + received_pkt.data.connect_four.x2 * 10, yval2 = 11 + received_pkt.data.connect_four.y2 * 8;

  if (received_pkt.data.connect_four.roundWon) {
    u8g2.drawLine(xval1, yval1, xval2, yval2);
    u8g2.drawLine(xval1 - 1, yval1, xval2 - 1, yval2);
  }

}

void drawMenu() {
  u8g2.drawHLine(0, 10, 16);
  u8g2.drawVLine(16, 0, 10);
  u8g2.drawRFrame(18, 23, 87, 19, 6);
  u8g2.drawTriangle(6, 33, 13, 28, 13, 38);
  u8g2.drawTriangle(116, 33, 109, 28, 109, 38);
  drawChar(1, 1, 'P');
  drawChar(8, 1, (char)(48 + player_id));
  u8g2.drawStr(24, 29, menu_item_name[curr_menu_item]);
}

void drawInfMsg() {
  //u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  u8g2.drawStr(5, 30, "Rakip bekleniyor...");
  //u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawWinMsg() {
  u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  u8g2.drawStr(18, 25, "Tebrikler");
  u8g2.drawStr(21, 40, "Kazandin!");

  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawLoseMsg() {
  u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  //u8g2.drawStr(10,30,"Kaybettin...");
  u8g2.drawStr(13, 25, "Bir Dahaki");
  u8g2.drawStr(21, 40, "Sefere...");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawDrawMsg() {
  u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  u8g2.drawStr(21, 30, "Berabere!");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawPingWinMsg() {
  u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  u8g2.drawStr(18, 13, "Tebrikler");
  u8g2.drawStr(8, 28, "Ping Boss'u");
  u8g2.drawStr(5, 43, "Alt ettiniz!");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawPingLoseMsg() {
  u8g2.setFont(u8g2_font_roentgen_nbp_tr);
  u8g2.drawStr(6,   13, "PING BOSS'UN");
  u8g2.drawStr(20,  28, "GAZABINA");
  u8g2.drawStr(13,   43, "UGRADINIZ!");
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawReqMsg() {
  //u8g2.setFont(u8g2_font_smart_patrol_nbp_tr);
  u8g2.setCursor(40, 10);
  u8g2.print("Oyuncu ");
  drawChar(85, 10, 3 - player_id + '0');
  u8g2.setCursor(30, 23);
  u8g2.print(menu_item_name[received_pkt.data.menu.curr_item]);

  u8g2.setCursor(20, 35);
  u8g2.print("Oynamak İstiyor");

  //u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.setCursor(12, 51);
  u8g2.print("Kabul Et");

  u8g2.setCursor(86, 51);
  u8g2.print("Reddet");

}

void drawReqMsgSelection() {
  if (reqMsgSelection == 1) {
    u8g2.drawTriangle(4, 51, 4, 57, 9, 54);
  } else if (reqMsgSelection == 2) {
    u8g2.drawTriangle(78, 51, 78, 57, 83, 54);
  }
}

void drawLogoBitmap() {
  u8g2.drawXBMP(0, 0, olsun_artik2_width, olsun_artik2_height, olsun_artik2_bits);
}

/*
  char bossHp = 0;
  char bossY = 0;
  char p1pos = 0;
  char p2pos = 0;
*/
void drawPlayers() {
  //uint8_t paddlePositionPlayer1 = 20, paddleHeight = 4;
  // P1 Offset 12, 5
  // P2 Offset  2, 5
  u8g2.drawXBMP(0, p1pos, left_player_width, left_player_height, left_player_bits);
  u8g2.drawXBMP(127 - right_player_width, p2pos, right_player_width, right_player_height, right_player_bits);

}

void drawBoss() {
  u8g2.drawXBMP(50, bossY, boss_width, boss_height, boss_bits);

  u8g2.setFont(u8g2_font_u8glib_4_tr);
  u8g2.drawStr(2, 2, "BOSS HP > ");
  u8g2.drawBox(40, 4, (80 / BOSS_MAX_HP) * bossHp, 3);
  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
}

void drawShots() {
  int shotx, shoty;

  for (int i = 0; i < 10; i++) {
    shotx = (unsigned int)received_pkt.data.ping_boss.bossBullets[i] >> 4;      //8 bitin soldaki 4 biti dört sağa shift edilerek alınıyor.
    shoty = received_pkt.data.ping_boss.bossBullets[i] & 0b00001111; //8 bitin sağdaki 4 biti soldaki 4 biti 0000 ile and lenerek alınıyor.
    if (shotx != 0) {
      u8g2.drawBox(shotx * 4, shoty * 4, 3, 2); //her bir x ve y koordinatı 4 birim atlıyor.
    }
  }


  for (int i = 0; i < 5; i++) {
    shotx = (unsigned int)received_pkt.data.ping_boss.p1Bullets[i] >> 4;      //8 bitin soldaki 4 biti dört sağa shift edilerek alınıyor.
    shoty = received_pkt.data.ping_boss.p1Bullets[i] & 0b00001111; //8 bitin sağdaki 4 biti soldaki 4 biti 0000 ile and lenerek alınıyor.
    if (shotx != 0) {
      u8g2.drawBox(shotx * 4, shoty * 4, 3, 2); //her bir x ve y koordinatı 4 birim atlıyor.
    }
    shotx = (unsigned int)received_pkt.data.ping_boss.p2Bullets[i] >> 4;      //8 bitin soldaki 4 biti dört sağa shift edilerek alınıyor.
    shoty = received_pkt.data.ping_boss.p2Bullets[i] & 0b00001111; //8 bitin sağdaki 4 biti soldaki 4 biti 0000 ile and lenerek alınıyor.
    if (shotx != 0) {
      u8g2.drawBox(shotx * 4, shoty * 4, 3, 2); //her bir x ve y koordinatı 4 birim atlıyor.
    }
  }

}
/*************************** SETUP *************************/
void setup()
{
  //Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  BTserial.begin(baudRate);

  //  Serial.println(F("Program boot..."));

  rec_ptr = (uint8_t *)&received_pkt;

  delay(100);  // This delay is needed to let the display to initialize
  u8g2.begin();

  u8g2.setFont(u8g2_font_synchronizer_nbp_tr);
  u8g2.setFontPosTop();
  u8g2.enableUTF8Print();

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
  char i = 0, j = 0, counter = 0;

  if (BTserial.available() && received_c_count < 32) {
    received_c = BTserial.read();
    if (received_c == 127) {
      received_c_count = 0;
      rec_ptr = (uint8_t *)&received_pkt;
    }
    *rec_ptr = (uint8_t)received_c;
    rec_ptr++;
    received_c_count++;
  } else if (received_c_count == 32) {
    will_display = 1;
    rec_ptr = (uint8_t *)&received_pkt;
    for (i = 0; i < 32; ++i) {
    }
    rec_ptr = (uint8_t *)&received_pkt;

    // RESET LOGIC
    if (received_pkt.app_id == RESET_REQUEST_ID) {
      //TODO reset logic
      resetALL();
    } else {
      switch (general_state) {
        case INITIAL_STATE:
          player_id = received_pkt.data.initial.player_id;
          general_state = MENU_STATE;
          break;
        case MENU_STATE:
          general_state = received_pkt.app_id;
          curr_menu_item = received_pkt.data.menu.curr_item;
          break;
        case XOX_STATE:
          curr_player = received_pkt.data.xox.turn;
          for (i = 0; i < 9; ++i) {
            tic_tac_state[i] = received_pkt.data.xox.cell_states[i];
          }
          score_x = received_pkt.data.xox.scoreP1;
          score_o = received_pkt.data.xox.scoreP2;
          if (received_pkt.app_id == 9 || received_pkt.app_id == 10 || received_pkt.app_id == 11) {
            general_state = received_pkt.app_id;
          }
          break;
        case CONNECT4_STATE:
          curr_player = received_pkt.data.connect_four.turn;
          //Serial.println(F("CONNECT4 Packet received from server"));
          for (i = 0; i < 6; ++i) {
            for (j = 0; j < 7; ++j) {

              if ((received_pkt.data.connect_four.p1_cells[i] & 0x80) != 0) {
                connect_four_state[i][j] = 1;
              }
              if (((received_pkt.data.connect_four.p1_cells[i] & 0x80) == 0) && ((received_pkt.data.connect_four.p2_cells[i] & 0x80) == 0)) {
                connect_four_state[i][j] = 0;
              }
              if ((received_pkt.data.connect_four.p2_cells[i] & 0x80) != 0) {
                connect_four_state[i][j] = 2;
              }
              received_pkt.data.connect_four.p1_cells[i] = received_pkt.data.connect_four.p1_cells[i] << 1;
              received_pkt.data.connect_four.p2_cells[i] = received_pkt.data.connect_four.p2_cells[i] << 1;
            }
          }
          score_x = received_pkt.data.connect_four.scoreP1;
          score_o = received_pkt.data.connect_four.scoreP2;
          if (received_pkt.app_id == 9 || received_pkt.app_id == 10 || received_pkt.app_id == 11) {
            general_state = received_pkt.app_id;
          }
          break;
        case PINGBOSS_STATE:
          bossHp = received_pkt.data.ping_boss.bossHp;
          bossY = received_pkt.data.ping_boss.bossPos;
          p1pos = received_pkt.data.ping_boss.p1pos;
          p2pos = received_pkt.data.ping_boss.p2pos;
          if (received_pkt.app_id == 12 || received_pkt.app_id == 13) {
            general_state = received_pkt.app_id;
          }
          break;
        case INFMSG_STATE:
          if (received_pkt.data.menu.accepted == 1) {
            general_state = received_pkt.data.menu.curr_item;
          } else if (received_pkt.data.menu.accepted == 2) {
            general_state = MENU_STATE;
          }
          break;
        case REQMSG_STATE:
          if (received_pkt.data.menu.accepted == 1) {
            general_state = received_pkt.data.menu.curr_item;
          } else if (received_pkt.data.menu.accepted == 2) {
            general_state = MENU_STATE;
          }
          break;
        case WIN_STATE:
        //Same with DRAW_STATE thus no break
        case LOSE_STATE:
        //Same with DRAW_STATE thus no break
        case DRAW_STATE:
          if (received_pkt.app_id == MENU_STATE) {
            general_state = MENU_STATE;
            resetTicTac();
            resetCF();
          }
          break;
        case PING_WIN_STATE:
        //Same with DRAW_STATE thus no break
        case PING_LOSE_STATE:
          //Same with DRAW_STATE thus no break
          if (received_pkt.app_id == MENU_STATE) {
            general_state = MENU_STATE;
            resetPingBoss();
          }
          break;
        default:
          break;
      }
    }
    received_c_count = 0;
  }


  for (i = 0; i < BTN_COUNT; ++i) {
    btn_state_arr[i] = digitalRead(B_U + i);
  }

  /*
    i = 0 -> up     button
    i = 1 -> down   button
    i = 2 -> mid    button
    i = 3 -> left   button
    i = 4 -> right  button
  */
  for (i = 0; i < BTN_COUNT; ++i) {
    if ( (millis() - last_dbn_arr[i]) > DEBOUNCE_DELAY) {
      if (btn_state_arr[i] == LOW) {
        if (i == 2 && !mid_pressed_flag) {
          mid_pressed_ms = millis();
          mid_pressed_flag  = 1;
        }
        // Mid Button Long Pressed
        if ((i == 2) && ((millis() - mid_pressed_ms) >= MID_HOLD_MAX)) {
          memset(&to_send_pkt, 0, sizeof(to_send_pkt));
          to_send_pkt.start = 127;
          to_send_pkt.app_id = RESET_REQUEST_ID;
          send_packet(to_send_pkt);
          mid_pressed_flag  = 0;
        }
        if (btn_pressed_arr[i] == 0) {
          will_display = 1;
          btn_pressed_arr[i] = 1;
          switch (general_state) {
            case MENU_STATE:
              calculateMenuSelection(i);
              memset(&to_send_pkt, 0, sizeof(to_send_pkt));
              to_send_pkt.start = 127;
              to_send_pkt.app_id = 0;
              to_send_pkt.data.menu.curr_item = curr_menu_item;
              if (i == 2) {
                to_send_pkt.data.menu.is_selected = 1;
              }
              send_packet(to_send_pkt);
              break;
            case XOX_STATE:
              if (curr_player == player_id) {
                calculateCursorTicTac(i);
                memset(&to_send_pkt, 0, sizeof(to_send_pkt));
                to_send_pkt.start = 127;
                to_send_pkt.app_id = 1;
                to_send_pkt.data.xox.pos = curr_cursor_x + curr_cursor_y * 3;
                if (i == 2 && curr_player == player_id) {
                  to_send_pkt.data.xox.is_clicked = 1;
                  //Serial.println(F("Mid button pressed"));
                } else {
                  to_send_pkt.data.xox.is_clicked = 0;
                }
                send_packet(to_send_pkt);
              }
              break;
            case CONNECT4_STATE:
              if (curr_player == player_id) {
                calculateCursorConnectFour(i);
                memset(&to_send_pkt, 0, sizeof(to_send_pkt));
                if (i == 2 && curr_player == player_id && connect_four_state[0][curr_cursor_x] == 0) {
                  to_send_pkt.data.connect_four.is_clicked = 1;
                } else {
                  to_send_pkt.data.connect_four.is_clicked = 0;
                }
                to_send_pkt.start = 127;
                to_send_pkt.app_id = 2;


                to_send_pkt.data.connect_four.pos = curr_cursor_x;  // It is different from XOX's because cursor shows current cursor in (X axis).
                send_packet(to_send_pkt);
              }
              break;
            case REQMSG_STATE:
              if (i == 3) {
                reqMsgSelection = 1;
              } else if (i == 4) {
                reqMsgSelection = 2;
              }
              memset(&to_send_pkt, 0, sizeof(to_send_pkt));
              to_send_pkt.start = 127;
              to_send_pkt.app_id = 7;
              to_send_pkt.data.menu.cursor_ = reqMsgSelection;
              to_send_pkt.data.menu.curr_item = curr_menu_item;
              if (i == 2) {
                to_send_pkt.data.menu.accepted = reqMsgSelection;
              }
              send_packet(to_send_pkt);
              break;
            default:
              break;
          }
        }
        if (general_state == PINGBOSS_STATE && (millis() % 50 == 0)) {
          if (i == 0 || i == 1 || i == 2 ) {
            memset(&to_send_pkt, 0, sizeof(to_send_pkt));
            to_send_pkt.start = 127;
            to_send_pkt.app_id = PINGBOSS_STATE;
            to_send_pkt.data.ping_boss.pressedButton = i;
            send_packet(to_send_pkt);
          }
        }
        last_dbn_arr[i] = millis(); //set the current time
      } else {
        if ((i == 2) && ((millis() - mid_pressed_ms) < MID_HOLD_MAX)) {
          mid_pressed_flag  = 0;
        }
        btn_pressed_arr[i] = 0;
      }
    }
  }

  // Display logic
  if (will_display == 1) {
    u8g2.firstPage();
    do {
      switch (general_state) {
        case MENU_STATE:
          drawMenu();
          break;
        case XOX_STATE:
          drawTicTacBoard();
          if (curr_player == player_id) {
            drawCursorTicTac(curr_cursor_x, curr_cursor_y);
          }
          break;
        case CONNECT4_STATE:
          drawConnectFourBoard(); //TODO : Player's infos , left side and right must be filled. And draw only if that player's state.
          if (curr_player == player_id) {
            drawCursorConnectFour(curr_cursor_x);
          }
          break;
        case PINGBOSS_STATE:
          drawBoss();
          drawPlayers();
          drawShots();
          break;
        case INFMSG_STATE:
          drawInfMsg();
          break;
        case REQMSG_STATE:
          drawReqMsgSelection();
          drawReqMsg();
          break;
        case INITIAL_STATE:
          drawLogoBitmap();
          break;
        case WIN_STATE:
          drawWinMsg();
          break;
        case LOSE_STATE:
          drawLoseMsg();
          break;
        case DRAW_STATE:
          drawDrawMsg();
          break;
        case PING_WIN_STATE:
          drawPingWinMsg();
          break;
        case PING_LOSE_STATE:
          drawPingLoseMsg();
          break;
        default:
          break;
      }
    } while (u8g2.nextPage());
    will_display = 0;
  }
}
