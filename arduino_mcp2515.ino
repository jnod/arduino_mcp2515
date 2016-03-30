#include <SerialLCD.h>
#undef SPI
#include <SPI.h>

extern "C" {
  #include "mcp2515.h"
}

#define AUTH  0
#define INT   9
#define SS    10
#define MOSI  11
#define MISO  12
#define SCK   13

#define DEVICE_ID 0x0005

void setupCAN();
void setupUI();
void checkCAN();
void updateUI();
char keyPadGet();
void getNumber(char Number[]);

static CanMessage message = {MTYPE_STANDARD_DATA, 0x0400 | DEVICE_ID, 0, 1, {0}};
static SerialLCD lcd(2,40,9600,RS232);

void setup() {
  setupCAN();
  setupUI();
}

void loop() {
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

  mcp2515_loadTX0(&message);
  mcp2515_rtsTX0();
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

    static int authorized = 0;

    if (authorized) {
      digitalWrite(AUTH, LOW);
      authorized = 0;
    } else {
      digitalWrite(AUTH, HIGH);
      authorized = 1;
    }

    if (message.mtype == MTYPE_STANDARD_DATA && message.sid == 1) {
      uint8_t* data = &message.data[0];
      int device_id = data[0] << 3 | data[1] << 2 | data[2] << 1 | data[3];

      if (device_id == DEVICE_ID) {
        if (data[4] == 0) {
          digitalWrite(AUTH, LOW);
        } else {
          digitalWrite(AUTH, HIGH);
        }
      }
    }
  }
}

void updateUI() {
  char Username[9];
  char Password[9];
  
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.println("Enter Username: ");
  getNumber(Username);
  lcd.setCursor(1,1);
  lcd.clear();
  lcd.println("Enter Password: ");
  getNumber(Password);
}

char keyPadGet() {
  char Key = 0;
  int  Val1 = analogRead(A1); 
  if(Val1 > 154 && Val1 < 194){ Key = '*';}
  if(Val1 > 193 && Val1 < 233){ Key = '7';}
  if(Val1 > 244 && Val1 < 284){ Key = '4';}
  if(Val1 > 304 && Val1 < 344){ Key = '1';} 
  if(Val1 > 363 && Val1 < 403){ Key = '0';} 
  if(Val1 > 441 && Val1 < 481){ Key = '8';} 
  if(Val1 > 515 && Val1 < 555){ Key = '5';} 
  if(Val1 > 594 && Val1 < 634){ Key = '2';} 
  if(Val1 > 656 && Val1 < 696){ Key = '#';} 
  if(Val1 > 713 && Val1 < 753){ Key = '9';} 
  if(Val1 > 768 && Val1 < 808){ Key = '6';} 
  if(Val1 > 818 && Val1 < 858){ Key = '3';} 
  delay(100);
  return Key;
}

void getNumber(char Number[]) {
  char Key = 0;
  static char Last = 0;
  for(int i=0; i <= 8; i++)
  {
    Key = keyPadGet();
    
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
