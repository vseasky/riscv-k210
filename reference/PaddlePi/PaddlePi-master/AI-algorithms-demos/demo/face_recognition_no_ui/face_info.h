#ifndef _FACE_INFO_H_
#define _FACE_INFO_H_
#include <stdint.h>
#include "system_config.h"

typedef struct _face_info_t
{
//    uint32_t reserved0[4];
//    char name[32];
    uint16_t id;
    float info[196];
    uint16_t valid;
//    uint32_t reserve0;
} face_info_t;

typedef struct _face_save_info_t
{
    uint32_t header;
    uint32_t version;
    uint32_t number;
    uint32_t checksum;
    uint32_t face_info_index[FACE_DATA_MAX_COUNT / 4 * 4 + 1];
}__attribute__((packed, aligned(8))) face_save_info_t;

#endif
