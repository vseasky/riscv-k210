#ifndef _SYSTEM_CONFIG_H
#define _SYSTEM_CONFIG_H

//#define  IMAGE_SAVED_WIDTH                          128//160
//#define  IMAGE_SAVED_HEIGHT                         128//160
//#define  FACE_IMAGE_SIZE                            (IMAGE_SAVED_WIDTH * IMAGE_SAVED_HEIGHT * 2)

#define DATA_ADDRESS                                (0x600000 - 256 * 1024)
#define DATA_REBOOT_COUNT                           (0x600000 - 64 * 1024)
#define FACE_DATA_ADDERSS                           (0x600000)
#define FACE_HEADER                                 0x55AA5502
#define FACE_DATA_MAX_COUNT                         256
#define FACE_RECGONITION_SCORE                      (80.0)

#define WATCH_DOG_TIMEOUT                           (20000) //ms
#define WDT_GDB_DEBUG                               0
#define USE_SDCARD                                  1

#endif
