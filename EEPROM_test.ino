#include <TimeLib.h>        //timekeeping
#include <OneWire.h> 
#include <EEPROM.h>

#define EEPROMSIZE E2END

#define  SENOFF     8
#define  SENBIT     0x80

#define DEBUGGING
#ifdef DEBUGGING
#define PRINTLNF(s)       \
  {                       \
    Serial.println(F(s)); \
  }
#define PRINTLN(s, v)   \
  {                     \
    Serial.print(F(s)); \
    Serial.println(v);  \
  }
#define PRINTLNBIN(s, v)   \
  {                     \
    Serial.print(F(s)); \
    Serial.println(v,BIN);  \
  }  
#define PRINTLNHEX(s, v)    \
  {                         \
    Serial.print(F(s));     \
    Serial.println(v, HEX); \
  }
#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(v);    \
  }
#else
#define PRINTLNF(s)
#define PRINTLN(s, v)
#define PRINTLNHEX(s, v)
#define PRINT(s, v)
#endif

struct Log {
  int8_t home_temp; // temp +- 63, 1 byte, and sentinel, see this https://sites.google.com/site/dannychouinard/Home/atmel-avr-stuff/eeprom-longevity
  int8_t heater_temp; // heater +- 128, 1 byte
  int16_t balance; // balance +-32967, 2 byte
  time_t unix_time; // current POSIX time, 4 byte
  }; //total 8 bytes (64 bits)




void eepromRead(uint16_t addr, void* output, uint16_t length) {
    uint8_t* src; 
    uint8_t* dst;
    src = (uint8_t*)addr;
    dst = (uint8_t*)output;
    for (uint16_t i = 0; i < length; i++) {
        *dst++ = eeprom_read_byte(src++);
    }
}

void eepromWrite(uint16_t addr, void* input, uint16_t length) {
    uint8_t* src; 
    uint8_t* dst;
    src = (uint8_t*)input;
    dst = (uint8_t*)addr;
    for (uint16_t i = 0; i < length; i++) {
        eeprom_write_byte(dst++, *src++);
    }
}

void setup() {
  #ifdef DEBUGGING
  Serial.begin(9600);
  #endif
  PRINTLNF("Debug on");
  Log log_data;
//  setTime(12,52,28,20,12,19);
//  log_data.home_temp = -43;
//  log_data.heater_temp = -34;
//  log_data.balance = 57;
//  log_data.unix_time = now();   
//  EEPROM.put( 0, log_data );

//  setTime(12,59,59,31,12,19);
//  log_data.home_temp = -23;
//  log_data.heater_temp = -34;
//  log_data.balance = 57;
//  log_data.unix_time = now();   
//  EEPROM.put( sizeof(Log), log_data );

//  setTime(13,00,00,01,01,20);
//  log_data.home_temp = 23;
//  log_data.heater_temp = -34;
//  log_data.balance = 57;
//  log_data.unix_time = now();   
//  EEPROM.put( sizeof(Log)*5, log_data );
  
  log_data = Log(); 
//  EEPROM.get( 0, log_data );
//
//  PRINTLNBIN("sentinel bit is ", bitRead(log_data.home_temp,7));
//  PRINTLN("log_data.home_temp is ", log_data.home_temp);
//  PRINTLN("log_data.heater_temp is ", log_data.heater_temp);
//  PRINTLN("log_data.balance is ", log_data.balance);
//  PRINTLN("log_data.unix_time day is ", day(log_data.unix_time));  
//  PRINTLN("log_data.unix_time hour is ", hour(log_data.unix_time));
//  PRINTLN("log_data.unix_time minute is ", minute(log_data.unix_time));
//  PRINTLN("log_data.unix_time second is ", second(log_data.unix_time));
//
//  log_data = Log(); 
//  EEPROM.get( sizeof(Log), log_data );
//
//  PRINTLNBIN("sentinel bit is ", bitRead(log_data.home_temp,7));
//  PRINTLN("log_data.home_temp is ", log_data.home_temp);
//  PRINTLN("log_data.heater_temp is ", log_data.heater_temp);
//  PRINTLN("log_data.balance is ", log_data.balance);
//  PRINTLN("log_data.unix_time day is ", day(log_data.unix_time));  
//  PRINTLN("log_data.unix_time hour is ", hour(log_data.unix_time));
//  PRINTLN("log_data.unix_time minute is ", minute(log_data.unix_time));
//  PRINTLN("log_data.unix_time second is ", second(log_data.unix_time));

//  EEPROM.get( sizeof(Log)*2, log_data );
//  PRINTLNBIN("sentinel bit is ", bitRead(log_data.home_temp,7));
//  PRINTLN("log_data.home_temp is ", log_data.home_temp);
//
//  EEPROM.get( sizeof(Log)*5, log_data );
//  PRINTLNBIN("sentinel bit is ", bitRead(log_data.home_temp,7));
//  PRINTLN("log_data.home_temp is ", log_data.home_temp);
  
//  return;
//    PRINTLN("first byte is ", (int8_t) sentinel);
//    PRINTLNBIN("sentinel bit is ", bitRead(sentinel,7));

//    PRINTLN("sizeof Log is ", sizeof(Log));
//    PRINTLN("sizeof Log is ", sizeof(log_data));
//    PRINTLN("next byte is ", sentinel2);
//    PRINTLNBIN("sentinel next bit is ",  bitRead(sentinel2,7));
//  int i = 0;
//  while(i < EEPROM.length()) { // determine address to write data
//    EEPROM.update(i, 255);
//    PRINTLN("erase EEPROM done! ",i);
//    i++;
//  }




  ////////find address///////
  int16_t i = 0;
  int16_t read_byte_pos;
  int16_t write_byte_pos;
  
  while(i < EEPROM.length()) { // determine address to read/write data
    if (bitRead(EEPROM.read(i),7) !=  bitRead(EEPROM.read(i+ sizeof(Log)),7)) //compare first bit in structs, if not equal - we find address!
      break;
    i+=sizeof(Log); //go to next record
  }
  if (i >= EEPROM.length()) {
    PRINTLN("reach end of EEPROM, go to the beginning, i is ",i);
    i = 0;
  }

  read_byte_pos = i;
  if (i == EEPROM.length()-1) 
    write_byte_pos = 0; //go to the start of EEPROM
  else 
    write_byte_pos = i + sizeof(Log);
  
  PRINTLN("byte to read is ", read_byte_pos);
  PRINTLN("byte to write is ", write_byte_pos);
  ////////find address///////


  
  ////////read///////
  EEPROM.get(read_byte_pos, log_data);
  byte sentinel = bitRead(log_data.home_temp,7);
  PRINTLNBIN("sentinel bit is ", sentinel);
  log_data.home_temp &=~0x80; //clear last bit, its sentinel (0b10000000)
  PRINTLN("log_data.home_temp is ", log_data.home_temp);
  PRINTLN("log_data.heater_temp is ", log_data.heater_temp);
  PRINTLN("log_data.balance is ", log_data.balance);
  PRINTLN("log_data.unix_time day is ", day(log_data.unix_time));  
  PRINTLN("log_data.unix_time hour is ", hour(log_data.unix_time));
  PRINTLN("log_data.unix_time minute is ", minute(log_data.unix_time));
  PRINTLN("log_data.unix_time second is ", second(log_data.unix_time));
  ////////read///////
  return;



  
  ////////find real data, is anything recorded?///////
  for(int j=i;j<i+sizeof(Log);j++) {
    byte value = EEPROM.read(j);
    if (value!=255) // maybe something recorded
      break;
      
  }
  PRINTLN("oops, byte to write is ", i);  
  ////////find real data///////
  
  ////////write///////
  setTime(01,01,01,01,01,20);
  log_data.home_temp = -43;
  if (abs(log_data.home_temp) > 60 ) {
    //error! wrong temp  
  }
  
  bitWrite(log_data.home_temp,7,sentinel); //set last bit, its sentinel (0b10000000)
//  log_data.home_temp &=~0x80; 
  log_data.heater_temp = -34;
  log_data.balance = 57;
  log_data.unix_time = now();   
  EEPROM.put( i + sizeof(Log), log_data );
  ////////write///////




  
}

void loop() {
  // put your main code here, to run repeatedly:

}
