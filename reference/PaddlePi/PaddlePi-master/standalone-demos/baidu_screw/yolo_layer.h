#ifndef _YOLO_LAYER
#define _YOLO_LAYER

#include <stdint.h>
#include "kpu.h"

typedef struct
{
    uint32_t obj_number;
    struct
    {
        uint32_t x1;
        uint32_t y1;
        uint32_t x2;
        uint32_t y2;
        uint32_t class_id;
        float prob;
    } obj[10];
} obj_info_t;

typedef struct
{
    float threshold;
    float nms_value;
    uint32_t coords;
    uint32_t anchor_number;
    float *anchor;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t classes;
    uint32_t net_width;
    uint32_t net_height;
    uint32_t layer_width;
    uint32_t layer_height;
    uint32_t boxes_number;
    uint32_t output_number;
    void *boxes;
    float *input;
    float *output;
    float *probs_buf;
    float **probs;
} yolo_layer_t;

typedef void (*callback_draw_box)(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob);

int yolo_layer_init(yolo_layer_t *yl, int width, int height, int channels, int origin_width, int origin_height);
void yolo_layer_deinit(yolo_layer_t *yl);
void yolo_layer_run(yolo_layer_t *yl, obj_info_t *obj_info);
void yolo_layer_draw_boxes(yolo_layer_t *yl, callback_draw_box callback);

#endif // _YOLO_LAYER
