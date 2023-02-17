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
#include "iomem.h"

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

void crop_image(uint8_t *src_image, uint32_t x1, uint32_t y1, uint32_t w, uint32_t h, uint8_t *crop_image)
{
    uint8_t *v_src_r, *v_src_g, *v_src_b, *v_crop_r, *v_crop_g, *v_crop_b;
    uint32_t crop_w, crop_h;

    v_src_r = src_image;
    v_src_g = src_image + 320 * 240;
    v_src_b = src_image + 320 * 240 * 2;

    crop_w = w;
    crop_h = h;
    
    v_crop_r = crop_image;
    v_crop_g = crop_image + crop_w * crop_h;
    v_crop_b = crop_image + crop_w * crop_h * 2;

    uint32_t v_width_offset = x1;
    uint32_t v_height_offset = y1;

    uint32_t i;
    uint32_t j;
#if LCD_VERTICAL
    for(i = 0; i < 128; i++)
    {
        for(j = 0; j < 128; j++)
        {
            v_crop_r[i*128 + j] = v_src_r[((127 -j) + v_height_offset) * 320 + i + v_width_offset];
            v_crop_g[i*128 + j] = v_src_g[((127 -j) + v_height_offset) * 320 + i + v_width_offset];
            v_crop_b[i*128 + j] = v_src_b[((127 -j) + v_height_offset) * 320 + i + v_width_offset];
        }
    }
#else
    for(i = 0; i < crop_h; i++)
    {
        for(j = 0; j < crop_w; j++)
        {
            v_crop_r[i*crop_w + j] = v_src_r[(i + v_height_offset) * 320 + j + v_width_offset];
            v_crop_g[i*crop_w + j] = v_src_g[(i + v_height_offset) * 320 + j + v_width_offset];
            v_crop_b[i*crop_w + j] = v_src_b[(i + v_height_offset) * 320 + j + v_width_offset];
        }
    }
#endif
}

uint8_t image_resize(uint8_t *src, uint16_t w_src, uint16_t h_src, uint16_t w_dst, uint16_t h_dst, uint8_t *dst)
{
    uint16_t x, y;
    uint16_t x1, x2, y1, y2;
    double w_scale_v, h_scale_v;
    double temp1, temp2;
    double x_src, y_src;
    
    if (src == NULL || dst == NULL)
        return -1;

    if (w_src == 0 || h_src == 0 || w_dst ==0 || h_dst == 0)
        return -1;

    w_scale_v = (double)w_src / w_dst;
    h_scale_v = (double)h_src / h_dst;

    for (y = 0; y < h_dst; y++)
    {
        for (x = 0; x < w_dst; x++)
        {
            x_src = (x + 0.5) * w_scale_v - 0.5;
            x1 = (uint16_t)x_src;
            x2 = x1 + 1;
            y_src = (y + 0.5) * h_scale_v - 0.5;
            y1 = (uint16_t)y_src;
            y2 = y1 + 1;

            if (x2 >= w_src || y2 >= h_src)
            {    *(dst + x + y * w_dst) = *(src + x1 + y1 * w_src);
                continue;
            }

            temp1 = (x2 - x_src) * *(src + x1 + y1 * w_src) + (x_src - x1) * *(src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(src + x1 + y2 * w_src) + (x_src - x1) * *(src + x2 + y2 * w_src);
            *(dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
        }
    }
    return 0;        
}


void flip_left_right(uint8_t *src, uint16_t width)
{
	uint8_t temp_data = 0;
	for(uint32_t ww = 0; ww < width; ww++)
	{
		uint32_t index = (width - ww / 2 - 1) * 2 + ww % 2;
		temp_data = src[index];
		src[index] = src[ww];
		src[ww] = temp_data;		
	}
}