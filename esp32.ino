#include <Alash_DS1302.h>
#include "Alash_DS1302_Alarm.h"
#include "melody.h"
#include <RCSwitch.h>



#define RX_PIN 23  // пин ESP32, куда подключен DATA RX 433 МГц

#define IR_PIN 4  // пин, куда подключен датчик KY-022

// Подключение пина данных DS1302 к цифровому пину Arduino
#define DS1302_CLK_PIN      27
#define DS1302_IO_PIN       26
#define DS1302_CE_PIN       25


#define HOUR_ALARM 22
#define MINUTE_ALARM 24

// Создание объекта RTC
Alash_DS1302 rtc = Alash_DS1302(DS1302_CLK_PIN, DS1302_IO_PIN, DS1302_CE_PIN);

// радиоприемник
RCSwitch mySwitch = RCSwitch();

// Флаг активности будильника (был ли он заведен)
uint8_t alarmFlag = 0;

// Было ли время будильника введено.
// Устанавливается когда с 433 пришло время будильника
uint8_t timeSetFlag = 0;

// Время будильника
uint16_t timeAlarm = 0;

uint8_t hoursAlarm = 0;
uint8_t minutesAlarm = 0;

// Проверить сигнал с ИК порта
bool irCheck(){
  int irSignal = digitalRead(IR_PIN);

  if (irSignal == LOW) {  // сигнал активен
    Serial.print("SHOT");
    return 1;
  }
  return 0;
}

// Проверка сработки будилника
void alarmCheck(uint8_t hourAlarm, uint8_t minuteAlarm){
    
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  rtc.getTime(&hour, &minute, &second);
    if(hour == hourAlarm && minute == minuteAlarm && alarmFlag == 1){
      Serial.println("БУДИЛЬНИК");
      playMelody();

      if(irCheck()){
        alarmFlag = 0;
        timeSetFlag = 0;
        return;
      }

    }
  

  delay(1000);
}

// Опрос радиоприемника на наличие времени будильника
// Если оно еще не было введено, и на порту есть данные,
// Сохранить это значение
void timeSetCheckRX(){
  if (mySwitch.available() && timeSetFlag != 1) {
  
    timeAlarm = mySwitch.getReceivedValue();
    timeSetFlag = 1;
    Serial.println("Received ");
    Serial.print(mySwitch.getReceivedValue());
    mySwitch.resetAvailable();
    playTone(2000, 1000);
  }
}

// Разбиение значения времени на часы и минуты
// Устанавливается флаг заведения будильника
bool parseAlarmTime(){
  if(timeSetFlag == 1 && alarmFlag == 0){
    hoursAlarm = timeAlarm / 100;
    Serial.println(hoursAlarm);

    minutesAlarm = timeAlarm % 100;
    Serial.println(minutesAlarm);

    alarmFlag = 1;

    delay(100);
  }
  return alarmFlag;
}



// Вывод времени (часы, минуты, секунды)
void printCurrentTime(uint8_t hour, uint8_t minute, uint8_t second){
    char buf[10];

    // Вывод времени
    snprintf(buf, sizeof(buf), "%d:%02d:%02d", hour, minute, second);
    Serial.println(buf);
}

// Вывод времени каждую секунду
void printTimeLoop(){

    static uint8_t secondLast = 0xff;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

// Чтение времени RTC
    if (!rtc.getTime(&hour, &minute, &second)) {
        Serial.println(F("Ошибка: чтение DS1302 не удалось"));
    } else {
        // Вывод времени RTC каждую секунду
        if (second != secondLast) {
            secondLast = second;

            // Вывод времени RTC
            printCurrentTime(hour, minute, second);
  

        }
    }
}



void setup() {
    
  // Встроенный светодиод
  pinMode(2, OUTPUT);

  delay(500);
  
  // Инициализация монитора
  Serial.begin(115200);


  // Инициализация RTC
  while (!rtc.begin()) {
    Serial.println(F("RTC not found"));
    delay(3000);
  }
  
  // Инициализация динамика
  initBuzzer();
  
  // используем прерывание для радиоприемника
  mySwitch.enableReceive(digitalPinToInterrupt(RX_PIN));  

  delay(2000);

  
  // Включение RTC
  rtc.clockEnable(true);
}

void loop(){


  timeSetCheckRX();
  parseAlarmTime();

  if(alarmFlag){
    // Проверка будильника по указанному времени
    alarmCheck(hoursAlarm, minutesAlarm);
  };


  // Вывод текущего времени в порт
  printTimeLoop();

    // Ожидание 100 мс
  delay(100);
}