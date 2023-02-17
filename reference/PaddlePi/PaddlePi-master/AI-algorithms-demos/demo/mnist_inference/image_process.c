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

#include <stdlib.h>
#include "image_process.h"
#include <math.h>
#include "iomem.h"
#include <stdio.h>

int image_init(image_t *image)
{
    image->addr = iomem_malloc(image->width * image->height * image->pixel);
    if (image->addr == NULL)
        return -1;
    return 0;
}

void image_deinit(image_t *image)
{
    iomem_free(image->addr);
}

void image_crop(image_t *image_src, image_t *image_dst, uint16_t x_offset, uint16_t y_offset)
{
    uint8_t *src, *r_src, *g_src, *b_src, *dst, *r_dst, *g_dst, *b_dst;
    uint16_t w_src, h_src, w_dst, h_dst;

    src = image_src->addr;
    w_src = image_src->width;
    h_src = image_src->height;
    dst = image_dst->addr;
    w_dst = image_dst->width;
    h_dst = image_dst->height;

    r_src = src + y_offset * w_src + x_offset;
    g_src = r_src + w_src * h_src;
    b_src = g_src + w_src * h_src;
    r_dst = dst;
    g_dst = r_dst + w_dst * h_dst;
    b_dst = g_dst + w_dst * h_dst;

    for(uint16_t y = 0; y < h_dst; y++)
    {
        for(uint16_t x = 0; x < w_dst; x++)
        {
            *r_dst++ = r_src[x];
            *g_dst++ = g_src[x];
            *b_dst++ = b_src[x];
        }
        r_src += w_src;
        g_src += w_src;
        b_src += w_src;
    }
}

void rgb2gray(image_t *image_src, image_t *image_dst)
{
    uint32_t area = image_src->height * image_src->width;
    for(uint32_t hh = 0; hh < image_src->height; hh++)
    {
        for(uint32_t ww = 0; ww < image_src->width; ww++)
        {
            uint32_t index = hh * image_src->width + ww;
            uint8_t r = image_src->addr[index];
            uint8_t g = image_src->addr[index + area];
            uint8_t b = image_src->addr[index + 2 * area];
            float fgray = 0.30f * r + 0.59 * g + 0.11 * b;
            image_dst->addr[index] = (uint8_t)(floorf(fgray));
            //printf("%d ", image_dst->addr[index]);
        }
        //printf("\n");
    }
}

