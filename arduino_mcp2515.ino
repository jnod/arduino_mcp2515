#include <SerialLCD.h>
#undef SPI
#include <SPI.h>
#include "message_utils.h"

#define AUTH  0
#define INT   9
#define SS    10
#define MOSI  11
#define MISO  12
#define SCK   13

#define DEVICE_ID 0x0002

void setupCAN();
void setupUI();
void checkCAN();
void updateUI();
char keyPadGet();
void getNumber(char Number[]);

static CanMessage message = {MTYPE_STANDARD_DATA, 0x0400 | DEVICE_ID, 0, 1, {0}};
static StatusMessage statusMessage = {DEVICE_ID, 0, 0};
static SerialLCD lcd(2,40,9600,RS232);

void sendStatus() {
  writeStatusMessage(message, statusMessage);
  mcp2515_loadTX0(&message);
  mcp2515_rtsTX0();
}

void setup() {
  setupCAN();
  setupUI();
}

void loop() {
  static int count = 0;
  checkCAN();
  updateUI();
}

void setupCAN() {
  pinMode(AUTH, OUTPUT);
  pinMode(INT, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(SS, OUTPUT);

  digitalWrite(AUTH, LOW);
  digitalWrite(SS, HIGH);
  SPI.begin();

  delay(100); // Allows the mcp2515 to initialize

  mcp2515_reset();
  mcp2515_configCNFn(CNF1_10MHZ_125KBIT, CNF2_10MHZ_125KBIT, CNF3_10MHZ_125KBIT);
  mcp2515_setCANINTE(0x03); // Inturrupt when a message is received
  mcp2515_setRXBnCTRL(0x60, 0x60); // Ignore filters, receive all messages
  // mcp2515_setMode(MODE_LOOPBACK); // Loopback sends messages to itself for testing
  mcp2515_setMode(MODE_NORMAL); // Normal mode allows communication over CAN

  mcp2515_clearCANINTF(0xFF);

  sendStatus();
}

void setupUI() {
  Serial.begin(9600);
  lcd.init();
  lcd.setContrast(40);
  lcd.setBacklightBrightness(8);
}

void checkCAN() {
  if (!digitalRead(INT)) {
    mcp2515_readRX0(&message);
    mcp2515_clearCANINTF(0xFF);

    if (message.eid == 0 && message.sid == 1) {
      // message is an authorization message
      AuthMessage authMessage;
      readAuthMessage(authMessage, message);

      if (authMessage.node_id == DEVICE_ID) {
        statusMessage.authorized = authMessage.authorized;

        if (statusMessage.authorized) {
          digitalWrite(AUTH, HIGH);
        } else {
          digitalWrite(AUTH, LOW);
        }
      }
    }
  }
}

uint32_t convertNum(char arr[], int l) {
  uint32_t result = 0;
  
  for (int i = 0; i < l; i++) {
    result = result*10 + (arr[i]-48);
  }

  return result;
}

#define PROMPT_USER 0
#define PROMPT_PASS 2
#define READ_USER 1
#define READ_PASS 3
#define MAX 100

void updateUI() {
  static int state = PROMPT_USER;
  static char user[10], pass[10];
  static int ul = 0, pl = 0, count = 0;
  static char key = 0, last = 0;

  if (state == PROMPT_USER) { // prompt for user id
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("Enter User ID:");
    lcd.setCursor(2,1);
    ul = 0;
    state = READ_USER;
  } else if (state == READ_USER) { // read user id
    key = keyPadGet();
    
    if (key != last) {
      count = 0;    
      last = key;
    } else if (count < MAX) {
      count++;
    } else if (key != 0 && count == MAX) {
      if (key == '#') {
        state = PROMPT_PASS;
      } else if (key == '*') {
        if (ul > 0) {
          ul--;
          lcd.setCursor(2,ul+1);
          lcd.print(' ');
          lcd.setCursor(2,ul+1);
        }
      } else if (ul < 9) {
        user[ul] = key;
        ul++;
        lcd.print(key);
      }

      count++;
    }
  } else if (state == PROMPT_PASS) { // prompt for password
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("Enter Password: ");
    lcd.setCursor(2,1);
    pl = 0;
    state = READ_PASS;
  } else if (state == READ_PASS) { // read password
    key = keyPadGet();
    
    if (key != last) {
      count = 0;    
      last = key;
    } else if (count < MAX) {
      count++;
    } else if (key != 0 && count == MAX) {
      if (key == '#') {
        state = PROMPT_USER;
        UserMessage userMessage = {DEVICE_ID, convertNum(user, ul), convertNum(pass, pl)};
        writeUserMessage(message, userMessage);
        mcp2515_loadTX0(&message);
        mcp2515_rtsTX0();
      } else if (key == '*') {
        if (pl > 0) {
          pl--;
          lcd.setCursor(2,pl+1);
          lcd.print(' ');
          lcd.setCursor(2,pl+1);
        }
      } else if (ul < 9) {
        pass[pl] = key;
        pl++;
        lcd.print(key);
      }

      count++;
    }
  } else { // invalid state, reset
    state = PROMPT_USER;
  } 
}

char keyPadGet() {
  char Key = 0;
  int  Val1 = analogRead(A1); 
  if(Val1 > 154 && Val1 < 200){ Key = '*';}
  else if(Val1 > 205 && Val1 < 233){ Key = '7';}
  else if(Val1 > 244 && Val1 < 284){ Key = '4';}
  else if(Val1 > 304 && Val1 < 344){ Key = '1';} 
  else if(Val1 > 363 && Val1 < 403){ Key = '0';} 
  else if(Val1 > 441 && Val1 < 481){ Key = '8';} 
  else if(Val1 > 515 && Val1 < 555){ Key = '5';} 
  else if(Val1 > 594 && Val1 < 634){ Key = '2';} 
  else if(Val1 > 656 && Val1 < 696){ Key = '#';} 
  else if(Val1 > 713 && Val1 < 753){ Key = '9';} 
  else if(Val1 > 768 && Val1 < 808){ Key = '6';} 
  else if(Val1 > 818 && Val1 < 858){ Key = '3';} 
  return Key;
}

void getNumber(char Number[]) {
  char Key = 0;
  static char Last = 0;
  for(int i=0; i <= 8; i++)
  {
    Key = keyPadGet();
    delay(100);
    
    if (Key == 0 || Key == Last) {
      i--;
      Last = Key;
      continue;
    }
    Last = Key;
    if(Key == '#')
    {
      break;
    }
    if(Key == '*')
    {
      lcd.setCursor(2,i);
      lcd.print(' ');
      i -= 2;  // i = i-2
      continue;
    }
    Number[i] = Key;
    lcd.setCursor(2,i+1);
    lcd.print(Key);
  }
}

void mcp2515_spiTransfer(uint8_t *buf, uint8_t len) {
  digitalWrite(SS, LOW);
  SPI.transfer(buf, len);
  digitalWrite(SS, HIGH);
}
