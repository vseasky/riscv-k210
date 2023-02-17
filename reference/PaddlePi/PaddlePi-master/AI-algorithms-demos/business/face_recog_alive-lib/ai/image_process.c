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
#include "string.h"
#include <math.h>

int image_init(image_t *image)
{
    image->addr = iomem_malloc(image->width * image->height * image->pixel);
    if (image->addr == NULL)
        return -1;
    return 0;
}

void image_rotate(image_t *image_src, image_t *image_dst, image_mode_t img_rotate)
{
    switch(img_rotate)
    {
        case ROTATE_0:
            memcpy(image_dst->addr, image_src->addr, image_src->height * image_src->width * image_src->pixel);
            break;
        case ROTATE_90:
            for(uint16_t hh = 0; hh < image_src->height; hh++)
            {
                for(uint16_t ww = 0; ww < image_src->width; ww++)
                {
                    uint16_t src_index = hh * image_src->width + ww;
                    uint16_t dst_index = (image_src->width - 1 - ww) * image_dst->width + hh;
                    for(uint16_t cc = 0; cc < image_src->pixel; cc++)
                    {
                        uint8_t *pdst = image_dst->addr + cc * image_dst->height * image_dst->width;
                        uint8_t *psrc = image_src->addr + cc * image_src->height * image_src->width;
                        pdst[dst_index] = psrc[src_index];
                    }
                }
            }
            break;
        case ROTATE_180:
            for(uint16_t cc = 0; cc < image_dst->pixel; cc++)
            {
                for(uint16_t hh = 0; hh < image_dst->height; hh++)
                {
                    memcpy(image_dst->addr + (image_dst->height - 1 - hh) * image_dst->width, image_src->addr + hh * image_src->width, image_dst->width);
                }
            }
            break;
        case ROTATE_270:
            for(uint16_t hh = 0; hh < image_src->height; hh++)
            {
                for(uint16_t ww = 0; ww < image_src->width; ww++)
                {
                    uint16_t src_index = hh * image_src->width + ww;
                    uint16_t dst_index = ww * image_dst->width + image_src->height - 1 - hh;
                    for(uint16_t cc = 0; cc < image_src->pixel; cc++)
                    {
                        uint8_t *pdst = image_dst->addr + cc * image_dst->height * image_dst->width;
                        uint8_t *psrc = image_src->addr + cc * image_src->height * image_src->width;
                        pdst[dst_index] = psrc[src_index];
                    }
                }
            }
            break;
        default:
            break;
    }
}

void image_resize(image_t *image_src, image_t *image_dst, bool bilinear)
{
    uint16_t x1, x2, y1, y2;
    float w_scale, h_scale;
    float temp1, temp2;
    float x_src, y_src;

    uint8_t *r_src, *g_src, *b_src, *r_dst, *g_dst, *b_dst;
    uint16_t w_src, h_src, w_dst, h_dst;

    w_src = image_src->width;
    h_src = image_src->height;
    r_src = image_src->addr;
    g_src = r_src + w_src * h_src;
    b_src = g_src + w_src * h_src;
    w_dst = image_dst->width;
    h_dst = image_dst->height;
    r_dst = image_dst->addr;
    g_dst = r_dst + w_dst * h_dst;
    b_dst = g_dst + w_dst * h_dst;

    w_scale = (float)w_src / w_dst;
    h_scale = (float)h_src / h_dst;

    for(uint16_t y = 0; y < h_dst; y++)
    {
        for(uint16_t x = 0; x < w_dst; x++)
        {
            x_src = (x + 0.5f) * w_scale - 0.5f;
            x1 = (uint16_t)x_src;
            x2 = x1 + 1;
            y_src = (y + 0.5f) * h_scale - 0.5f;
            y1 = (uint16_t)y_src;
            y2 = y1 + 1;

            if((!bilinear) || (x2 >= w_src || y2 >= h_src))
            {
                *(r_dst + x + y * w_dst) = *(r_src + x1 + y1 * w_src);
                *(g_dst + x + y * w_dst) = *(g_src + x1 + y1 * w_src);
                *(b_dst + x + y * w_dst) = *(b_src + x1 + y1 * w_src);
                continue;
            }

            temp1 = (x2 - x_src) * *(r_src + x1 + y1 * w_src) + (x_src - x1) * *(r_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(r_src + x1 + y2 * w_src) + (x_src - x1) * *(r_src + x2 + y2 * w_src);
            *(r_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
            temp1 = (x2 - x_src) * *(g_src + x1 + y1 * w_src) + (x_src - x1) * *(g_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(g_src + x1 + y2 * w_src) + (x_src - x1) * *(g_src + x2 + y2 * w_src);
            *(g_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
            temp1 = (x2 - x_src) * *(b_src + x1 + y1 * w_src) + (x_src - x1) * *(b_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(b_src + x1 + y2 * w_src) + (x_src - x1) * *(b_src + x2 + y2 * w_src);
            *(b_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
        }
    }
}

void image_absolute_src_resize(image_t *image_src, image_t *image_dst, uint16_t *x_start, uint16_t *y_start, uint16_t *x_end, uint16_t *y_end, float margin, bool bilinear)
{
    uint32_t width = *x_end - *x_start + 1;
    uint32_t height = *y_end - *y_start + 1;
    float new_x_start = *x_start - (margin * width);
    float new_x_end = *x_end + (margin * width);
    float new_y_start = *y_start - (margin * height);
    float new_y_end = *y_end + (margin * height);
    *x_start = new_x_start > 1 ? floorf(new_x_start) : 1;
    *x_end = new_x_end < image_src->width ? floorf(new_x_end) : image_src->width - 1;
    *y_start = new_y_start > 1 ? floorf(new_y_start) : 1;
    *y_end = new_y_end < image_src->height ? floorf(new_y_end) : image_src->height - 1;
    uint16_t x1, x2, y1, y2;
    float w_scale, h_scale;
    float temp1, temp2;
    float x_src, y_src;

    uint8_t *r_src, *g_src, *b_src, *r_dst, *g_dst, *b_dst;
    uint16_t w_src, h_src, w_dst, h_dst;

    w_src = image_src->width;
    h_src = image_src->height;
    r_src = image_src->addr;
    g_src = r_src + w_src * h_src;
    b_src = g_src + w_src * h_src;
    w_dst = image_dst->width;
    h_dst = image_dst->height;
    r_dst = image_dst->addr;
    g_dst = r_dst + w_dst * h_dst;
    b_dst = g_dst + w_dst * h_dst;

    w_scale = (float)(*x_end - *x_start + 1) / w_dst;
    h_scale = (float)(*y_end - *y_start + 1) / h_dst;

    for(uint16_t y = 0; y < h_dst; y++)
    {
        for(uint16_t x = 0; x < w_dst; x++)
        {
            x_src = (x + 0.5f) * w_scale - 0.5f;
            x_src += *x_start;
            x1 = (uint16_t)x_src;
            x2 = x1 + 1;
            y_src = (y + 0.5f) * h_scale - 0.5f;
            y_src += *y_start;
            y1 = (uint16_t)y_src;
            y2 = y1 + 1;

            if((!bilinear) || (x2 >= w_src || y2 >= h_src))
            {
                *(r_dst + x + y * w_dst) = *(r_src + x1 + y1 * w_src);
                *(g_dst + x + y * w_dst) = *(g_src + x1 + y1 * w_src);
                *(b_dst + x + y * w_dst) = *(b_src + x1 + y1 * w_src);
                continue;
            }

            temp1 = (x2 - x_src) * *(r_src + x1 + y1 * w_src) + (x_src - x1) * *(r_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(r_src + x1 + y2 * w_src) + (x_src - x1) * *(r_src + x2 + y2 * w_src);
            *(r_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
            temp1 = (x2 - x_src) * *(g_src + x1 + y1 * w_src) + (x_src - x1) * *(g_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(g_src + x1 + y2 * w_src) + (x_src - x1) * *(g_src + x2 + y2 * w_src);
            *(g_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
            temp1 = (x2 - x_src) * *(b_src + x1 + y1 * w_src) + (x_src - x1) * *(b_src + x2 + y1 * w_src);
            temp2 = (x2 - x_src) * *(b_src + x1 + y2 * w_src) + (x_src - x1) * *(b_src + x2 + y2 * w_src);
            *(b_dst + x + y * w_dst) = (uint8_t)((y2 - y_src) * temp1 + (y_src - y1) * temp2);
        }
    }
}

static void svd22(const float a[4], float u[4], float s[2], float v[4])
{
    s[0] = (sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)) + sqrtf(powf(a[0] + a[3], 2) + powf(a[1] - a[2], 2))) / 2;
    s[1] = fabsf(s[0] - sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)));
    v[2] = (s[0] > s[1]) ? sinf((atan2f(2 * (a[0] * a[1] + a[2] * a[3]), a[0] * a[0] - a[1] * a[1] + a[2] * a[2] - a[3] * a[3])) / 2) : 0;
    v[0] = sqrtf(1 - v[2] * v[2]);
    v[1] = -v[2];
    v[3] = v[0];
    u[0] = (s[0] != 0) ? -(a[0] * v[0] + a[1] * v[2]) / s[0] : 1;
    u[2] = (s[0] != 0) ? -(a[2] * v[0] + a[3] * v[2]) / s[0] : 0;
    u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
    u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
    v[0] = -v[0];
    v[2] = -v[2];
}

void image_umeyama(float *src, float *dst, float *umeyama_args)
{
#define SRC_NUM 5
#define SRC_DIM 2
    int i, j, k;
    float src_mean[SRC_DIM] = {0.0};
    float dst_mean[SRC_DIM] = {0.0};
    for(i = 0; i < SRC_NUM * 2; i += 2)
    {
        src_mean[0] += src[i];
        src_mean[1] += src[i + 1];
        dst_mean[0] += umeyama_args[i];
        dst_mean[1] += umeyama_args[i + 1];
    }
    src_mean[0] /= SRC_NUM;
    src_mean[1] /= SRC_NUM;
    dst_mean[0] /= SRC_NUM;
    dst_mean[1] /= SRC_NUM;

    float src_demean[SRC_NUM][2] = {0.0};
    float dst_demean[SRC_NUM][2] = {0.0};

    for(i = 0; i < SRC_NUM; i++)
    {
        src_demean[i][0] = src[2 * i] - src_mean[0];
        src_demean[i][1] = src[2 * i + 1] - src_mean[1];
        dst_demean[i][0] = umeyama_args[2 * i] - dst_mean[0];
        dst_demean[i][1] = umeyama_args[2 * i + 1] - dst_mean[1];
    }

    float A[SRC_DIM][SRC_DIM] = {0.0};
    for(i = 0; i < SRC_DIM; i++)
    {
        for(k = 0; k < SRC_DIM; k++)
        {
            for(j = 0; j < SRC_NUM; j++)
            {
                A[i][k] += dst_demean[j][i] * src_demean[j][k];
            }
            A[i][k] /= SRC_NUM;
        }
    }

    float(*T)[SRC_DIM + 1] = (float(*)[SRC_DIM + 1]) dst;
    T[0][0] = 1;
    T[0][1] = 0;
    T[0][2] = 0;
    T[1][0] = 0;
    T[1][1] = 1;
    T[1][2] = 0;
    T[2][0] = 0;
    T[2][1] = 0;
    T[2][2] = 1;

    float U[SRC_DIM][SRC_DIM] = {0};
    float S[SRC_DIM] = {0};
    float V[SRC_DIM][SRC_DIM] = {0};
    svd22(&A[0][0], &U[0][0], S, &V[0][0]);

    T[0][0] = U[0][0] * V[0][0] + U[0][1] * V[1][0];
    T[0][1] = U[0][0] * V[0][1] + U[0][1] * V[1][1];
    T[1][0] = U[1][0] * V[0][0] + U[1][1] * V[1][0];
    T[1][1] = U[1][0] * V[0][1] + U[1][1] * V[1][1];

    float scale = 1.0;
    float src_demean_mean[SRC_DIM] = {0.0};
    float src_demean_var[SRC_DIM] = {0.0};
    for(i = 0; i < SRC_NUM; i++)
    {
        src_demean_mean[0] += src_demean[i][0];
        src_demean_mean[1] += src_demean[i][1];
    }
    src_demean_mean[0] /= SRC_NUM;
    src_demean_mean[1] /= SRC_NUM;

    for(i = 0; i < SRC_NUM; i++)
    {
        src_demean_var[0] += (src_demean_mean[0] - src_demean[i][0]) * (src_demean_mean[0] - src_demean[i][0]);
        src_demean_var[1] += (src_demean_mean[1] - src_demean[i][1]) * (src_demean_mean[1] - src_demean[i][1]);
    }
    src_demean_var[0] /= (SRC_NUM);
    src_demean_var[1] /= (SRC_NUM);
    scale = 1.0 / (src_demean_var[0] + src_demean_var[1]) * (S[0] + S[1]);
    T[0][2] = dst_mean[0] - scale * (T[0][0] * src_mean[0] + T[0][1] * src_mean[1]);
    T[1][2] = dst_mean[1] - scale * (T[1][0] * src_mean[0] + T[1][1] * src_mean[1]);
    T[0][0] *= scale;
    T[0][1] *= scale;
    T[1][0] *= scale;
    T[1][1] *= scale;
}

void image_similarity(image_t *image_src, image_t *image_dst, float *T)
{
    int width = image_src->width;
    int height = image_src->height;
    int channels = image_src->pixel;
    int step = width;
    int color_step = width * height;
    int sim_step;
    int i, j, k;

    /* Init image info handled */
    int sim_color_step = image_dst->width * image_dst->height;
    sim_step = image_dst->width;

    int pre_x, pre_y; /* pixel position */
    int x, y;
    unsigned short color[2][2];
    float(*TT)[3] = (float(*)[3])T;

    /* Äæ¾ØÕó²ÎÊý¼ÆËã */
    float ax = TT[1][1] / (TT[1][1] * TT[0][0] - TT[0][1] * TT[1][0]);
    float ay = TT[0][1] / (TT[1][1] * TT[0][0] - TT[0][1] * TT[1][0]);
    float az = (TT[1][2] * TT[0][1] - TT[1][1] * TT[0][2]) / (TT[1][1] * TT[0][0] - TT[0][1] * TT[1][0]);
    float bx = TT[1][0] / (TT[1][0] * TT[0][1] - TT[1][1] * TT[0][0]);
    float by = TT[0][0] / (TT[1][0] * TT[0][1] - TT[1][1] * TT[0][0]);
    float bz = (TT[1][2] * TT[0][0] - TT[1][0] * TT[0][2]) / (TT[1][0] * TT[0][1] - TT[1][1] * TT[0][0]);
    for(i = 0; i < image_dst->height; i++)
    {
        for(j = 0; j < image_dst->width; j++)
        {
            pre_x = (int)(ax * (j << 8) - ay * (i << 8) + az * (1 << 8));
            pre_y = (int)(bx * (j << 8) - by * (i << 8) + bz * (1 << 8));

            y = pre_y & 0xFF;
            x = pre_x & 0xFF;
            pre_x >>= 8;
            pre_y >>= 8;
            if(pre_x < 0 || pre_x > (width - 1) || pre_y < 0 || pre_y > (height - 1))
                continue;
            for(k = 0; k < channels; k++)
            {
                color[0][0] = image_src->addr[pre_y * step + pre_x + k * color_step];
                color[1][0] = image_src->addr[pre_y * step + (pre_x + 1) + k * color_step];
                color[0][1] = image_src->addr[(pre_y + 1) * step + pre_x + k * color_step];
                color[1][1] = image_src->addr[(pre_y + 1) * step + (pre_x + 1) + k * color_step];
                int final = (0x100 - x) * (0x100 - y) * color[0][0] + x * (0x100 - y) * color[1][0] + (0x100 - x) * y * color[0][1] + x * y * color[1][1];
                final = final >> 16;
                image_dst->addr[i * sim_step + j + k * sim_color_step] = final;
            }
        }
    }
}

void image_deinit(image_t *image)
{
    iomem_free(image->addr);
}
