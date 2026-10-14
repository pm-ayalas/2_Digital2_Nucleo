#ifndef LEVEL_IMAGES_H
#define LEVEL_IMAGES_H
#include <stdint.h>
typedef struct {
    const uint16_t *runs;
    uint32_t pair_count;
} LevelImage;
extern const LevelImage level_images[4];
#endif
