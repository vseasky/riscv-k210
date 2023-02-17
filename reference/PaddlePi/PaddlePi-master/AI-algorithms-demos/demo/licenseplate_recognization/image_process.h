/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _IMAGE_PROCESS_H
#define _IMAGE_PROCESS_H

#include <stdint.h>

typedef struct
{
    uint8_t *addr;
    uint16_t width;
    uint16_t height;
    uint16_t pixel;
    uint16_t format;
} image_t;


int image_init(image_t *image);
void image_deinit(image_t *image);
void crop_image(uint8_t *src_image, uint32_t x1, uint32_t y1, uint32_t w, uint32_t h, uint8_t *crop_image);
void flip_left_right(uint8_t *src, uint16_t width);
uint8_t image_resize(uint8_t *src, uint16_t w_src, uint16_t h_src, uint16_t w_dst, uint16_t h_dst, uint8_t *dst);

#endif /* _IMAGE_PROCESS_H */
