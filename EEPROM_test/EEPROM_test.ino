#include <TimeLib.h>        //timekeeping https://github.com/PaulStoffregen/Time
#include <OneWire.h> 
#include <EEPROM.h>

//#define  SENOFF     8
//#define  SENBIT     0x80

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
  int16_t balance; // balance +-32967, 2 bytes
  time_t unix_time; // current POSIX time, 4 bytes
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

void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0

void setup() {
  #ifdef DEBUGGING
  Serial.begin(9600);
  #endif
  PRINTLNF("Debug on");
  Log log_data;

  int16_t i = 0;
  int16_t read_byte_pos = 0;
  byte sentinel = 0;
  
  ///////print all datas//////
  while(i <= EEPROM.length()) { // determine address to read/write data
    EEPROM.get(i, log_data);
    PRINT("unix_time is ", log_data.unix_time);
    PRINTLN(", i is ", i);
    sentinel = bitRead(EEPROM.read(i),7);
    if (sentinel !=  bitRead(EEPROM.read(i+ sizeof(Log)),7)) //compare first bit in structs (sentinels), if not equal - we find address!
      break;
    i+=sizeof(Log); //go to next record
  }
  log_data = Log();
  ///////print all datas//////

  i = 0;
  ////////find address///////
  while(i < EEPROM.length()) { // determine address to read data
    sentinel = bitRead(EEPROM.read(i),7);
    if (sentinel !=  bitRead(EEPROM.read(i+ sizeof(Log)),7)) //compare first bit in structs (sentinels), if not equal - we find address!
      break;
    i+=sizeof(Log); //go to next record
  }
  if (i > EEPROM.length()) {
    PRINTLN("reach end of EEPROM, go to the beginning, i is ",i);
    i = 0;
  }
  read_byte_pos = i;
  ////////find address///////
  
  ////////read///////
  EEPROM.get(read_byte_pos, log_data);
  //byte sentinel = bitRead(log_data.home_temp,7);
  bitWrite(log_data.home_temp,7,bitRead(log_data.home_temp,6)); //copy 6 bit to 7, its now sign bit
  PRINTLNBIN("sentinel bit is ", sentinel);
  PRINTLN("log_data.home_temp is ", log_data.home_temp);
  PRINTLN("log_data.heater_temp is ", log_data.heater_temp);
  PRINTLN("log_data.balance is ", log_data.balance);
  PRINTLN("log_data.unix_time year is ", year(log_data.unix_time));  
  PRINTLN("log_data.unix_time day is ", day(log_data.unix_time));  
  PRINTLN("log_data.unix_time hour is ", hour(log_data.unix_time));
  PRINTLN("log_data.unix_time minute is ", minute(log_data.unix_time));
  PRINTLN("log_data.unix_time second is ", second(log_data.unix_time));
  PRINTLN("log_data.unix_time is ", log_data.unix_time);
  ////////read///////
  
  ////////write///////
  int16_t write_byte_pos = 0;
  if (read_byte_pos >= EEPROM.length()-1) { //TODO test , go to the start of EEPROM
    write_byte_pos = 0; //read byte is 1023, write is 0
    sentinel ^= 1 ; //invert sentintel bit
  }
  else 
    write_byte_pos = read_byte_pos + sizeof(Log);
    
  if (log_data.unix_time == 0xFFFFFFFF) {// empty date = empty data, read
    write_byte_pos = read_byte_pos;
    PRINTLNBIN("sentinel bit is ", sentinel);
    sentinel ^= 1 ; //0b00000001 invert sentintel bit
    PRINTLNBIN("sentinel bit is ", sentinel);
  }
  PRINTLN("byte to read is ", read_byte_pos);
  PRINTLN("byte to write is ", write_byte_pos);
  return;
  setTime(log_data.unix_time);
  adjustTime(7); //add 7 sec
//  log_data = Log();
  log_data.home_temp = -18; //home temp must be <= 0b00111111, i.e. abs(home_temp) <= 63
  bitWrite(log_data.home_temp,7,sentinel); //set last bit, its sentinel (0b10000000)
  log_data.heater_temp = -20;
  log_data.balance = 47;
  log_data.unix_time = now();   
  EEPROM.put(write_byte_pos, log_data );
  PRINTLNF("writing done ");
  
  ////////write///////
  delay(10000);
  resetFunc(); //вызываем reset
}

void loop() {
  // put your main code here, to run repeatedly:

}
