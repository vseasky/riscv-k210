#ifndef _FLASH_H
#define _FLASH_H
#include "face_info.h"
#include <stdint.h>
#include "system_config.h"

extern volatile face_save_info_t g_face_save_info;
extern face_info_t g_face_info;
extern volatile uint32_t g_wdt_reboot_count;

void flash_init(void);

int flash_get_face_img(uint8_t *image, uint32_t id);

int flash_delete_face_info(uint32_t id);
int flash_save_face_info(uint8_t *image, float *features);
int calulate_score(float *features, float *score);
int flash_delete_face_all(void);
uint32_t flash_get_wdt_reboot_count(void);
float calCosinDistance(float *faceFeature0P, float *faceFeature1P, int featureLen);

#endif
