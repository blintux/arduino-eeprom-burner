/*
 * *********************************************************************************
 * EEPROM DATA
 * *********************************************************************************
 */

/*
 * EEPROM content for write
 */
const PROGMEM uint32_t data[] = {
  'H', 'E', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', 
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
  0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 
  0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
 * EEPROM settings
 */
const int write_start = 0;          // Write start address
const int read_start  = 256;          // Read start address
const int read_end    = 512;        // Read end address
const int erase_end   = 512;        // Erase end address

/*
 * *********************************************************************************
 * EEPROM WRITER PROGRAM
 * *********************************************************************************
 */
 
/*
 * Variables
 */
#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

int command;
int shift;

/*
 * Set EEPROM Address
 */
void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

/*
 * Write a byte to the EEPROM at the specified address.
 */
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

/*
 * Read a byte from the EEPROM at the specified address.
 */
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

/*
 * Init the EEPROM Writer
 */
void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);

  Serial.println("COMMANDS:");
  Serial.println("Write: w   Erase: e   Read: r \n");
}

/*
 * Program loop
 */
void loop() {
  if (Serial.available() > 0) {
    command = Serial.read();

    switch (command) {
      
      //Erase EEPROM
      case 'e':
        Serial.print("Erasing EEPROM .");
        
        for (int address = 0; address <= erase_end; address += 1) {
          writeEEPROM(address, 0xff);
          if (address % 64 == 0) Serial.print(".");
        }

        Serial.print(" Done!\n\n");
      break;

      //Read EEPROM
      case 'r':
        Serial.print("Reading EEPROM ... \n\n");
      
        for (int base = read_start; base <= read_end; base += 16) {
          byte data[16];
          for (int offset = 0; offset <= 15; offset += 1) {
            data[offset] = readEEPROM(base + offset);
          }
      
          char buf[80];
          sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                  base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                  data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
      
          Serial.println(buf);
        }
      break;

      // Write all EEPROM
      case 'w':
        Serial.print("Writeing EEPROM .");

        for (int address = 0; address < sizeof(data)/sizeof( pgm_read_dword(data) ); address += 1) {
          writeEEPROM(address + write_start, pgm_read_dword(data + address) );
          if (address % 64 == 0) Serial.print(".");
        }

        Serial.print(" Done!\n\n");
      break;

    }
  }
}
