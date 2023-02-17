#ifndef _PCH_H
#define _PCH_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// supression value
#define MIN_FLOAT 1e-9f //fixed
// white and black
#ifdef	WHITE
#undef	WHITE
#endif
#ifdef	BLACK
#undef	BLACK
#endif
#define WHITE 255 //fixed
#define BLACK 0   //fixed
#define PI 3.14159265f //fixed
#define HIST_NUM 1024 //fixed

// rgb888 转 灰度图
void make_rgb2gray(uint8_t *rgb, uint32_t height, uint32_t width, uint8_t *gray);
//中值滤波
// Mirror超出边界的坐标
uint32_t get_range_coord(int32_t input_coord, uint32_t boarder);
// Canny边缘检测
// 高斯滤波
void K3x3_filter(uint8_t *input, uint32_t height, uint32_t width, float *filter, uint8_t *output);
// Soble算子滤波，得到x梯度和y梯度，得到梯度的角度
void K3x3_2_filter_3_output(uint8_t *input, uint32_t height, uint32_t width, float *filter_x, float *filter_y, float *output0, float *output1, float *output2);
// 根据角度和梯度值做一个非极大值抑制
void Nms(float *input0, float *input1, float *input2, uint32_t height, uint32_t width, float min_thresh, float max_thresh, float *min_value, float *max_value, uint8_t *output);
// 对于中间态的边缘点，做一个八邻域矫正
void refine(uint8_t *input, uint32_t height, uint32_t width, float min_thresh, float max_thresh, uint8_t *output);

#endif