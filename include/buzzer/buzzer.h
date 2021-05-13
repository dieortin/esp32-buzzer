/**
 * @file buzzer.h
 * @author Diego Ortín Fernández
 * @date 12-5-2021
 * @brief File containing the declarations for the public functions of the buzzer module, as well as some public structs
 * like note and note type declarations.
 */

#ifndef GYRO_READER_BUZZER_H
#define GYRO_READER_BUZZER_H

#include <esp_err.h>
#include <driver/ledc.h>

#define BUZZER_BASE_PULSE_DIVISIONS 8 ///< How many parts a pulse is divided by before multiplying by the note type

#define BUZZER_INTIIAL_FREQ 440 ///< Frequency the buzzer will be set to at initialization time

/**
 * Enumeration containing the different musical notes. It also contains the "rest note", which isn't a real musical
 * note but can be used to "play" a silence.
 */
typedef enum _buzzer_note_t {
    BUZZER_NOTE_C,   ///< C
    BUZZER_NOTE_Cs,  ///< C#
    BUZZER_NOTE_D,   ///< D
    BUZZER_NOTE_Ds,  ///< D#
    BUZZER_NOTE_E,   ///< E
    BUZZER_NOTE_F,   ///< F
    BUZZER_NOTE_Fs,  ///< F#
    BUZZER_NOTE_G,   ///< G
    BUZZER_NOTE_Gs,  ///< G#
    BUZZER_NOTE_A,   ///< A
    BUZZER_NOTE_As,  ///< A#
    BUZZER_NOTE_B,   ///< B
    BUZZER_NOTE_MAX,
    BUZZER_NOTE_REST ///< 𝄽 (rest)
} buzzer_note_t;

/**
 * Enumeration containing the different note types, according to their duration.
 * The corresponding number for each note type corresponds with the number of eights of a pulse the type takes.
 */
typedef enum _buzzer_note_type_t {
    BUZZER_NTYPE_SEMIBREVE_DOTTED   = 48,   ///< 𝅝𝅭 or 𝄻𝅭 (6 pulses sound) (48/8)
    BUZZER_NTYPE_SEMIBREVE          = 32,   ///< 𝅝 or 𝄻 (4 pulses sound) (32/8)
    BUZZER_NTYPE_MINIM_DOTTED       = 24,   ///< 𝅗𝅥𝅭 or 𝄼𝅭 (3 pulses sound) (24/8)
    BUZZER_NTYPE_MINIM              = 16,   ///< 𝅗𝅥 or 𝄼 (2 pulses sound) (16/8)
    BUZZER_NTYPE_CROTCHET_DOTTED    = 12,   ///< 𝅘𝅥𝅭 or 𝄽𝅭 (3/2 pulses sound) (12/8)
    BUZZER_NTYPE_CROTCHET           = 8,    ///< 𝅘𝅥 or 𝄽 (1 pulse sound) (8/8)
    BUZZER_NTYPE_QUAVER_DOTTED      = 6,    ///< 𝅘𝅥𝅮𝅭 or 𝄾𝅭 (3/4 pulses sound) (6/8)
    BUZZER_NTYPE_QUAVER             = 4,    ///< 𝅘𝅥𝅮 or 𝄾 (1/2 pulses sound) (4/8)
    BUZZER_NTYPE_SEMIQUAVER_DOTTED  = 3,    ///< 𝅘𝅥𝅯𝅭  or 𝄿𝅭 (3/8 pulses sound) (3/8)
    BUZZER_NTYPE_SEMIQUAVER         = 2,    ///< 𝅘𝅥𝅯 or 𝄿 (1/4 pulses sound) (2/8)
} buzzer_note_type_t;

typedef struct _buzzer_t buzzer_t;

/**
 * Structure with the required attributes to fully define a musical note (pitch and duration)
 */
typedef struct _buzzer_melody_note_t {
    buzzer_note_t note; ///< Musical note
    uint8_t octave; ///< Octave of the musical note (from 0 to 8)
    buzzer_note_type_t type; ///< Duration type of the musical note
} buzzer_musical_note_t;

/**
 * Structure with a sequence of musical notes, and its length for iteration purposes
 */
typedef struct _buzzer_melody_t {
    buzzer_musical_note_t *melody; ///< Pointer to an array of musical notes, to be played in order
    uint32_t length; ///< Length of the array of musical notes
} buzzer_melody_t;

/**
 * Creates and initializes the buzzer, using the provided LEDC channel and timer, on the specified GPIO pin.
 *
 * @details The LEDC channel and timer don't need to be configured in advance, as this function will automatically set them up.
 * @param channel LEDC channel to use
 * @param timer LEDC timer to use
 * @param gpio_num GPIO pìn to use
 * @return Pointer to an initialized buzzer
 */
buzzer_t *buzzer_init(ledc_channel_t channel, ledc_timer_t timer, gpio_num_t gpio_num);

/**
 * Plays a melody on the buzzer to test if it's working correctly.
 *
 * @details Keep in mind this function only returns an error when something goes wrong while outputting the signal to the buzzer,
 * but doesn't know if the buzzer is correctly connected or not.
 * @param buzzer Buzzer object where the sound must be played
 * @param bpm Speed the test must be played at, in beats per minute (1 beat = 1 crochet)
 * @return ESP_OK if the test playing operation was executed successfully, ESP_FAIL otherwise
 */
esp_err_t buzzer_play_test(buzzer_t *buzzer, uint16_t bpm);

/**
 * Frees the associated memory with a buzzer
 * @param buzzer Buzzer to destroy
 */
void buzzer_destroy(buzzer_t *buzzer);

/**
 * Returns the buzzer's associated tag, for use when logging with the ESP_LOG functions.
 * @return String with the buzzer's tag
 */
const char* buzzer_get_tag();

/**
 * Turns on the buzzer, making it play whichever frequency it's currently set to.
 * @param buzzer Buzzer to start
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_play(buzzer_t *buzzer);

/**
 * Pauses the buzzer, making it stop emitting any sound.
 * @param buzzer Buzzer to stop
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_pause(buzzer_t *buzzer);

/**
 * Checks if the buzzer is currently playing
 * @param buzzer Buzzer to check
 * @return true if the buzzer is currently playing, false if it's paused
 */
bool buzzer_is_playing(buzzer_t *buzzer);

/**
 * Turns on the buzzer for the provided amount of time in milliseconds.
 * @param buzzer Buzzer to turn on
 * @param time_ms Time the buzzer must be on in milliseconds
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_play_ms(buzzer_t *buzzer, uint32_t time_ms);

/**
 * Turns off the buzzer for the provided amount of time in milliseconds.
 * @param buzzer Buzzer to turn off
 * @param time_ms
 * @return
 */
esp_err_t buzzer_rest_ms(buzzer_t *buzzer, uint32_t time_ms);

/**
 * Plays the provided melody in the buzzer, at the given speed in beats per minute.
 * @param buzzer Buzzer to play the melody on
 * @param melody Melody to play
 * @param bpm Speed to play the melody at (in beats per minute)
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_play_melody(buzzer_t *buzzer, buzzer_melody_t *melody, uint32_t bpm);

/**
 * Sets the frequency the buzzer plays in Hertzs
 * @param buzzer Buzzer whose frequency must be set
 * @param freq_hz Frequency to set in Hz
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_set_freq(buzzer_t *buzzer, uint32_t freq_hz);

/**
 * Checks and returns the frequency the buzzer is currently set to, in Hertzs
 * @param buzzer Buzzer whose frequency must be checked
 * @return Frequency currently set in the buzzer, in Hz
 */
uint32_t buzzer_get_freq(buzzer_t *buzzer);

/**
 * Sets the frequency the buzzer plays to that of the provided note (in the given octave)
 * @param buzzer Buzzer to set the frequency for
 * @param note Note to set
 * @param octave Octave of the note (from 0 to 8)
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_set_note(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave);

//esp_err_t buzzer_set_volume(buzzer_t *buzzer, uint8_t volume);

/**
 * Plays the provided note for the given amount of time in milliseconds.
 * @param buzzer Buzzer to play the note from
 * @param note Note to play
 * @param octave Octave of the note (from 0 to 8)
 * @param time_ms Time to play the note for, in milliseconds
 * @return ESP_OK if the operation was executed successfully, ESP_FAIL if something went wrong
 */
esp_err_t buzzer_play_note_ms(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave, uint32_t time_ms);

/**
 * Returns the time in milliseconds corresponding to the given note type at the provided speed in beats per minute.
 * @param type Note type to convert
 * @param bpm Playing speed in beats per minute
 * @return Milliseconds equivalent to the note type at that speed
 */
uint32_t buzzer_note_type_to_ms(buzzer_note_type_t type, uint32_t bpm);

#endif //GYRO_READER_BUZZER_H
