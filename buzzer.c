//
// Created by diego on 12/5/21.
//

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "buzzer/buzzer.h"

#define BUZZER_TAG "BUZZER"

#define BUZZER_SPEED_MODE LEDC_LOW_SPEED_MODE
#define BUZZER_DUTY_RESOLUTION LEDC_TIMER_15_BIT
#define BUZZER_DUTY_RES_BITS 15u
#define BUZZER_CLK_CONFIG LEDC_AUTO_CLK
#define BUZZER_MAX_VOL 100u

#define BUZZER_1_MIN_MS 60000u

const uint16_t note_base_freq[12] = {
        4186, // C
        4435, // C#
        4699, // D
        4978, // D#
        5274, // E
        5588, // F
        5920, // F#
        6272, // G
        6645, // G#
        7040, // A
        7459, // A#
        7902  // B
};

struct _buzzer_t {
    ledc_channel_t channel;
    ledc_timer_t timer;
    gpio_num_t gpio_num;
    bool playing;
    int32_t freq_hz;
};



// Private function declarations
double buzzer_get_note_freq(buzzer_note_t note, uint8_t octave);


buzzer_t *buzzer_init(ledc_channel_t channel, ledc_timer_t timer, gpio_num_t gpio_num) {
    buzzer_t *buzzer = malloc(sizeof(buzzer_t));
    buzzer->channel = channel;
    buzzer->timer = timer;
    buzzer->gpio_num = gpio_num;
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

    buzzer_melody_t melody;
    buzzer_musical_note_t melody_notes[] = {
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_D, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_E, 4, BUZZER_NTYPE_MINIM},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_D, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_G, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F, 4, BUZZER_NTYPE_MINIM},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_C, 4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_C, 5, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_A, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_E, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_D, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_As, 4, BUZZER_NTYPE_QUAVER_DOTTED},
            {BUZZER_NOTE_As, 4, BUZZER_NTYPE_SEMIQUAVER},
            {BUZZER_NOTE_A, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_G, 4, BUZZER_NTYPE_CROTCHET},
            {BUZZER_NOTE_F, 4, BUZZER_NTYPE_MINIM}
    };
    melody.melody = melody_notes;
    melody.length = sizeof(melody_notes) / sizeof(buzzer_musical_note_t);
    return buzzer_play_melody(buzzer, &melody, bpm);
}

void buzzer_destroy(buzzer_t *buzzer) {
    free(buzzer);
}

const char* buzzer_get_tag() {
    return BUZZER_TAG;
}

esp_err_t buzzer_play(buzzer_t *buzzer) {
    if (!buzzer) return ESP_FAIL;
    if (buzzer->playing == true) return ESP_OK;
    esp_err_t ret = ledc_timer_resume(BUZZER_SPEED_MODE, buzzer->timer);
    if (ret == ESP_FAIL) return ret;
    buzzer->playing = true;
    return ESP_OK;
}

esp_err_t buzzer_pause(buzzer_t *buzzer) {
    if (!buzzer) return ESP_FAIL;
    if (buzzer->playing == false) return ESP_OK;
    esp_err_t ret = ledc_timer_pause(BUZZER_SPEED_MODE, buzzer->timer);
    if (ret == ESP_FAIL) return ret;
    buzzer->playing = false;
    return ESP_OK;
}

esp_err_t buzzer_play_ms(buzzer_t *buzzer, uint32_t time_ms) {
    if (!buzzer) return ESP_FAIL;
    esp_err_t ret;
    ret = buzzer_play(buzzer);
    if (ret == ESP_FAIL) return ret;
    vTaskDelay(pdMS_TO_TICKS(time_ms));
    //ets_delay_us(time_ms * 1000);
    ret = buzzer_pause(buzzer);
    return ret;
}

esp_err_t buzzer_rest_ms(buzzer_t *buzzer, uint32_t time_ms) {
    if (!buzzer) return ESP_FAIL;
    esp_err_t ret;
    ret = buzzer_pause(buzzer);
    if (ret == ESP_FAIL) return ret;
    vTaskDelay(pdMS_TO_TICKS(time_ms));
    return ESP_OK;
}

esp_err_t buzzer_set_freq(buzzer_t *buzzer, uint32_t freq_hz) {
    if (!buzzer || freq_hz == 0) return ESP_FAIL;
    //if (freq_hz == buzzer->freq_hz) return ESP_OK;
    esp_err_t ret = ledc_set_freq(BUZZER_SPEED_MODE, buzzer->timer, freq_hz);
    if (ret == ESP_FAIL) return ret;
    buzzer->freq_hz = freq_hz;
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
    if (note->note != BUZZER_NOTE_REST) {
        ret = buzzer_set_note(buzzer, note->note, note->octave);
        if (ret == ESP_FAIL) return ret;
    }
    uint32_t time_ms = buzzer_note_type_to_ms(note->type, bpm);
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
    ret = buzzer_set_note(buzzer, note, octave);
    if (ret == ESP_FAIL) return ret;
    ret = buzzer_play_ms(buzzer, time_ms);
    return ret;
}

esp_err_t buzzer_play_melody(buzzer_t *buzzer, buzzer_melody_t *melody, uint32_t bpm) {
    if (!buzzer || !melody || bpm == 0) return ESP_FAIL;

    for (uint i = 0; i < melody->length; i++) {
        esp_err_t ret = buzzer_play_note(buzzer, &melody->melody[i], bpm);
        if (ret == ESP_FAIL) return ret;
    }
    return ESP_OK;
}

esp_err_t buzzer_set_volume(buzzer_t *buzzer, uint8_t volume) {
    if (!buzzer) return ESP_FAIL;
    if (volume > BUZZER_MAX_VOL) volume = BUZZER_MAX_VOL;

    uint32_t duty = ((1u << BUZZER_DUTY_RES_BITS) * volume) / BUZZER_MAX_VOL;
    esp_err_t ret = ledc_set_duty(BUZZER_SPEED_MODE, buzzer->channel, duty);
    if (ret == ESP_FAIL) return ret;
    return ledc_update_duty(BUZZER_SPEED_MODE, buzzer->channel);
    //return ledc_set_duty_and_update(BUZZER_SPEED_MODE, buzzer->channel, duty, 0);
}

uint32_t buzzer_note_type_to_ms(buzzer_note_type_t type, uint32_t bpm) {
    uint32_t ms_per_beat = BUZZER_1_MIN_MS / bpm;
    return (ms_per_beat * type) / BUZZER_BASE_PULSE_DIVISIONS;
}

double buzzer_get_note_freq(buzzer_note_t note, uint8_t octave) {
    if (octave > 8) octave = 8;
    double divider = (double) (1u << (8u - octave));
    if (note == BUZZER_NOTE_REST || note == BUZZER_NOTE_MAX) return 0;
    return (double) note_base_freq[note] / divider;
}