/* date = May 5th 2021 4:12 pm */

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

//- NOTE(Momo): Global Settings
#define GAME_DESIGN_WIDTH 800
#define GAME_DESIGN_HEIGHT 800
#define GAME_DESIGN_DPI 96
#define GAME_DESIGN_DEPTH 500 // some big number is okay

#define GAME_MEMORY_SIZE Mebibytes(24)
#define GAME_RENDER_COMMAND_SIZE Mebibytes(32)

#define GAME_AUDIO_SAMPLES_PER_SECOND 48000
#define GAME_AUDIO_CHANNELS 2
#define GAME_AUDIO_BITS_PER_SAMPLE 16
#define GAME_AUDIO_LATENCY_FRAMES 1

#define GAME_ASSET_FILENAME "yuu"
#define GAME_ASSET_FILE_SIGNATURE "MOMO"
#define GAME_SAVE_FILENAME "save_data"
#define GAME_TITLE "Dots and Circles v1.0"

//- NOTE(Momo): Game related
#define GAME_BULLET_CAP 1024
#define GAME_PARTICLE_CAP 2048
#define GAME_ENEMY_CAP 128
#define GAME_SCORE_MAX_PLACES 128

#define GAME_BGM_BPM 120.f
#define GAME_SECS_PER_BEAT BeatsPerMinToSecsPerBeat(GAME_BGM_BPM)

#define GAME_SLOWMO_BULLET_LIMIT 100
#define GAME_SLOWMO_COOLDOWN 30.f
#define GAME_SLOWMO_RESUME_TRANSITION_DURATION 4.f
#define GAME_SLOWMO_EFFECT_DURATION 0.5f
#define GAME_SWEEPER_COMBO_REQUIREMENTS 50

#define GAME_SWEEPER_SPEED 450.f
#define GAME_DEATH_BOMB_SPEED 1000.f

#endif //GAME_CONFIG_H