#ifndef MELODY_H
#define MELODY_H


#define BUZZER_PIN 33

// Ноты (частоты в Гц, подобраны для громкости на зуммерах 3–5 В)
#define NOTE_C4  2000   // вместо 262
#define NOTE_D4  2300   // вместо 294
#define NOTE_E4  2600   // вместо 330
#define NOTE_F4  2800   // вместо 349
#define NOTE_G4  3100   // вместо 392
#define NOTE_A4  3400   // вместо 440
#define NOTE_B4  3700   // вместо 494
#define NOTE_C5  4000   // вместо 523
#define NOTE_PAUSE  0   // пауза


extern int melody[];

void initBuzzer();

void playTone(int freq, int duration);

void playMelody();



#endif