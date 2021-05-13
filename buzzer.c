/**
 * @file buzzer.c
 * @author Diego Ortín Fernández
 * @date 12-5-2021
 * @brief File containing the definitions for the public buzzer functions, as well as some private functions and structs
 * like the underlying implementation of the buzzer_t type and the base frequencies for notes.
 */

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "buzzer/buzzer.h"

#define BUZZER_TAG "BUZZER" ///< Tag for usage with the ESP_LOG functions

#define BUZZER_SPEED_MODE LEDC_LOW_SPEED_MODE ///< Speed mode to be used with the buzzer
#define BUZZER_DUTY_RESOLUTION LEDC_TIMER_15_BIT    ///< Duty resolution to be used with the buzzer's timer. Can't be
                                                    ///< set too high, because then we won't be able to play high
                                                    ///< frequencies
#define BUZZER_DUTY_RES_BITS 15u ///< Bits of resolution set in the buzzer's timer. Same as above, but in number form
#define BUZZER_CLK_CONFIG LEDC_AUTO_CLK ///< Clock to use with the buzzer's timer. We let it be set automatically.
#define BUZZER_MAX_VOL 100u ///< Upper boundary of the volume value

#define BUZZER_1_MIN_MS 60000u ///< Amount of milliseconds in 1 minute

/**
 * Array containing the base frequencies for each musical note. Final frequencies can then be calculated from these
 * by dividing depending on the octave.
 */
const uint16_t note_base_freq[12] = {
        4186, ///< C
        4435, ///< C#
        4699, ///< D
        4978, ///< D#
        5274, ///< E
        5588, ///< F
        5920, ///< F#
        6272, ///< G
        6645, ///< G#
        7040, ///< A
        7459, ///< A#
        7902  ///< B
};

/**
 * Struct storing the information required to work with a buzzer
 */
struct _buzzer_t {
    ledc_channel_t channel; ///< LEDC channel to use with this buzzer (should be free)
    ledc_timer_t timer; ///< LEDC timer to use with this buzzer (should be free)
    bool playing; ///< Indicates whether the buzzer is currently playing. Updated when the buzzer is paused or resumed.
    int32_t freq_hz; ///< Indicates the current frequency the buzzer is set to. Updated when the frequency is changed.
};


// Private function declarations
double buzzer_get_note_freq(buzzer_note_t note, uint8_t octave);

// Public functions

buzzer_t *buzzer_init(ledc_channel_t channel, ledc_timer_t timer, gpio_num_t gpio_num) {
    buzzer_t *buzzer = malloc(sizeof(buzzer_t)); // Allocate space for the structure

    // Initialize the fields in the structure
    buzzer->channel = channel;
    buzzer->timer = timer;
    buzzer->playing = false;
    buzzer->freq_hz = BUZZER_INTIIAL_FREQ;

    // Timer configuration
    ledc_timer_config_t led_conf = {
            .speed_mode = BUZZER_SPEED_MODE,
            .timer_num = timer,
            .freq_hz = buzzer->freq_hz,
            .duty_resolution = BUZZER_DUTY_RESOLUTION,
            .clk_cfg = BUZZER_CLK_CONFIG
    };
    ledc_timer_config(&led_conf);

    // Channel configuration
    ledc_channel_config_t channel_conf = {
            .speed_mode = BUZZER_SPEED_MODE,
            .intr_type = LEDC_INTR_DISABLE,
            .channel = channel,
            .gpio_num = gpio_num,
            .duty = (1u << 14u),
            .timer_sel = timer,
            .hpoint = 0
    };
    ledc_channel_config(&channel_conf);

    // Pause the timer so the sound doesn't play
    ledc_timer_pause(BUZZER_SPEED_MODE, timer);
    return buzzer;
}

esp_err_t buzzer_play_test(buzzer_t *buzzer, uint16_t bpm) {
    if (!buzzer || bpm == 0) return ESP_FAIL;

    // Definition of the test melody
    buzzer_melody_t melody;
    buzzer_musical_note_t melody_notes[] = {
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_D,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_E,  4, BUZZER_NTYPE_MINIM},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_D,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_G,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F,  4, BUZZER_NTYPE_MINIM},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C,  4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_C,  5, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_A,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_E,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_D,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_As, 4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_As, 4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_A,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_G,  4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F,  4, BUZZER_NTYPE_MINIM}
    };
    melody.melody = melody_notes;
    melody.length = sizeof(melody_notes) / sizeof(buzzer_musical_note_t);

    return buzzer_play_melody(buzzer, &melody, bpm);
}

void buzzer_destroy(buzzer_t *buzzer) {
    free(buzzer);
}

const char *buzzer_get_tag() {
    return BUZZER_TAG;
}

esp_err_t buzzer_play(buzzer_t *buzzer) {
    if (!buzzer) return ESP_FAIL;
    if (buzzer->playing == true) return ESP_OK; // If we're already playing, no need to do anything
    esp_err_t ret = ledc_timer_resume(BUZZER_SPEED_MODE, buzzer->timer); // Resume playing
    if (ret == ESP_FAIL) return ret;
    buzzer->playing = true; // Update the structure
    return ESP_OK;
}

bool buzzer_is_playing(buzzer_t *buzzer) {
    if (!buzzer) return ESP_FAIL;
    return buzzer->playing;
}

esp_err_t buzzer_pause(buzzer_t *buzzer) {
    if (!buzzer) return ESP_FAIL;
    if (buzzer->playing == false) return ESP_OK; // If the buzzer is already paused, no need to do anything
    esp_err_t ret = ledc_timer_pause(BUZZER_SPEED_MODE, buzzer->timer); // Pause the buzzer
    if (ret == ESP_FAIL) return ret;
    buzzer->playing = false; // Update the structure
    return ESP_OK;
}

esp_err_t buzzer_play_ms(buzzer_t *buzzer, uint32_t time_ms) {
    if (!buzzer) return ESP_FAIL;
    esp_err_t ret;
    ret = buzzer_play(buzzer); // Starts playing
    if (ret == ESP_FAIL) return ret;

    // Wait for the set time before pausing, letting FreeRTOS do other things meanwhile
    // NOTE: using vTaskDelay means this won't be very precise, and can wait a minimum of one FreeRTOS tick. By default,
    // the tick rate is set at 100Hz, which means the minimum duration that can be played is 10ms.
    vTaskDelay(pdMS_TO_TICKS(time_ms));

    // Alternative implementation using busy waiting, which is more precise but blocks the processor
    //ets_delay_us(time_ms * 1000);
    ret = buzzer_pause(buzzer); // Stops playing again
    return ret;
}

esp_err_t buzzer_rest_ms(buzzer_t *buzzer, uint32_t time_ms) {
    if (!buzzer) return ESP_FAIL;
    esp_err_t ret;

    bool was_playing = buzzer->playing; // Stores the previous state of the buzzer

    ret = buzzer_pause(buzzer); // Pauses the buzzer
    if (ret == ESP_FAIL) return ret;

    // Wait for the set time after pausing, letting FreeRTOS do other things meanwhile
    // NOTE: using vTaskDelay means this won't be very precise, and can wait a minimum of one FreeRTOS tick. By default,
    // the tick rate is set at 100Hz, which means the minimum duration that can be waited is 10ms.
    vTaskDelay(pdMS_TO_TICKS(time_ms));

    if (was_playing == true) return buzzer_play(buzzer); // If the buzzer was playing, start playing again
    return ESP_OK;
}

esp_err_t buzzer_set_freq(buzzer_t *buzzer, uint32_t freq_hz) {
    if (!buzzer || freq_hz == 0) return ESP_FAIL;

    // When playing the same frequency two consecutive times, this commented statement causes no distinct sounds to be
    // played. When it's commented, the change of frequency causes a small interruption (even if the frequency is the
    // same)
    //if (freq_hz == buzzer->freq_hz) return ESP_OK;

    esp_err_t ret = ledc_set_freq(BUZZER_SPEED_MODE, buzzer->timer, freq_hz);
    if (ret == ESP_FAIL) return ret;
    buzzer->freq_hz = freq_hz; // Update the structure
    return ESP_OK;
}

uint32_t buzzer_get_freq(buzzer_t *buzzer) {
    if (!buzzer) return 0;
    return buzzer->freq_hz;
}

esp_err_t buzzer_set_note(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave) {
    if (!buzzer) return ESP_FAIL;
    return buzzer_set_freq(buzzer, buzzer_get_note_freq(note, octave));
}

esp_err_t buzzer_play_note(buzzer_t *buzzer, buzzer_musical_note_t *note, uint32_t bpm) {
    if (!buzzer || !note || bpm == 0) return ESP_FAIL;
    esp_err_t ret;

    // When the note is not a rest, we just play the note. When it's a rest, we don't need to change the frequency, as
    // the rest doesn't have an associated frequency.
    if (note->note != BUZZER_NOTE_REST) {
        ret = buzzer_set_note(buzzer, note->note, note->octave);
        if (ret == ESP_FAIL) return ret;
    }
    uint32_t time_ms = buzzer_note_type_to_ms(note->type, bpm);

    // When the note is a rest, rest during the set time. When it's a normal note, play it for the set time.
    if (note->note == BUZZER_NOTE_REST) {
        ret = buzzer_rest_ms(buzzer, time_ms);
    } else {
        ret = buzzer_play_ms(buzzer, time_ms);
    }
    return ret;
}


esp_err_t buzzer_play_note_ms(buzzer_t *buzzer, buzzer_note_t note, uint8_t octave, uint32_t time_ms) {
    if (!buzzer) return ESP_FAIL;
    esp_err_t ret;
    ret = buzzer_set_note(buzzer, note, octave); // Set the frequency to the note
    if (ret == ESP_FAIL) return ret;
    ret = buzzer_play_ms(buzzer, time_ms); // Play the buzzer during the provided time
    return ret;
}

esp_err_t buzzer_play_melody(buzzer_t *buzzer, buzzer_melody_t *melody, uint32_t bpm) {
    if (!buzzer || !melody || bpm == 0) return ESP_FAIL;

    // Sequentially play all the notes in the melody
    for (uint i = 0; i < melody->length; i++) {
        esp_err_t ret = buzzer_play_note(buzzer, &melody->melody[i], bpm);
        if (ret == ESP_FAIL) return ret;
    }
    return ESP_OK;
}

//esp_err_t buzzer_set_volume(buzzer_t *buzzer, uint8_t volume) {
//    if (!buzzer) return ESP_FAIL;
//    if (volume > BUZZER_MAX_VOL) volume = BUZZER_MAX_VOL;
//
//    uint32_t duty = ((1u << BUZZER_DUTY_RES_BITS) * volume) / BUZZER_MAX_VOL;
//    esp_err_t ret = ledc_set_duty(BUZZER_SPEED_MODE, buzzer->channel, duty);
//    if (ret == ESP_FAIL) return ret;
//    return ledc_update_duty(BUZZER_SPEED_MODE, buzzer->channel);
//    //return ledc_set_duty_and_update(BUZZER_SPEED_MODE, buzzer->channel, duty, 0);
//}

uint32_t buzzer_note_type_to_ms(buzzer_note_type_t type, uint32_t bpm) {
    uint32_t ms_per_beat = BUZZER_1_MIN_MS / bpm;
    return (ms_per_beat * type) / BUZZER_BASE_PULSE_DIVISIONS;
}

double buzzer_get_note_freq(buzzer_note_t note, uint8_t octave) {
    if (octave > 8) octave = 8;
    double divider = (double) (1u << (8u - octave)); // Divide the base frequency by 2^(8-octave)

    // Warning: if a frequency of 0 is played, an error might occur. This return statement is only here to prevent
    // out of bounds errors when indexing the array.
    if (note == BUZZER_NOTE_REST || note == BUZZER_NOTE_MAX) return 0;
    return (double) note_base_freq[note] / divider;
}