/*
 * *********************************************************************************
 * EEPROM DATA
 * *********************************************************************************
 */

/*
 * EEPROM content for write
 */

const PROGMEM uint32_t data[] =
{
  'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x61, 0x62, 0x63, 0x64, 
  0x65, 0x66, 0x67, 0x68, 0x20, 0x30, 0x31, 0x32, 0x33
  
};

/*
 * EEPROM settings
 */
const int write_start = 0;          // Write start address
const int read_start  = 0;          // Read start address
const int read_end    = 511;        // Read end address
const int erase_end   = 511;        // Erase end address

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
void setAddress(int address, bool outputEnable)
{
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

/*
 * Write a byte to the EEPROM at the specified address.
 */
void writeEEPROM(int address, byte data)
{
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
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
byte readEEPROM(int address)
{
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1)
  {
    pinMode(pin, INPUT);
  }
  
  setAddress(address, true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1)
  {
    data = (data << 1) + digitalRead(pin);
  }
  
  return data;
}

/*
 * Init the EEPROM Writer
 */
void setup()
{
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
void loop()
{
  if (Serial.available() > 0)
  {
    command = Serial.read();

    switch (command)
    {  
      //Erase EEPROM
      case 'e':
        Serial.print("Erasing EEPROM .");
        
        for (int addr = 0; addr <= erase_end; addr += 1)
        {
          writeEEPROM(addr, 0xff);
          if (addr % 64 == 0) Serial.print(".");
        }

        Serial.print(" Done!\n\n");
        break;

      //Read EEPROM
      case 'r':
        Serial.print("Reading EEPROM ... \n\n");
     
        for (int base = read_start; base <= read_end; base += 16)
        {
          byte data[16];
          char hex_buf[80];
          String str_buf;
          
          for (int offset = 0; offset <= 15; offset += 1)
          {
            data[offset] = readEEPROM(base + offset);
          }

          for (char c : data)
          {
            str_buf += (c >= ' ' && c < 128) ? c : '.';
          }

          sprintf(hex_buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x   ",
                  base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], 
                  data[10], data[11], data[12], data[13], data[14], data[15]);
          
          Serial.print(hex_buf);
          Serial.println(str_buf);
        }

        break;

      // Write all EEPROM
      case 'w':
        Serial.print("Writeing EEPROM .");

        for (int addr = 0; addr < sizeof(data)/sizeof( pgm_read_dword(data) ); addr += 1)
        {
          writeEEPROM(addr + write_start, pgm_read_dword(data + addr) );
          if (addr % 64 == 0) Serial.print(".");
        }

        Serial.print(" Done!\n\n");
        break;
    }
  }
}
