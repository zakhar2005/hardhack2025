#include <Wire.h>
#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
int time = 0;
RCSwitch mySwitch = RCSwitch();
int flagMove = 0;
bool flagPaus = 0;
int Pos = 0;
int directionIterator = 0;
int timeIterator = 0;
uint8_t timeDisplay[4] = {0, 0, 0, 0}; // HHMM (без позиции для двоеточия)
String menu[4] = {"Start", "SetTime", "CheckTime", "Rage"};

void Menu(){
    lcd.setCursor(0,0);
    lcd.print(menu[0]);
    lcd.print(" ");
    lcd.print(menu[1]);
    lcd.setCursor(0,1);
    lcd.print(menu[2]);
    lcd.print(" ");
    lcd.print(menu[3]);
    lcd.blink();
}

// Функция проверки корректности времени
bool isValidTime() {
    // Проверка часов (первые два элемента)
    if (timeDisplay[0] > 2) return false;          // Первая цифра часов: 0-2
    if (timeDisplay[0] == 2 && timeDisplay[1] > 3) return false; // Максимум 23 часа
    if (timeDisplay[1] > 9) return false;          // Вторая цифра часов: 0-9
    
    // Проверка минут (последние два элемента)
    if (timeDisplay[2] > 5) return false;          // Первая цифра минут: 0-5
    if (timeDisplay[3] > 9) return false;          // Вторая цифра минут: 0-9
    
    return true;
}

// Функция для установки значения с проверкой
bool setTimeDigit(int position, int value) {
    if (position < 0 || position > 3) return false;
    
    int oldValue = timeDisplay[position];
    timeDisplay[position] = value;
    
    // Если время стало некорректным, восстанавливаем старое значение
    if (!isValidTime()) {
        timeDisplay[position] = oldValue;
        return false;
    }
    
    return true;
}

String direction() {
    int x = analogRead(A0);
    int y = analogRead(A1);
    String direction = "center";
    
    if(x > 750){
        direction = "Right";
    }
    else if (x < 250) {
        direction = "Left";
    }
    else if(y < 250) {
        direction = "Up";
    }
    else if(y > 750){
        direction = "Down";
    }
    return direction;
}

// Функция для вычисления позиции курсора на экране
int getCursorPosition() {
    if (directionIterator < 2) {
        return directionIterator; // Позиции 0,1 (часы)
    } else {
        return directionIterator + 1; // Позиции 3,4 (минуты) - +1 из-за двоеточия
    }
}

void updateScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time:");
    
    lcd.setCursor(0, 1);
    // Выводим часы, двоеточие и минуты
    lcd.print(timeDisplay[0]);
    lcd.print(timeDisplay[1]);
    lcd.print(":");
    lcd.print(timeDisplay[2]);
    lcd.print(timeDisplay[3]);
    
    // Показываем статус корректности времени
    lcd.setCursor(10, 1);
    if (isValidTime()) {
        lcd.print("OK");
    } else {
        lcd.print("ERR");
    }
}

void setup() {
    Serial.begin(9600);
    mySwitch.enableTransmit(3);
    pinMode(2, INPUT); // кнопка
    pinMode(11, OUTPUT);// ИК
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    Menu();
    
    // Устанавливаем курсор на первую позицию
    lcd.setCursor(0,0);
    lcd.blink();
}

void loop() {
    String dir = direction();
    switch(flagMove){
        case(0): //Menu
            if(dir == "Right"){
                if(Pos==0){
                    lcd.setCursor(6,0);
                    Pos=1;
                }
                else if(Pos==2){
                    lcd.setCursor(10,1);
                    Pos=3;
                }
            }
            else if(dir == "Left"){
                if(Pos==1){
                    lcd.setCursor(0,0);
                    Pos=0;
                }
                else if(Pos==3){
                    lcd.setCursor(0,1);
                    Pos=2;
                }
            }
        
            // Обработка движения вверх/вниз для изменения значений
            if(dir == "Up"){
                if(Pos==2){
                    lcd.setCursor(0,0);
                    Pos=0;
                }
                else if(Pos==3){
                    lcd.setCursor(6,0);
                    Pos=1;
                }
            }
            else if(dir == "Down"){
                if(Pos==0){
                    lcd.setCursor(0,1);
                    Pos=2;
                }
                else if(Pos==1){
                    lcd.setCursor(10,1);
                    Pos=3;
                }
            }
            
            if(digitalRead(2)==LOW){
                flagMove=Pos+1;
                lcd.clear();
            }
            break;

        case(1): //Start
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(menu[0]);
            time = timeDisplay[0]*1000+timeDisplay[1]*100+timeDisplay[2]*10+timeDisplay[3];
            for(int i=0;i<5;i++){
                mySwitch.send(time, 12);
                delay(100);
            }
            lcd.clear();
            flagMove=4;
        break;

        case(2): //SetTime
                // Обработка движения влево/вправо для выбора позиции
                if(flagPaus==0){
                    while(digitalRead(2) == LOW){
                    }
                    flagPaus =1;
                    updateScreen();
                }
            if(dir == "Right" && timeIterator < 3){
                timeIterator++;
                directionIterator++;
                lcd.setCursor(getCursorPosition(), 1);
                delay(300);
            }
            else if(dir == "Left" && timeIterator > 0){
                timeIterator--;
                directionIterator--;
                lcd.setCursor(getCursorPosition(), 1);
                delay(300);
            }
            
            // Обработка движения вверх/вниз для изменения значений
            if(dir == "Up"){
                if (setTimeDigit(timeIterator, (timeDisplay[timeIterator] + 1) % 10)) {
                    updateScreen();
                    lcd.setCursor(getCursorPosition(), 1);
                    lcd.blink();
                }
                delay(300);
            }
            else if(dir == "Down"){
                int newValue = timeDisplay[timeIterator] - 1;
                if (newValue < 0) newValue = 9;
                if (setTimeDigit(timeIterator, newValue)) {
                    updateScreen();
                    lcd.setCursor(getCursorPosition(), 1);
                    lcd.blink();
                }
                delay(300);
            }
            
            // Обработка кнопки для сброса или подтверждения
            if(digitalRead(2) == LOW ) {
                if (isValidTime()) {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Time set:");
                    lcd.setCursor(0, 1);
                    lcd.print(timeDisplay[0]);
                    lcd.print(timeDisplay[1]);
                    lcd.print(":");
                    lcd.print(timeDisplay[2]);
                    lcd.print(timeDisplay[3]);
                    delay(2000);
                    updateScreen();
                    lcd.setCursor(getCursorPosition(), 1);
                    lcd.blink();
                    lcd.clear();
                    Menu();
                    flagPaus=0;
                    flagMove=0;
                } else {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Invalid time!");
                    delay(1000);
                    updateScreen();
                    lcd.setCursor(getCursorPosition(), 1);
                    lcd.blink();
                }
                delay(300);
            }
        break;

        case(3): //CheckTime
                lcd.print("Time set:");
                lcd.setCursor(0, 1);
                lcd.print(timeDisplay[0]);
                lcd.print(timeDisplay[1]);
                lcd.print(":");
                lcd.print(timeDisplay[2]);
                lcd.print(timeDisplay[3]);
                delay(2000);
                lcd.clear();
                flagMove = 0;
                Menu();
        break;
        
        case(4)://Rage
                lcd.setCursor(0,0);
                lcd.print("KILL THEM ALL!!!");
                if(digitalRead(2)==LOW){
                    lcd.setCursor(0,1);
                    lcd.print("!!!!!!!!!!!!!!!!");
                    digitalWrite(11, HIGH);
                    delay(300);
                    digitalWrite(11, LOW);
                    lcd.setCursor(0,1);
                    lcd.print("                ");
                }
                if(dir=="Left"){
                    lcd.clear();
                    flagMove = 0;
                    Menu();
                }
        break;

    }
    
    // // Обработка движения влево/вправо для выбора позиции
    // if(dir == "Right" && timeIterator < 3){
    //     timeIterator++;
    //     directionIterator++;
    //     lcd.setCursor(getCursorPosition(), 1);
    //     delay(300);
    // }
    // else if(dir == "Left" && timeIterator > 0){
    //     timeIterator--;
    //     directionIterator--;
    //     lcd.setCursor(getCursorPosition(), 1);
    //     delay(300);
    // }
    
    // // Обработка движения вверх/вниз для изменения значений
    // if(dir == "Up"){
    //     if (setTimeDigit(timeIterator, (timeDisplay[timeIterator] + 1) % 10)) {
    //         updateScreen();
    //         lcd.setCursor(getCursorPosition(), 1);
    //         lcd.blink();
    //     }
    //     delay(300);
    // }
    // else if(dir == "Down"){
    //     int newValue = timeDisplay[timeIterator] - 1;
    //     if (newValue < 0) newValue = 9;
    //     if (setTimeDigit(timeIterator, newValue)) {
    //         updateScreen();
    //         lcd.setCursor(getCursorPosition(), 1);
    //         lcd.blink();
    //     }
    //     delay(300);
    // }
    // // Обработка кнопки для сброса или подтверждения
    // if(digitalRead(2) == LOW) {
    //     if (isValidTime()) {
    //         lcd.clear();
    //         lcd.setCursor(0, 0);
    //         lcd.print("Time set:");
    //         lcd.setCursor(0, 1);
    //         lcd.print(timeDisplay[0]);
    //         lcd.print(timeDisplay[1]);
    //         lcd.print(":");
    //         lcd.print(timeDisplay[2]);
    //         lcd.print(timeDisplay[3]);
    //         delay(2000);
    //         updateScreen();
    //         lcd.setCursor(getCursorPosition(), 1);
    //         lcd.blink();
    //     } else {
    //         lcd.clear();
    //         lcd.setCursor(0, 0);
    //         lcd.print("Invalid time!");
    //         delay(1000);
    //         updateScreen();
    //         lcd.setCursor(getCursorPosition(), 1);
    //         lcd.blink();
    //     }
    //     delay(300);
    // }
    
    // delay(100);
}