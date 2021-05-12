//
// Created by diego on 12/5/21.
//

#ifndef GYRO_READER_BUZZER_H
#define GYRO_READER_BUZZER_H

#include <esp_err.h>
#include <driver/ledc.h>

#define BUZZER_BASE_PULSE_DIVISIONS 8 //< How many parts a pulse is divided by before multiplying by the note type

#define BUZZER_INTIIAL_FREQ 440
typedef enum _buzzer_note_t {
    BUZZER_NOTE_C,   // C
    BUZZER_NOTE_Cs,  // C#
    BUZZER_NOTE_D,   // D
    BUZZER_NOTE_Ds,  // D#
    BUZZER_NOTE_E,   // E
    BUZZER_NOTE_F,   // F
    BUZZER_NOTE_Fs,  // F#
    BUZZER_NOTE_G,   // G
    BUZZER_NOTE_Gs,  // G#
    BUZZER_NOTE_A,   // A
    BUZZER_NOTE_As,  // A#
    BUZZER_NOTE_B,   // B
    BUZZER_NOTE_MAX,
    BUZZER_NOTE_REST // 𝄽
} buzzer_note_t;

typedef enum _buzzer_note_type_t {
    BUZZER_NTYPE_SEMIBREVE_DOTTED   = 48,   // 𝅝𝅭 (6 pulses sound) (48/8)
    BUZZER_NTYPE_SEMIBREVE          = 32,   // 𝅝 (4 pulses sound) (32/8)
    BUZZER_NTYPE_MINIM_DOTTED       = 24,   // 𝅗𝅥𝅭 (3 pulses sound) (24/8)
    BUZZER_NTYPE_MINIM              = 16,   // 𝅗𝅥 (2 pulses sound) (16/8)
    BUZZER_NTYPE_CROTCHET_DOTTED    = 12,   // 𝅘𝅥𝅭 (3/2 pulses sound) (12/8)
    BUZZER_NTYPE_CROTCHET           = 8,    // 𝅘𝅥 (1 pulse sound) (8/8)
    BUZZER_NTYPE_QUAVER_DOTTED      = 6,    // 𝅘𝅥𝅮𝅭 (3/4 pulses sound) (6/8)
    BUZZER_NTYPE_QUAVER             = 4,    // 𝅘𝅥𝅮 (1/2 pulses sound) (4/8)
    BUZZER_NTYPE_SEMIQUAVER_DOTTED  = 3,    // 𝅘𝅥𝅯𝅭 (3/8 pulses sound) (3/8)
    BUZZER_NTYPE_SEMIQUAVER         = 2,    // 𝅘𝅥𝅯 (1/4 pulses sound) (2/8)
//    BUZZER_NTYPE_SEMIBREVE_REST_DOTTED,     // 𝄻𝅭 (6 pulses silence) (48/8)
//    BUZZER_NTYPE_SEMIBREVE_REST,            // 𝄻 (4 pulses silence) (32/8)
//    BUZZER_NTYPE_MINIM_REST_DOTTED,         // 𝄼𝅭 (3 pulses silence) (24/8)
//    BUZZER_NTYPE_MINIM_REST,                // 𝄼 (2 pulses silence) (16/8)
//    BUZZER_NTYPE_CROTCHET_REST_DOTTED,      // 𝄽𝅭 (3/2 pulses silence) (12/8)
//    BUZZER_NTYPE_CROTCHET_REST,             // 𝄽 (1 pulse silence) (8/8)
//    BUZZER_NTYPE_QUAVER_REST_DOTTED,        // 𝄾𝅭 (3/4 pulses silence) (6/8)
//    BUZZER_NTYPE_QUAVER_REST,               // 𝄾 (1/2 pulses silence) (4/8)
//    BUZZER_NTYPE_SEMIQUAVER_REST_DOTTED,    // 𝄿𝅭 (3/8 pulses silence) (3/8)
//    BUZZER_NTYPE_SEMIQUAVER_REST,           // 𝄿 (1/4 pulses silence) (2/8)
} buzzer_note_type_t;

typedef struct _buzzer_t buzzer_t;

typedef struct  _buzzer_melody_note_t {
    buzzer_note_t note;
    uint8_t octave;
    buzzer_note_type_t type;
} buzzer_musical_note_t;

typedef struct _buzzer_melody_t {
    buzzer_musical_note_t *melody;
    uint32_t length;
} buzzer_melody_t;

/**
 * Creates and initializes the buzzer, using the provided LEDC channel and timer, on the specified GPIO pin.
 * The LEDC channel and timer don't need to be configured in advance, as this function will automatically set them up.
 * @param channel LEDC channel to use
 * @param timer LEDC timer to use
 * @param gpio_num GPIO pìn to use
 * @return Pointer to an initialized buzzer
 */
buzzer_t *buzzer_init(ledc_channel_t channel, ledc_timer_t timer, gpio_num_t gpio_num);

/**
 * Plays a melody on the buzzer to test if it's working correctly.
 * @param buzzer
 * @param bpm
 * @return
 */
esp_err_t buzzer_play_test(buzzer_t *buzzer, uint16_t bpm);

void buzzer_destroy(buzzer_t *buzzer);

const char* buzzer_get_tag();

esp_err_t buzzer_play(buzzer_t *buzzer);

esp_err_t buzzer_pause(buzzer_t *buzzer);

esp_err_t buzzer_play_ms(buzzer_t *buzzer, uint32_t time_ms);

esp_err_t buzzer_rest_ms(buzzer_t *buzzer, uint32_t time_ms);

esp_err_t buzzer_play_melody(buzzer_t *buzzer, buzzer_melody_t *melody, uint32_t bpm);

esp_err_t buzzer_set_freq(buzzer_t *buzzer, uint32_t freq_hz);

uint32_t buzzer_get_freq(buzzer_t *buzzer);

esp_err_t buzzer_set_note(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave);

esp_err_t buzzer_set_volume(buzzer_t *buzzer, uint8_t volume);

esp_err_t buzzer_play_note_ms(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave, uint32_t time_ms);

uint32_t buzzer_note_type_to_ms(buzzer_note_type_t type, uint32_t bpm);

#endif //GYRO_READER_BUZZER_H
