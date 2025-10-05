#include <Arduino.h>
#include "melody.h"
#include <cstdint>


uint8_t resolution = 8;   // 8 бит
uint32_t baseFreq = 1000; // стартовая частота для инициализации

int melody[] = {
  NOTE_C4, 4, NOTE_D4, 4, NOTE_E4, 4, NOTE_F4, 4,
  NOTE_G4, 2, NOTE_G4, 2, NOTE_A4, 4, NOTE_B4, 4,
  NOTE_C5, 2, NOTE_PAUSE, 4
};

int melodyLength = sizeof(melody) / sizeof(melody[0]);

// Воспроизведение одной ноты
void playTone(int freq, int duration) {
  if (freq == NOTE_PAUSE) {
    ledcWrite(BUZZER_PIN, 0);   // тишина
    delay(duration);
  } else {
    ledcWriteTone(BUZZER_PIN, freq);
    delay(duration);
    ledcWrite(BUZZER_PIN, 0);   // остановить
  }
}

// Воспроизведение всей мелодии
void playMelody() {
  for (int i = 0; i < melodyLength; i += 2) {
    int note = melody[i];
    int duration = 200 * melody[i + 1];  // множитель для длительности
    playTone(note, duration);
    delay(50);  // пауза между нотами
  }
}

void initBuzzer(){
    bool ok = ledcAttach(BUZZER_PIN, baseFreq, resolution);
}