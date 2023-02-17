#include "flash.h"
#include "w25qxx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "sysctl.h"
#include <string.h>

volatile face_save_info_t g_face_save_info;
face_info_t g_face_info;

static int get_face_id(void)
{
    int ret = -1;
    int i;
    for(i = 0; i < FACE_DATA_MAX_COUNT; i++)
    {
        if(((g_face_save_info.face_info_index[i / 32] >> (i % 32)) & 0x1) == 0)
        {
            ret = i;
            break;
        }
    }

    if(i >= FACE_DATA_MAX_COUNT)
    {
        printf("get_face_id:> Err too many data\n");
        return -1;
    }
    return ret;
}

int flash_delete_face_info(uint32_t id)
{
    if(g_face_save_info.number == 0)
    {
        printf("del pic, no pic\n");
        return -1;
    }
    g_face_save_info.face_info_index[id / 32] &= ~(1 << (id % 32));
    g_face_save_info.number--;
    w25qxx_write_data(DATA_ADDRESS, (uint8_t *)&g_face_save_info, sizeof(face_save_info_t));
    return 0;
}

int flash_delete_face_all(void)
{
    g_face_save_info.number = 0;
    memset(g_face_save_info.face_info_index, 0, sizeof(g_face_save_info.face_info_index));
    w25qxx_write_data(DATA_ADDRESS, (uint8_t *)&g_face_save_info, sizeof(face_save_info_t));
    return 0;
}

int flash_get_face_img(uint8_t *image, uint32_t id)
{
//    uint32_t image_address = FACE_DATA_ADDERSS + id * (sizeof(face_info_t) + FACE_IMAGE_SIZE) + sizeof(face_info_t);

//    w25qxx_read_data(image_address, image, FACE_IMAGE_SIZE);
//    return 0;
}

int flash_get_face_info(face_info_t *face_info, uint32_t id)
{
//    uint32_t image_address = FACE_DATA_ADDERSS + id * (sizeof(face_info_t) + FACE_IMAGE_SIZE);
    uint32_t image_address = FACE_DATA_ADDERSS + id * (sizeof(face_info_t));
    w25qxx_read_data(image_address, face_info, sizeof(face_info_t));
}

int flash_save_face_info(uint8_t *image, float *features)
{
    int face_id = get_face_id();
    if(face_id >= FACE_DATA_MAX_COUNT || face_id < 0)
    {
        printf("get_face_id err\n");
        return -1;
    }
    printf("Save face_id is %d\n", face_id);
    memcpy(g_face_info.info, features, sizeof(g_face_info.info));
    w25qxx_write_data(FACE_DATA_ADDERSS + face_id * (sizeof(face_info_t)), &g_face_info, sizeof(face_info_t));
//    w25qxx_write_data(FACE_DATA_ADDERSS + face_id * (sizeof(face_info_t) + FACE_IMAGE_SIZE), &g_face_info, sizeof(face_info_t));
//    w25qxx_write_data(FACE_DATA_ADDERSS + face_id * (sizeof(face_info_t) + FACE_IMAGE_SIZE) + sizeof(face_info_t), image, FACE_IMAGE_SIZE);
    g_face_save_info.number++;
    g_face_save_info.face_info_index[face_id / 32] |= (1<<(face_id % 32));
    w25qxx_write_data(DATA_ADDRESS, (uint8_t *)&g_face_save_info, sizeof(face_save_info_t));

    return 0;
}

uint32_t flash_get_wdt_reboot_count(void)
{
    uint32_t v_wdt_reboot_count;
    w25qxx_read_data(DATA_REBOOT_COUNT, &v_wdt_reboot_count, 4);
    return v_wdt_reboot_count;
}

static void flash_check_wdt_reboot_count(void)
{
    uint32_t v_wdt_reboot_count = flash_get_wdt_reboot_count();
    sysctl_reset_enum_status_t v_reset_status = sysctl_get_reset_status();
    if (v_reset_status == SYSCTL_RESET_STATUS_WDT0 || v_reset_status == SYSCTL_RESET_STATUS_WDT1 )
    {
        printf("wdt reboot!\n");
        v_wdt_reboot_count++;
        w25qxx_write_data(DATA_REBOOT_COUNT, (uint8_t *)&v_wdt_reboot_count, 4);
    }
    else if(v_wdt_reboot_count!= 0)
    {
        v_wdt_reboot_count = 0;
        w25qxx_write_data(DATA_REBOOT_COUNT, (uint8_t *)&v_wdt_reboot_count, 4);
    }
}

void flash_init(void)
{
    /* spi flash init */
    w25qxx_init(3, 0, 60000000);

    uint8_t manuf_id, device_id;
    w25qxx_read_id(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if ((manuf_id != 0xEF && manuf_id != 0xC8) || (device_id != 0x17 && device_id != 0x16))
    {
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return ;
    }

    w25qxx_read_data(DATA_ADDRESS, (uint8_t *)&g_face_save_info, sizeof(face_save_info_t));

    if(g_face_save_info.header == FACE_HEADER)
    {
        printf("The header ok\n");
        uint32_t v_num = g_face_save_info.number;
        printf("there is %d img\n", v_num);
        v_num = 0;

        for(uint32_t i = 0; i < FACE_DATA_MAX_COUNT; i++)
        {
            if(g_face_save_info.face_info_index[i / 32] >> (i % 32) & 0x1)
            {
                v_num++;
            }
        }

        if(v_num != g_face_save_info.number)
        {
            printf("err:> index is %d, but saved is %d\n", v_num, g_face_save_info.number);
            g_face_save_info.number = v_num;
        }

        if(v_num >= FACE_DATA_MAX_COUNT)
        {
            printf("ERR, too many pic\n");
        }
    }
    else
    {
        printf("No header\n");
        g_face_save_info.header = FACE_HEADER;
        g_face_save_info.number = 0;
        memset(g_face_save_info.face_info_index, 0, sizeof(g_face_save_info.face_info_index));
    }
    flash_check_wdt_reboot_count();

}

float calCosinDistance(float *faceFeature0P, float *faceFeature1P, int featureLen)
{
    // definitiion
    float coorFeature = 0;
    float sumFeature0 = 0;
    float sumFeature1 = 0;

    // calculate the sum square
    for (int fIdx = 0; fIdx < featureLen; fIdx++)
    {
        float featureVal0 = *(faceFeature0P + fIdx);
        float featureVal1 = *(faceFeature1P + fIdx);

        sumFeature0 += featureVal0 * featureVal0;
        sumFeature1 += featureVal1 * featureVal1;
        coorFeature += featureVal0 * featureVal1;
    }

    // cosin distance
    return (0.5 + 0.5 * coorFeature / sqrt(sumFeature0 * sumFeature1))*100;
}


int calulate_score(float *features, float *score)
{
    int i;
    int v_id = -1;
    face_info_t v_face_info;
    float v_score;
    float v_score_max = 0.0;
    for(i = 0; i < FACE_DATA_MAX_COUNT; i++)
    {
        if((g_face_save_info.face_info_index[i / 32] >> (i % 32)) & 0x1)
        {
            flash_get_face_info(&v_face_info, i);
            v_score = calCosinDistance(features, v_face_info.info, 196);
            if(v_score > v_score_max)
            {
                v_score_max = v_score;
                v_id = i;
            }
        }
    }
    *score = v_score_max;
    return v_id;
}

