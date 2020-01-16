#include <TimeLib.h>        //timekeeping https://github.com/PaulStoffregen/Time
#include <EEPROM.h>

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

void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0

int16_t addr_to_read() {
  int16_t read_byte_pos = 0;
  while(read_byte_pos < EEPROM.length()-sizeof(Log)) { // determine address to read data
    if (bitRead(EEPROM.read(read_byte_pos),7) !=  bitRead(EEPROM.read(read_byte_pos+sizeof(Log)),7)) //compare first bit in structs (sentinels), if not equal - we find address!
      break;
    read_byte_pos+=sizeof(Log); //go to next record
  }
//  if (read_byte_pos >= EEPROM.length()-sizeof(Log)) {
//    PRINTLN("reach end of EEPROM, go to the beginning, read_byte_pos is ",read_byte_pos);
//    read_byte_pos = 0;
//  }
  return read_byte_pos;
}

void setup() {
  #ifdef DEBUGGING
  Serial.begin(9600);
  #endif
  PRINTLNF("Debug on");
  uint16_t i = 0;

//  while(i < EEPROM.length()) {
//    EEPROM.write(i,255);
//    i++;
//  }
//  PRINTLNF("Clear EEPROM done!");
//  return;
  
  ///////print all datas//////
  
  Log log_data;
  while(i < EEPROM.length()) { // determine address to read/write data
    EEPROM.get(i, log_data);
    PRINT("unix_time is ", log_data.unix_time);
    PRINT(", address to read from is ", i);
    PRINTLN(", sentinel is ", bitRead(EEPROM.read(i),7));
    if (bitRead(EEPROM.read(i),7) !=  bitRead(EEPROM.read(i+sizeof(Log)),7)) //compare first bit in structs (sentinels), if not equal - we find address!
      break;
    i+=sizeof(Log); //go to next record
  }
  ///////print all datas//////


  int16_t read_byte_pos = addr_to_read(); //find EEPROM address to read from
  PRINTLN("EEPROM_addr_to_read is ",read_byte_pos);  
//  log_data;
  EEPROM.get(read_byte_pos, log_data); //read EEPROM data
  byte sentinel = bitRead(log_data.home_temp,7); //read sentinel bit

  time_t previous_datetime = log_data.unix_time;
  if (log_data.unix_time == 0xFFFFFFFF) {//check data, if empty date = empty data, nothing to read
    previous_datetime = 0; //no data - send SMS and write to EEPROM
    sentinel ^= 1 ; // invert sentintel bit
  }
  setTime(1599171670);
  if (elapsedDays(now()) - elapsedDays(previous_datetime) >= 80) { //if more than 80 days have passed from last sending, send new SMS
    //write datetitme (data struct) to EEPROM
    int16_t write_byte_pos = read_byte_pos + sizeof(Log);
    if (write_byte_pos >= EEPROM.length()) { //reaching end of EEPROM go to begining (to 0), to the start of EEPROM
      PRINTLNF("reaching end of EEPROM go to begining (to 0), to the start of EEPROM");
      write_byte_pos = 0;
      if (log_data.unix_time != 0xFFFFFFFF)
        sentinel ^= 1 ; //invert sentintel bit
    }
    log_data.home_temp = 1; //home temp must be <= 0b00111111, i.e. abs(home_temp) <= 63
    bitWrite(log_data.home_temp,7,sentinel); //set last bit, its sentinel (0b10000000)
    log_data.heater_temp = 2;
    log_data.balance = 51;
    log_data.unix_time = now();   
//    EEPROM.put(write_byte_pos, log_data );
    PRINTLN("sentinel ", sentinel);
    PRINTLN("write_byte_pos ", write_byte_pos);
    PRINTLNF("Writing to EEPROM done!");
  }
  return;
  
  read_byte_pos = addr_to_read(); //find address to read
  PRINTLN("EEPROM_addr_to_read is ",read_byte_pos);
  
  ////////read///////
  EEPROM.get(read_byte_pos, log_data);
  sentinel = bitRead(log_data.home_temp,7);
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
  write_byte_pos = read_byte_pos + sizeof(Log);
  if (write_byte_pos >= EEPROM.length()) { //TODO test , go to the start of EEPROM
    PRINTLN("oops! reaching end of EEPROM go to begining (to 0), write_byte_pos is ", write_byte_pos);
    write_byte_pos = 0; //read bytes is 1016+8, write is 0
    PRINTLNBIN("sentinel bit is ", sentinel);
    sentinel ^= 1 ; //invert sentintel bit
    PRINTLNBIN("sentinel bit is ", sentinel);
  }
//  if (log_data.unix_time == 0xFFFFFFFF) {//check data, empty date = empty data
//    write_byte_pos = read_byte_pos;
//    PRINTLNBIN("sentinel bit is ", sentinel);
//    sentinel ^= 1 ; // invert sentintel bit
//    PRINTLNBIN("invert sentintel bit now ", sentinel);
//  }
  PRINTLN("byte to read is ", read_byte_pos);
  PRINTLN("byte to write is ", write_byte_pos);
  return;

  ////////write///////
  setTime(log_data.unix_time);
  adjustTime(7); //add 7 sec
  log_data.home_temp = -18; //home temp must be <= 0b00111111, i.e. abs(home_temp) <= 63
  bitWrite(log_data.home_temp,7,sentinel); //set last bit, its sentinel (0b10000000)
  log_data.heater_temp = -20;
  log_data.balance = 47;
  log_data.unix_time = now();   
  EEPROM.put(write_byte_pos, log_data );
  PRINTLNF("writing done ");
  
  
  delay(5000);
  resetFunc(); //вызываем reset
}

void loop() {
  // put your main code here, to run repeatedly:

}
