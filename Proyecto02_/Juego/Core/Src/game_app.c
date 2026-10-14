
#include "main.h"
#include "game_app.h"
#include "ili9341.h"
#include "splash_images.h"
#include "level_images.h"
#include "level1_scene.h"
#include "level2_scene.h"
#include "level3_scene.h"
#include "gameplay.h"
#include "victory_screen.h"
#include "pause_screen.h"
#include "board_link.h"
#include "storage.h"
#include "status_text.h"
#include <stdio.h>

#define SHOW_LINK_DIAGNOSTICS 0

#define FRAME_INTERVAL_MS 500U
#define DEBOUNCE_MS 30U
#define F_LEFT  (1U << 0)
#define F_RIGHT (1U << 1)
#define F_JUMP  (1U << 2)
#define F_PAUSE (1U << 3)
#define A_LEFT  (1U << 4)
#define A_RIGHT (1U << 5)
#define A_JUMP  (1U << 6)
#define A_PAUSE (1U << 7)
typedef enum {
    APP_INTRO,
    APP_LEVEL_MENU,
    APP_LEVEL1,
    APP_LEVEL2,
    APP_LEVEL3,
    APP_VICTORY,
    APP_PAUSED
} AppState;

static AppState state;
static uint8_t selected_level;
static uint8_t current_frame;
static uint32_t frame_changed_at;
static uint8_t candidate;
static uint8_t armed;
static uint8_t menu_stable;
static uint32_t candidate_since[8];
static uint32_t neutral_since;
static uint8_t victory_choice = 1U;

#if 0
static uint8_t ReadButtons(void)
{
    uint8_t mask = 0U;
#define READ_BUTTON(label, bit) do { \
    if (HAL_GPIO_ReadPin(label##_GPIO_Port, label##_Pin) == GPIO_PIN_RESET) \
        mask |= (bit); \
} while (0)
    READ_BUTTON(BTN_F_LEFT, F_LEFT);
    READ_BUTTON(BTN_F_RIGHT, F_RIGHT);
    READ_BUTTON(BTN_F_JUMP, F_PAUSE);
    READ_BUTTON(BTN_F_PAUSE, F_JUMP);
    READ_BUTTON(BTN_A_LEFT, A_LEFT);
    READ_BUTTON(BTN_A_RIGHT, A_RIGHT);
    READ_BUTTON(BTN_A_JUMP, A_PAUSE);
    READ_BUTTON(BTN_A_PAUSE, A_JUMP);
#undef READ_BUTTON
    return mask;
}

#endif
static uint8_t ReadButtons(void){ return BoardLink_Buttons(); }

static void ResetButtons(void)
{
    candidate = ReadButtons();
    for(unsigned i=0;i<8;i++) candidate_since[i] = HAL_GetTick();
    neutral_since=HAL_GetTick();
    armed = 0U;
    menu_stable = candidate;
}


static uint8_t ButtonEvent(uint32_t now)
{
    uint8_t raw=ReadButtons(),rising=0;
    if(raw & (F_JUMP|A_JUMP|F_PAUSE|A_PAUSE)) neutral_since=now;
    else if((uint32_t)(now-neutral_since)>=DEBOUNCE_MS) armed=1;
    for(unsigned i=0;i<8;i++){
        uint8_t bit=(uint8_t)(1u<<i);
        if((raw & bit)!=(candidate & bit)){
            candidate^=bit;candidate_since[i]=now;
        }
        if((uint32_t)(now-candidate_since[i])>=DEBOUNCE_MS &&
           (candidate & bit)!=(menu_stable & bit)){
            menu_stable^=bit;
            if(menu_stable & bit)rising|=bit;
        }
    }
    return armed?rising:0;
}

static void DrawIntro(void)
{
    /* Inicio desde SD */
    if (!Storage_Draw(current_frame ? "INTRO1  BIN" : "INTRO0  BIN", 0)) {
        ILI9341_FillScreen(0);
        StatusText(24,72,"NO SD");

    }
    frame_changed_at = HAL_GetTick();
}

static void DrawLevelMenu(void)
{
    const LevelImage *img = &level_images[selected_level - 1U];
    if (!ILI9341_DrawRGB565RLE(img->runs, img->pair_count)) Error_Handler();
}

void GameApp_Init(void)
{
    ILI9341_Init();
    state = APP_INTRO;
    selected_level = 1U;
    current_frame = 1U;
    DrawIntro();
    ResetButtons();
}

uint8_t GameApp_GetSelectedLevel(void)
{
    return selected_level;
}

static void StartPlayableLevel(uint8_t level)
{
    if (BoardLink_Connected()!=3U) { StatusText(8,226,"VERIFICAR CONEXION MANDOS :c"); return; }
    BoardLink_AudioState(0);
    if (level == 1U) {
        if (!Storage_Draw("LEVEL1  BIN",1) && !Level1_Draw()) Error_Handler();
        state = APP_LEVEL1;
    } else if (level == 2U) {
        if (!Storage_Draw("LEVEL2  BIN",1) && !Level2_Draw()) Error_Handler();
        state = APP_LEVEL2;
    } else if (level == 3U) {
        if (!Storage_Draw("LEVEL3  BIN",1) && !Level3_Draw()) Error_Handler();
        state = APP_LEVEL3;
    } else {
        return;
    }
    if (!Gameplay_Start(level)) Error_Handler();
    selected_level = level;
    ResetButtons();
}


static AppState paused_from = APP_INTRO;


static uint8_t PausePressed(uint8_t raw, uint32_t now)
{
    static uint8_t candidate_pause = 0U;
    static uint8_t stable_pause = 0U;
    static uint32_t changed[2] = {0U, 0U};
    const uint8_t bits[2] = {F_PAUSE, A_PAUSE};
    uint8_t pressed = 0U;
    for (uint8_t i = 0U; i < 2U; i++) {
        uint8_t bit = bits[i];
        if ((raw & bit) != (candidate_pause & bit)) {
            candidate_pause ^= bit;
            changed[i] = now;
        }
        if ((uint32_t)(now - changed[i]) >= 25U &&
            (candidate_pause & bit) != (stable_pause & bit)) {
            stable_pause ^= bit;
            if (stable_pause & bit) pressed = 1U;
        }
    }
    return pressed;
}

static void RestorePausedScreen(void)
{
    if ((paused_from==APP_LEVEL1 || paused_from==APP_LEVEL2 || paused_from==APP_LEVEL3) && BoardLink_Connected()!=3U) return;
    state = paused_from;
    if (state == APP_INTRO) {
        DrawIntro();
    } else if (state == APP_LEVEL_MENU) {
        DrawLevelMenu();
    } else if (state == APP_VICTORY) {
        Victory_Draw(victory_choice);
    } else if (state == APP_LEVEL1 || state == APP_LEVEL2 || state == APP_LEVEL3) {
        if (!Gameplay_Resume()) Error_Handler();
    }
    ResetButtons();
}

static uint8_t GameApp_PollCore(void)
{
    static uint8_t pause_choice = PAUSE_CHOICE_CONTINUE;
    uint32_t now = HAL_GetTick();
    uint8_t raw = ReadButtons();

    if ((state==APP_LEVEL1 || state==APP_LEVEL2 || state==APP_LEVEL3) && BoardLink_Connected()!=3U) {
        paused_from=state;state=APP_PAUSED;pause_choice=PAUSE_CHOICE_CONTINUE;
        BoardLink_AudioState(0);Pause_DrawChoice(pause_choice);ResetButtons();return 0;
    }
    if (PausePressed(raw, now)) {
        BoardLink_AudioState(0);
        if (state == APP_INTRO) {
            state = APP_LEVEL_MENU;
            selected_level = 1U;
            DrawLevelMenu();
            ResetButtons();
        } else if (state == APP_LEVEL_MENU) {
            state = APP_INTRO;
            current_frame = 1U;
            DrawIntro();
            ResetButtons();
        } else if (state == APP_PAUSED) {
            RestorePausedScreen();
        } else {
            paused_from = state;
            state = APP_PAUSED;
            pause_choice = PAUSE_CHOICE_CONTINUE;
            Pause_DrawChoice(pause_choice);
            ResetButtons();
        }
        return 0U;
    }
    uint8_t event = ButtonEvent(now) & (uint8_t)~(F_PAUSE | A_PAUSE);
    if (state == APP_PAUSED) {
        if (event & (F_JUMP | A_JUMP)) {
            if (pause_choice == PAUSE_CHOICE_CONTINUE) {
                RestorePausedScreen();
            } else {
                state = APP_LEVEL_MENU;
                DrawLevelMenu();
                ResetButtons();
            }
            return 0U;
        }
        uint8_t left = (event & (F_LEFT | A_LEFT)) != 0U;
        uint8_t right = (event & (F_RIGHT | A_RIGHT)) != 0U;
        if (left != right) {
            uint8_t choice = right ? PAUSE_CHOICE_CONTINUE : PAUSE_CHOICE_MENU;
            if (choice != pause_choice) {
                pause_choice = choice;
                Pause_DrawChoice(pause_choice);
            }
        }
        return 0U;

    if (state == APP_INTRO) {
        if (event != 0U) {
            state = APP_LEVEL_MENU;
            selected_level = 1U;
            DrawLevelMenu();
            ResetButtons();
        } else if ((uint32_t)(now - frame_changed_at) >= FRAME_INTERVAL_MS) {
            current_frame ^= 1U;
            DrawIntro();
        }
        return 0U;
    }

if (state == APP_LEVEL1 || state == APP_LEVEL2 || state == APP_LEVEL3)
{
    uint8_t raw = ReadButtons();
    uint8_t keys = 0U;


    if (raw & F_LEFT) keys |= PLAY_RED_LEFT;
    if (raw & F_RIGHT) keys |= PLAY_RED_RIGHT;
    if (raw & F_JUMP)  keys |= PLAY_RED_JUMP;

    if (raw & A_LEFT) keys |= PLAY_BLUE_LEFT;
    if (raw & A_RIGHT) keys |= PLAY_BLUE_RIGHT;
    if (raw & A_JUMP)  keys |= PLAY_BLUE_JUMP;



    (void)Gameplay_Update(keys);
    if (Gameplay_IsWon())
    {
        BoardLink_AudioState(0);
        BoardLink_Effect(3);
        state = APP_VICTORY;
        victory_choice = selected_level < 3U ? 1U : 0U;
        Victory_Draw(victory_choice);
        ResetButtons();
    }
    return 0U;
}


    if (state == APP_VICTORY)
    {

        if (event & (F_JUMP | A_JUMP)) {
            if (victory_choice == 1U && selected_level < 3U) {
                StartPlayableLevel((uint8_t)(selected_level + 1U));
                return 1U;
            }
            state = APP_LEVEL_MENU;
            DrawLevelMenu();
            ResetButtons();
            return 0U;
        }
        uint8_t left = (event & (F_LEFT | A_LEFT)) != 0U;
        uint8_t right = (event & (F_RIGHT | A_RIGHT)) != 0U;
        if (left != right) {
            uint8_t choice = right ? 1U : 0U;
            if (choice != victory_choice) {
                victory_choice = choice;
                Victory_Draw(victory_choice);
            }
        }
        return 0U;
    }


if (event & (F_JUMP | A_JUMP))
{
    if (selected_level >= 1U && selected_level <= 3U)
        StartPlayableLevel(selected_level);
    return 1U;
}



    uint8_t left = (event & (F_LEFT | A_LEFT)) != 0U;
    uint8_t right = (event & (F_RIGHT | A_RIGHT)) != 0U;
    if (left != right) {
        if (right) selected_level = selected_level == 3U ? 1U : selected_level + 1U;
        else selected_level = selected_level == 1U ? 3U : selected_level - 1U;
        DrawLevelMenu();
    }
    return 0U;
}




static void DrawLinkDiagnostics(void)
{
#if SHOW_LINK_DIAGNOSTICS
    static uint32_t last;
    static uint8_t shown;
    uint32_t now=HAL_GetTick();
    if(state==APP_LEVEL1 || state==APP_LEVEL2 || state==APP_LEVEL3){shown=0;return;}
    if(shown && (uint32_t)(now-last)<250u)return;
    shown=1;last=now;
    char line[41];
    snprintf(line,sizeof line,"RX3 C:%u B:%02X X:F04/A40 O:F08/A80",
             (unsigned)BoardLink_Connected(),(unsigned)ReadButtons());
    StatusText(0,158,line);
    snprintf(line,sizeof line,"F R:%06lu P:%06lu T:%02X",
             (unsigned long)(g_rx_bytes[0]%1000000u),
             (unsigned long)(g_fuego_frames%1000000u),(unsigned)g_rx_type[0]);
    StatusText(0,172,line);
    snprintf(line,sizeof line,"A R:%06lu P:%06lu T:%02X",
             (unsigned long)(g_rx_bytes[1]%1000000u),
             (unsigned long)(g_agua_frames%1000000u),(unsigned)g_rx_type[1]);
    StatusText(0,186,line);
    snprintf(line,sizeof line,"E:%06lu K:%06lu  SUELTA Y PULSA X",
             (unsigned long)(g_link_errors%1000000u),
             (unsigned long)((g_rx_bad_crc[0]+g_rx_bad_crc[1])%1000000u));
    StatusText(0,200,line);
#endif
}


uint8_t GameApp_Poll(void)
{
    uint8_t result=GameApp_PollCore();
    DrawLinkDiagnostics();
    uint8_t playing=state==APP_LEVEL1 || state==APP_LEVEL2 || state==APP_LEVEL3;
    uint8_t menu_music=state==APP_INTRO || state==APP_LEVEL_MENU;
    BoardLink_AudioState(playing?selected_level:(menu_music?1U:0U));
    return playing ? result : 0U;
}
