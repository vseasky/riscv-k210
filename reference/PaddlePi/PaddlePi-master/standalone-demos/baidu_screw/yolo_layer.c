#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "yolo_layer.h"

typedef struct
{
    float x;
    float y;
    float w;
    float h;
} box_t;

typedef struct
{
    int index;
    int class;
    float **probs;
} sortable_box_t;


int yolo_layer_init(yolo_layer_t *yl, int width, int height, int channels, int origin_width, int origin_height)
{
    int flag = 0;

    yl->coords = 4;

    yl->classes = channels / yl->anchor_number - 5;
    yl->net_width = origin_width;
    yl->net_height = origin_height;
    yl->layer_width = width;
    yl->layer_height = height;
    yl->boxes_number = (yl->layer_width * yl->layer_height * yl->anchor_number); 
    yl->output_number = (yl->boxes_number * (yl->classes + yl->coords + 1));

    yl->output = malloc(yl->output_number * sizeof(float));
    if (yl->output == NULL)
    {
        flag = -1;
        goto malloc_error;
    }
    yl->boxes = malloc(yl->boxes_number * sizeof(box_t));
    if (yl->boxes == NULL)
    {
        flag = -2;
        goto malloc_error;
    }
    yl->probs_buf = malloc(yl->boxes_number * (yl->classes + 1) * sizeof(float));
    if (yl->probs_buf == NULL)
    {
        flag = -3;
        goto malloc_error;
    }
    yl->probs = malloc(yl->boxes_number * sizeof(float *));
    if (yl->probs == NULL)
    {
        flag = -4;
        goto malloc_error;
    }
    for (uint32_t i = 0; i < yl->boxes_number; i++)
        yl->probs[i] = &(yl->probs_buf[i * (yl->classes + 1)]);
    return 0;
malloc_error:
    free(yl->output);
    free(yl->boxes);
    free(yl->probs_buf);
    free(yl->probs);
    return flag;
}

void yolo_layer_deinit(yolo_layer_t *yl)
{
    free(yl->output);
    free(yl->boxes);
    free(yl->probs_buf);
    free(yl->probs);
}

static inline float sigmoid(float x)
{
    return 1.f / (1.f + expf(-x));
}

static void activate_array(yolo_layer_t *yl, int index, int n)
{
    float *output = &yl->output[index];
    float *input = &yl->input[index];

    for (int i = 0; i < n; ++i)
        output[i] = sigmoid(input[i]);
}

static int entry_index(yolo_layer_t *yl, int location, int entry)
{
    int wh = yl->layer_width * yl->layer_height;
    int n   = location / wh;
    int loc = location % wh;

    return n * wh * (yl->coords + yl->classes + 1) + entry * wh + loc;
}

static void forward_yolo_layer(yolo_layer_t *yl)
{
    int index;

    for (index = 0; index < yl->output_number; index++)
        yl->output[index] = yl->input[index];

    for (int n = 0; n < yl->anchor_number; ++n)
    {
        index = entry_index(yl, n * yl->layer_width * yl->layer_height, 0);
        activate_array(yl, index, 2 * yl->layer_width * yl->layer_height);
        index = entry_index(yl, n * yl->layer_width * yl->layer_height, 4);
        activate_array(yl, index, yl->layer_width * yl->layer_height * (yl->classes + 1));
    }
}

static void correct_yolo_boxes(yolo_layer_t *yl, box_t *boxes)
{
    uint32_t net_width = yl->net_width;
    uint32_t net_height = yl->net_height;
    uint32_t image_width = yl->net_width;
    uint32_t image_height = yl->net_height;
    uint32_t boxes_number = yl->boxes_number;
    int new_w = 0;
    int new_h = 0;

    if (((float)net_width / image_width) <
        ((float)net_height / image_height)) {
        new_w = net_width;
        new_h = (image_height * net_width) / image_width;
    } else {
        new_h = net_height;
        new_w = (image_width * net_height) / image_height;
    }
    for (int i = 0; i < boxes_number; ++i) {
        box_t b = boxes[i];

        b.x = (b.x - (net_width - new_w) / 2. / net_width) /
              ((float)new_w / net_width);
        b.y = (b.y - (net_height - new_h) / 2. / net_height) /
              ((float)new_h / net_height);
        b.w *= (float)net_width / new_w;
        b.h *= (float)net_height / new_h;
        boxes[i] = b;
    }
}

static box_t get_yolo_box(float *x, float *biases, int n, int index, int i, int j, int lw, int lh, int w, int h, int stride)
{
    volatile box_t b;

    b.x = (i + x[index + 0 * stride]) / lw;
    b.y = (j + x[index + 1 * stride]) / lh;
    b.w = expf(x[index + 2 * stride]) * biases[2 * n] / w;
    b.h = expf(x[index + 3 * stride]) * biases[2 * n + 1] / h;
    return b;
}

static void get_yolo_boxes(yolo_layer_t *yl, float *predictions, float **probs, box_t *boxes)
{
    uint32_t layer_width = yl->layer_width;
    uint32_t layer_height = yl->layer_height;
    uint32_t anchor_number = yl->anchor_number;
    uint32_t classes = yl->classes;
    uint32_t coords = yl->coords;
    float threshold = yl->threshold;

    for (int i = 0; i < layer_width * layer_height; ++i)
    {
        int row = i / layer_width;
        int col = i % layer_width;

        for (int n = 0; n < anchor_number; ++n)
        {
            int index = n * layer_width * layer_height + i;

            for (int j = 0; j < classes; ++j)
                probs[index][j] = 0;
            int obj_index = entry_index(yl, n * layer_width * layer_height + i, coords);
            int box_index = entry_index(yl, n * layer_width * layer_height + i, 0);
            float scale  = predictions[obj_index];

            boxes[index] = get_yolo_box(predictions, yl->anchor, n, box_index, col, row,
                layer_width, layer_height, yl->net_width, yl->net_height, layer_width * layer_height);

            float max = 0;

            for (int j = 0; j < classes; ++j)
            {
                int class_index = entry_index(yl, n * layer_width * layer_height + i, coords + 1 + j);
                float prob = scale * predictions[class_index];

                probs[index][j] = (prob > threshold) ? prob : 0;
                if (prob > max)
                    max = prob;
            }
            probs[index][classes] = max;
        }
    }
    correct_yolo_boxes(yl, boxes);
}

static int nms_comparator(void *pa, void *pb)
{
    sortable_box_t a = *(sortable_box_t *)pa;
    sortable_box_t b = *(sortable_box_t *)pb;
    float diff = a.probs[a.index][b.class] - b.probs[b.index][b.class];

    if (diff < 0)
        return 1;
    else if (diff > 0)
        return -1;
    return 0;
}

static float oveylap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static float box_intersection(box_t a, box_t b)
{
    float w = oveylap(a.x, a.w, b.x, b.w);
    float h = oveylap(a.y, a.h, b.y, b.h);

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static float box_union(box_t a, box_t b)
{
    float i = box_intersection(a, b);
    float u = a.w * a.h + b.w * b.h - i;

    return u;
}

static float box_iou(box_t a, box_t b)
{
    return box_intersection(a, b) / box_union(a, b);
}

static void do_nms_sort(yolo_layer_t *yl, box_t *boxes, float **probs)
{
    uint32_t boxes_number = yl->boxes_number;
    uint32_t classes = yl->classes;
    float nms_value = yl->nms_value;
    int i, j, k;
    sortable_box_t s[boxes_number];

    for (i = 0; i < boxes_number; ++i)
    {
        s[i].index = i;
        s[i].class = 0;
        s[i].probs = probs;
    }

    for (k = 0; k < classes; ++k)
    {
        for (i = 0; i < boxes_number; ++i)
            s[i].class = k;
        qsort(s, boxes_number, sizeof(sortable_box_t), nms_comparator);
        for (i = 0; i < boxes_number; ++i)
        {
            if (probs[s[i].index][k] == 0)
                continue;
            box_t a = boxes[s[i].index];

            for (j = i + 1; j < boxes_number; ++j)
            {
                box_t b = boxes[s[j].index];

                if (box_iou(a, b) > nms_value)
                    probs[s[j].index][k] = 0;
            }
        }
    }
}

static int max_index(float *a, int n)
{
    int i, max_i = 0;
    float max = a[0];

    for (i = 1; i < n; ++i)
    {
        if (a[i] > max)
        {
            max   = a[i];
            max_i = i;
        }
    }
    return max_i;
}

static void yolo_layer_output(yolo_layer_t *yl, obj_info_t *obj_info)
{
    uint32_t obj_number = 0;
    uint32_t image_width = yl->image_width;
    uint32_t image_height = yl->image_height;
    uint32_t boxes_number = yl->boxes_number;
    float threshold = yl->threshold;
    box_t *boxes = (box_t *)yl->boxes;
    
    for (int i = 0; i < yl->boxes_number; ++i)
    {
        int class  = max_index(yl->probs[i], yl->classes);
        float prob = yl->probs[i][class];

        if (prob > threshold)
        {
            box_t *b = boxes + i;
            obj_info->obj[obj_number].x1 = roundf(b->x * image_width - (b->w * image_width / 2));
            obj_info->obj[obj_number].y1 = roundf(b->y * image_height - (b->h * image_height / 2));
            obj_info->obj[obj_number].x2 = roundf(b->x * image_width + (b->w * image_width / 2));
            obj_info->obj[obj_number].y2 = roundf(b->y * image_height + (b->h * image_height / 2));
            obj_info->obj[obj_number].class_id = class;
            obj_info->obj[obj_number].prob = prob;
            obj_number++;
        }
    }
    obj_info->obj_number = obj_number;
}

void yolo_layer_run(yolo_layer_t *yl, obj_info_t *obj_info)
{
    forward_yolo_layer(yl);
    get_yolo_boxes(yl, yl->output, yl->probs, yl->boxes);
    do_nms_sort(yl, yl->boxes, yl->probs);
}

void yolo_layer_draw_boxes(yolo_layer_t *yl, callback_draw_box callback)
{
    uint32_t image_width = yl->image_width;
    uint32_t image_height = yl->image_height;
    float threshold = yl->threshold;
    box_t *boxes = (box_t *)yl->boxes;

    for (int i = 0; i < yl->boxes_number; ++i)
    {
        int class  = max_index(yl->probs[i], yl->classes);
        float prob = yl->probs[i][class];

        if (prob > threshold)
        {
            box_t *b = boxes + i;
            uint32_t x1 = roundf(b->x * image_width - (b->w * image_width / 2));
            uint32_t y1 = roundf(b->y * image_height - (b->h * image_height / 2));
            uint32_t x2 = roundf(b->x * image_width + (b->w * image_width / 2));
            uint32_t y2 = roundf(b->y * image_height + (b->h * image_height / 2));
            callback(x1, y1, x2, y2, class, prob);
        }
    }
}
