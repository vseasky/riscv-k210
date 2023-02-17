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
#include <stdbool.h>

typedef struct
{
    uint8_t *addr;
    uint16_t width;
    uint16_t height;
    uint16_t pixel;
    uint16_t format;
} image_t;

typedef enum {
    ROTATE_0 = 0,
    ROTATE_90,
    ROTATE_180,
	ROTATE_270,
    IMAGE_MODE_MAX,
} image_mode_t;

int image_init(image_t *image);
void image_rotate(image_t *image_src, image_t *image_dst, image_mode_t img_rotate);
void image_resize(image_t *image_src, image_t *image_dst, bool bilinear);
void image_absolute_src_resize(image_t *image_src, image_t *image_dst, uint16_t *x_start, uint16_t *y_start, uint16_t *x_end, uint16_t *y_end, float margin, bool bilinear);
void image_umeyama(float *src, float *dst, float *umeyama_args);
void image_similarity(image_t *image_src, image_t *image_dst, float *T);
void image_deinit(image_t *image);

#endif /* _IMAGE_PROCESS_H */
