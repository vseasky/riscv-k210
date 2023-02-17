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
#ifndef _KEY_POINT_H
#define _KEY_POINT_H

#include <stdint.h>
#include "kpu.h"

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

void key_point_last_handle(kpu_task_t *task, key_point_t *key_point);

#endif /* _KEY_POINT_H */
