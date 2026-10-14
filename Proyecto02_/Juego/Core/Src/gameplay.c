#include "main.h"
#include "gameplay.h"
#include "board_link.h"
#include "ili9341.h"
#include "scene_cache.h"
#include "player_sprites.h"
#include "puddle_sprites.h"
#include <string.h>

#define FP 256L
#define STEP_MS 16U
#define DEBOUNCE_MS 20U
#define SPRITE_W 12
#define SPRITE_H 18
#define SPEED_X 512L
#define JUMP_VY (-1536L)
#define GRAVITY 82L
#define MAX_FALL 2048L

typedef struct { int16_t x, y, w, h; } Solid;
typedef struct {
    int32_t x, y, vy;
    uint8_t grounded, facing_left, frame;
    uint16_t walk_ticks;
} Player;


static const Solid level1_solids[] = {
    {0,33,209,8}, {257,65,63,8}, {145,88,80,8}, {49,113,63,8},
    {0,136,32,8}, {0,168,241,8}, {0,200,81,8}, {272,191,48,33},
    {0,224,320,16}
};
static const Solid level2_solids[] = {
    {33,32,15,8}, {112,32,16,8}, {193,32,16,8}, {273,32,15,8},
    {33,40,95,1}, {193,40,95,1}, {65,41,16,63}, {241,41,15,63},
    {129,62,63,8}, {145,70,31,34}, {0,72,32,8}, {289,72,31,8},
    {33,104,255,8}, {0,145,17,8}, {97,145,32,8}, {192,145,32,8},
    {304,145,16,8}, {0,153,129,1}, {192,153,128,1},
    {128,177,65,8}, {144,185,32,39}, {97,193,31,8}, {193,193,31,8},
    {0,224,32,8}, {64,224,193,8}, {289,224,31,8}, {0,232,320,8}
};
static const Solid level3_solids[] = {
    {0,36,256,8}, {289,62,31,8}, {49,94,32,8}, {113,94,32,8},
    {177,94,32,8}, {241,94,79,8}, {49,102,271,1}, {0,135,273,9},
    {32,183,8,8}, {104,183,8,8}, {177,183,7,8}, {248,183,9,8},
    {289,183,31,9}, {32,191,80,1}, {177,191,80,1},
    {0,224,40,8}, {104,224,80,8}, {248,224,72,8}, {0,232,320,8}
};
static const Solid *solids;
static uint16_t solid_count;
static Player players[2]; /* 0 = F/rojo, 1 = A/azul */
static uint16_t patch[SPRITE_W * SPRITE_H];
static uint32_t step_at, changed_at[7];
static uint8_t candidates, stable, valid, jump_armed, jump_pending;
static uint8_t active;
static uint8_t won;
static uint8_t current_level;

/* safe_player: 0 = rojo, 1 = azul, 2 = toxico para ambos. */
typedef struct { Solid area; uint8_t safe_player; } Puddle;
static const Puddle level2_puddles[] = {
    {{48,32,64,8},0},   {{209,32,64,8},1},
    {{17,145,80,8},0},  {{224,145,80,8},1},
    {{32,224,32,8},0},  {{257,224,32,8},1}
};
static const Puddle level3_puddles[] = {
    {{81,94,32,8},2},   {{145,94,32,8},2}, {{209,94,32,8},2},
    {{40,183,64,8},1},  {{184,183,64,8},0},
    {{40,224,64,8},0},  {{184,224,64,8},1}
};
static const Puddle *puddles;
static uint16_t puddle_count;
#define PUDDLE_FRAME_MS 160U
static uint8_t puddle_frame;
static uint32_t puddle_at;
static Player spawn_players[2];

static uint16_t PuddlePixel(int16_t x, int16_t y)
{
    for (uint16_t i = 0U; i < puddle_count; i++) {
        const Puddle *p = &puddles[i];
        const Solid *r = &p->area;
        if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) {
            const uint16_t *sheet = p->safe_player == 2U ? puddle_green :
                                    (p->safe_player == 1U ? puddle_blue : puddle_red);
            return sheet[(y-r->y)*64U + puddle_frame*16U + (x-r->x)%16U];
        }
    }
    return 0U;
}

static int16_t Pixel(int32_t fp)
{

    return (int16_t)(fp >= 0 ? fp / FP : -((-fp + FP - 1L) / FP));
}

static uint8_t HorizontalOverlap(int32_t x, const Solid *r)
{
    return x + 10L * FP > (int32_t)r->x * FP &&
           x + 2L * FP < (int32_t)(r->x + r->w) * FP;
}
static uint8_t VerticalOverlap(int32_t y, const Solid *r)
{
    return y + 18L * FP > (int32_t)r->y * FP &&
           y + FP < (int32_t)(r->y + r->h) * FP;
}
static uint8_t IsGrounded(const Player *p)
{
    for (uint16_t i = 0; i < solid_count; i++) {
        if (HorizontalOverlap(p->x, &solids[i]) &&
            p->y + 18L * FP == (int32_t)solids[i].y * FP) return 1U;
    }
    return 0U;
}


static uint8_t CheckPuddles(void)
{
    for (uint8_t i = 0U; i < 2U; i++) {
        Player *p = &players[i];
        for (uint16_t j = 0U; j < puddle_count; j++) {
            const Puddle *water = &puddles[j];
            const Solid *r = &water->area;
            if (water->safe_player == i) continue;
            if (HorizontalOverlap(p->x, r) &&
                p->y + 18L*FP >= (int32_t)r->y*FP &&
                p->y + FP < (int32_t)(r->y+r->h)*FP) {
                /* Muerte reinicia a ambos */
                BoardLink_Effect(2);
                memcpy(players, spawn_players, sizeof players);
                jump_pending = jump_armed = 0U;
                return 1U;
            }
        }
    }
    return 0U;
}


static const Solid goals[3][2] = {
    {{10,6,20,27}, {51,6,20,27}},
    {{97,77,20,27}, {204,77,20,27}},
    {{17,9,20,27}, {50,9,20,27}}
};
static uint8_t BothAtOwnDoors(void)
{
    if (current_level < 1U || current_level > 3U) return 0U;
    for (uint8_t i = 0U; i < 2U; i++) {
        const Player *p = &players[i];
        const Solid *door = &goals[current_level - 1U][i];
        if (!p->grounded ||
            p->x + 2L * FP < (int32_t)door->x * FP ||
            p->x + 10L * FP > (int32_t)(door->x + door->w) * FP ||
            p->y + FP < (int32_t)door->y * FP ||
            p->y + 18L * FP > (int32_t)(door->y + door->h) * FP) return 0U;
    }
    return 1U;
}
uint8_t Gameplay_IsWon(void)
{
    return won;
}

static void MovePlayer(Player *p, int8_t direction, uint8_t jump)
{
    p->grounded = IsGrounded(p);
    if (jump && p->grounded) { BoardLink_Effect(1); p->vy = JUMP_VY; p->grounded = 0U; }
    if (direction) p->facing_left = direction < 0;
    int32_t old_x = p->x;
    int32_t new_x = old_x + (int32_t)direction * SPEED_X;
    if (new_x < 0) new_x = 0;
    if (new_x > (320L - SPRITE_W) * FP) new_x = (320L - SPRITE_W) * FP;
    for (uint16_t i = 0; i < solid_count; i++) {
        const Solid *r = &solids[i];
        if (!VerticalOverlap(p->y, r)) continue;
        int32_t left = (int32_t)r->x * FP;
        int32_t right = (int32_t)(r->x + r->w) * FP;
        if (direction > 0 && old_x + 10L * FP <= left && new_x + 10L * FP > left)
            new_x = left - 10L * FP;
        if (direction < 0 && old_x + 2L * FP >= right && new_x + 2L * FP < right)
            new_x = right - 2L * FP;
    }
    p->x = new_x;
    p->vy += GRAVITY;
    if (p->vy > MAX_FALL) p->vy = MAX_FALL;
    int32_t old_y = p->y;
    int32_t new_y = old_y + p->vy;
    int32_t velocity = p->vy;
    for (uint16_t i = 0; i < solid_count; i++) {
        const Solid *r = &solids[i];
        if (!HorizontalOverlap(p->x, r)) continue;
        int32_t top = (int32_t)r->y * FP;
        int32_t bottom = (int32_t)(r->y + r->h) * FP;
        if (velocity > 0 && old_y + 18L * FP <= top && new_y + 18L * FP >= top) {
            new_y = top - 18L * FP;
            p->vy = 0;
        }
        if (velocity < 0 && old_y + FP >= bottom && new_y + FP < bottom) {
            new_y = bottom - FP;
            p->vy = 0;
        }
    }
    if (new_y < 0) { new_y = 0; p->vy = 0; }
    p->y = new_y;
    p->grounded = IsGrounded(p);
    if (direction && old_x != p->x && p->grounded) {
        p->walk_ticks++;
        p->frame = (uint8_t)((p->walk_ticks / 8U) & 1U);
    } else { p->walk_ticks = 0U; p->frame = 0U; }
}

static uint16_t SpritePixel(uint8_t player, int16_t x, int16_t y)
{
    const Player *p = &players[player];
    int16_t sx = x - Pixel(p->x), sy = y - Pixel(p->y);
    if (sx < 0 || sy < 0 || sx >= SPRITE_W || sy >= SPRITE_H) return 0U;
    if (p->facing_left) sx = SPRITE_W - 1 - sx;
    const uint16_t *sheet = player ? player_blue : player_red;
    return sheet[(uint16_t)sy * 24U + (uint16_t)p->frame * SPRITE_W + (uint16_t)sx];
}

static void DrawPatch(int16_t x, int16_t y, int16_t w, int16_t h)
{

    int16_t x1 = x + w, y1 = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x1 > 320) x1 = 320;
    if (y1 > 240) y1 = 240;
    if (x >= x1 || y >= y1) return;
    uint16_t n = 0U;
    for (int16_t py = y; py < y1; py++) {
        for (int16_t px = x; px < x1; px++) {
            uint16_t color = SceneCache_Pixel(px, py);
            uint16_t water = PuddlePixel(px, py);
            if (water != 0U) color = water;
            for (uint8_t i = 0; i < 2U; i++) {
                uint16_t sprite = SpritePixel(i, px, py);
                if (sprite != 0U) color = sprite;
            }
            patch[n++] = color;
        }
    }
    ILI9341_DrawRectRGB565((uint16_t)x, (uint16_t)y,
                          (uint16_t)(x1-x), (uint16_t)(y1-y), patch);
}

static void RestoreRect(int16_t x, int16_t y)
{
    DrawPatch(x, y, SPRITE_W, SPRITE_H);
}

static void DrawPuddles(void)
{
    for (uint16_t i = 0U; i < puddle_count; i++) {
        const Solid *r = &puddles[i].area;
        for (int16_t dx = 0; dx < r->w; dx += 16) {
            int16_t width = r->w - dx;
            if (width > 16) width = 16;
            DrawPatch(r->x + dx, r->y, width, r->h);
        }
    }
}

uint8_t Gameplay_Start(uint8_t level)
{
    active = 0U;
    won = 0U;
    current_level = level;
    puddles = NULL;
    puddle_count = 0U;
    if (level == 2U) {
        puddles = level2_puddles;
        puddle_count = sizeof level2_puddles / sizeof level2_puddles[0];
    } else if (level == 3U) {
        puddles = level3_puddles;
        puddle_count = sizeof level3_puddles / sizeof level3_puddles[0];
    }
    if (!SceneCache_Ready()) return 0U;
    switch (level) {
    case 1: solids = level1_solids; solid_count = sizeof level1_solids / sizeof *solids; break;
    case 2: solids = level2_solids; solid_count = sizeof level2_solids / sizeof *solids; break;
    case 3: solids = level3_solids; solid_count = sizeof level3_solids / sizeof *solids; break;
    default: return 0U;
    }
    memset(players, 0, sizeof players);

    players[0].x = (level == 1U ? 96L : (level == 2U ? 302L : 6L)) * FP;
    players[1].x = (level == 1U ? 112L : (level == 2U ? 6L : 22L)) * FP;
    players[0].y = players[1].y = (224L - SPRITE_H) * FP;
    players[0].grounded = players[1].grounded = 1U;
    memcpy(spawn_players, players, sizeof players);
    puddle_frame = 0U;
    candidates = stable = valid = jump_armed = jump_pending = 0U;
    step_at = HAL_GetTick();
    for (uint8_t i = 0; i < 7U; i++) changed_at[i] = step_at;
    active = 1U;
    DrawPuddles();
    puddle_at = HAL_GetTick();
    RestoreRect(Pixel(players[0].x), Pixel(players[0].y));
    RestoreRect(Pixel(players[1].x), Pixel(players[1].y));
    return 1U;
}

uint8_t Gameplay_Update(uint8_t raw_buttons)
{
    if (!active || won) return 0U;
    uint32_t now = HAL_GetTick();
    uint8_t rising = 0U;
    for (uint8_t i = 0; i < 7U; i++) {
        uint8_t bit = (uint8_t)(1U << i);
        if ((raw_buttons & bit) != (candidates & bit)) {
            candidates ^= bit;
            changed_at[i] = now;
        }
        if ((uint32_t)(now - changed_at[i]) >= DEBOUNCE_MS) {
            valid |= bit;
            if ((stable & bit) != (candidates & bit)) {
                stable ^= bit;
                if (stable & bit) rising |= bit;
            }
        }
    }
    if (rising & PLAY_PAUSE) { active = 0U; return 1U; }
    const uint8_t jump_bits[2] = {PLAY_RED_JUMP, PLAY_BLUE_JUMP};
    for (uint8_t i = 0; i < 2U; i++) {
        uint8_t bit = jump_bits[i];
        if ((valid & bit) && !(stable & bit)) jump_armed |= bit;
        if ((rising & bit) && (jump_armed & bit)) {
            jump_pending |= bit;
            jump_armed &= (uint8_t)~bit;
        }
    }
    Player old[2] = {players[0], players[1]};
    uint8_t steps = 0U;
    while ((uint32_t)(now - step_at) >= STEP_MS && steps < 5U) {
        step_at += STEP_MS;
        int8_t red_dir = ((stable & PLAY_RED_RIGHT) != 0U) - ((stable & PLAY_RED_LEFT) != 0U);
        int8_t blue_dir = ((stable & PLAY_BLUE_RIGHT) != 0U) - ((stable & PLAY_BLUE_LEFT) != 0U);
        MovePlayer(&players[0], red_dir, (jump_pending & PLAY_RED_JUMP) != 0U);
        MovePlayer(&players[1], blue_dir, (jump_pending & PLAY_BLUE_JUMP) != 0U);
        if (CheckPuddles()) {

            step_at = now;
            break;
        }
        jump_pending = 0U;
        steps++;
        if (BothAtOwnDoors()) { won = 1U; break; }
    }
    if ((uint32_t)(now - step_at) >= STEP_MS) step_at = now;
    if (puddle_count != 0U && (uint32_t)(now - puddle_at) >= PUDDLE_FRAME_MS) {
        uint32_t frames = (uint32_t)(now - puddle_at) / PUDDLE_FRAME_MS;
        puddle_at += frames * PUDDLE_FRAME_MS;
        puddle_frame = (uint8_t)((puddle_frame + frames) & 3U);
        DrawPuddles();
    }
    for (uint8_t i = 0; i < 2U; i++) {
        if (Pixel(old[i].x) != Pixel(players[i].x) || Pixel(old[i].y) != Pixel(players[i].y) ||
            old[i].frame != players[i].frame || old[i].facing_left != players[i].facing_left) {
            RestoreRect(Pixel(old[i].x), Pixel(old[i].y));
            RestoreRect(Pixel(players[i].x), Pixel(players[i].y));
        }
    }
    return 0U;
}

uint8_t Gameplay_Resume(void)
{
    if (!SceneCache_Ready() || current_level < 1U || current_level > 3U) return 0U;
    static uint16_t restore_row[320];
    for (uint16_t y = 0U; y < 240U; y++) {
        for (uint16_t x = 0U; x < 320U; x++)
            restore_row[x] = SceneCache_Pixel((int16_t)x, (int16_t)y);
        ILI9341_DrawRectRGB565(0U, y, 320U, 1U, restore_row);
    }
    DrawPuddles();
    RestoreRect(Pixel(players[0].x), Pixel(players[0].y));
    RestoreRect(Pixel(players[1].x), Pixel(players[1].y));

    step_at = HAL_GetTick();
    puddle_at = step_at;
    candidates = stable = valid = jump_armed = jump_pending = 0U;
    for (uint8_t i = 0U; i < 7U; i++) changed_at[i] = step_at;
    active = 1U;
    return 1U;
}
