#ifndef __AI_H
#define __AI_H
#include "image_process.h"
#include "region_layer.h"

typedef struct
{
    uint64_t lic1_h;
    uint64_t lic1_l;
    uint64_t lic2_h;
    uint64_t lic2_l;
} license_t;

typedef enum face_save_mode
{
    FACE_SUBSTITUTE = 0,
    FACE_APPEND,
    MAX_FACE_REGISTER_MODE
} face_save_mode_t;

typedef struct
{
    uint32_t width;
    uint32_t height;
    struct
    {
        uint32_t x;
        uint32_t y;
    } point[5];
} key_point_t;

extern float features_tmp[MAX_FOUND_FACES][192];
extern uint32_t alive_tmp[MAX_FOUND_FACES];
uint32_t ai_run_fd(obj_info_t *obj_info);
uint32_t ai_run_keypoint(image_t *ori_image, image_t *image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, key_point_t *key_point);
uint32_t ai_run_feature(image_t *image, uint32_t index);
uint32_t ai_run_alive(image_t *ori_image, image_t *image, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, key_point_t *key_point);
uint32_t ai_threshold(float threshold, float nms);
uint32_t ai_load_model_all();
uint32_t ai_init_license(license_t *lic, uint32_t lic_num);
uint32_t ai_init_model_address(uint32_t face_kmodel_adr, uint32_t keypoint_kmodel_addr, uint32_t feature_kmodel_addr, uint32_t fc_bin_addr, uint32_t alive_kmodel_addr);
uint32_t ai_init_kpu_image_address(uint8_t *k_image_adress);
uint32_t l2normalize(float *x, float *dx, int len);
float calCosinDistance(float *faceFeature0P, float *faceFeature1P, int featureLen);
uint32_t calulate_score(float *features, float *saved_features, uint32_t saved_len, float *score);
#endif
