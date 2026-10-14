#ifndef DUAL_INPUT_H
#define DUAL_INPUT_H
#include "wire_protocol.h"
typedef struct {
    WireParser parser;
    uint32_t received, frames, errors;
    uint8_t buttons, connected, seen, pending;
} PlayerInput;
static inline void PlayerInput_Byte(PlayerInput *p, unsigned player,
                                    uint8_t byte, uint32_t now) {
    if (!WireFeed(&p->parser, byte)) return;
    if (p->parser.p[3] != 0x11u + player || p->parser.p[4] > 15 || p->parser.p[5] > 1) {
        p->errors++; return;
    }
    uint8_t b = p->parser.p[5] ? p->parser.p[4] : 0;
    if (p->seen && (uint32_t)(now-p->received) >= 300u) {
        p->buttons=0; p->pending=0;
    }
    if (!p->parser.p[5]) p->pending=0;
    p->pending |= (uint8_t)(b & (uint8_t)~p->buttons & 12u);
    p->buttons=b; p->connected=p->parser.p[5]; p->received=now;
    p->seen=1; p->frames++;
}
static inline uint8_t PlayerInput_Alive(const PlayerInput *p, uint32_t now) {
    return p->seen && p->connected && (uint32_t)(now-p->received)<300u;
}
#endif
