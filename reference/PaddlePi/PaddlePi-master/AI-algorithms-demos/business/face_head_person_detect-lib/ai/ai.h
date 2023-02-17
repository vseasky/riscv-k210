#ifndef		__AI_H
#define		__AI_H

typedef	enum {
	FACE_KMODEL = 0,
	HEAD_KMODEL,
	BODY_KMODEL,
	MAX_KMODEL
} kmodel_t;

typedef	struct {
	uint64_t	lic1_h;
	uint64_t	lic1_l;
	uint64_t	lic2_h;
	uint64_t	lic2_l;
} license_t;


uint32_t	ai_load_model(kmodel_t type);
uint32_t	ai_run(kmodel_t type, obj_info_t *obj_info);
uint32_t	ai_threshold(kmodel_t type, float threshold, float nms);
uint32_t	ai_load_model_all(void);
uint32_t 	ai_init_license(license_t *lic, uint32_t lic_num);
uint32_t	ai_init_model_address(uint32_t face_kmodel_adr, uint32_t head_kmodel_adr, uint32_t body_kmodel_adr);
uint32_t 	ai_init_kpu_image_address(uint8_t *k_image_adress);

#endif

