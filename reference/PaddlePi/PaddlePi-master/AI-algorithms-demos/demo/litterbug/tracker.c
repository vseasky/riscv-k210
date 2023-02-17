#include <stdio.h>
#include <time.h>
#include "region_layer.c"

#define OBJECT (0)
#define HUMAN (1)
#define DIST_MAX (160000.)
#define DIST_THRESH (4096.)

#define STATE_NEW (0)
#define STATE_TRACKING (1)
#define STATE_UNTRACKED (2)

#define STATE_OWNED (10)
#define STATE_DISCARDED (11)
#define STATE_NOT_OWNED (12)
#define STATE_PICKED (13)

#define STATE_NORMAL (20)
#define STATE_HERO (21)
#define STATE_LITTERER (22)

#define LIFE_INIT (16)

//#define DELTA_T (0.5)
#define P_INIT (5)
#define P_P_NOISE (0.5)
#define P_L_NOISE (0.05)
#define P_V_NOISE (0.5)

typedef struct
{
    uint8_t is_valid;
    float cx;
    float cy;
    float w;
    float h;

    uint8_t id;
    uint8_t class;

    float prob_is_litterer;
    uint8_t track_state;
    uint8_t interaction_state;
    uint8_t belongs_to;
    uint8_t life;
    uint8_t updated_this_run;

    float vx;
    float vy;

    float pcx;
    float pcy;
    float pw;
    float ph;
    float pvx;
    float pvy;

    clock_t time_stamp;
} obj_t;

typedef struct
{
    obj_t *objects;
    uint16_t max_n_object;
    uint16_t n_object;
    uint16_t new_object_id;
} tracker_t;

static void init_object(obj_t *object)
{
    object->is_valid = 0;
}

static void init_tracker(tracker_t *tracker, obj_t *objects, uint16_t n_object)
{
    tracker->objects = objects;
    tracker->max_n_object = n_object;
    tracker->n_object = 0;
    tracker->new_object_id = 1;
    for (uint16_t i = 0; i < n_object; ++i)
        init_object(&objects[i]);
}

static uint8_t _add_object(tracker_t *tracker, float *measurement, uint8_t class, clock_t cur_time)
{
    uint8_t ok = 0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        if (!tracker->objects[i].is_valid)
        {
            obj_t *obj = &(tracker->objects[i]);
            obj->cx = *get_x(measurement);
            obj->cy = *get_y(measurement);
            obj->w = *get_w(measurement);
            obj->h = *get_h(measurement);
            obj->vx = 0.;
            obj->vy = 0.;
            obj->time_stamp = cur_time;

            obj->is_valid = 1;
            obj->id = tracker->new_object_id;
            obj->class = class;
            obj->track_state = STATE_NEW;
            obj->interaction_state = STATE_NORMAL;
            obj->belongs_to = 255; // actually max uint8
            obj->life = LIFE_INIT;
            obj->updated_this_run = 1;

            obj->pcx = P_INIT;
            obj->pcy = P_INIT;
            obj->pw = P_INIT;
            obj->ph = P_INIT;
            obj->pvx = P_INIT;
            obj->pvy = P_INIT;

            ok = 1;
            ++tracker->new_object_id;
            ++tracker->n_object;
            break;
        }
    }
    return ok;
}

static float _get_squared_distance(float xa, float ya, float xb, float yb)
{
    float dist_x = xb - xa;
    float dist_y = yb - ya;
    dist_x *= dist_x;
    dist_y *= dist_y;
    float squared_dist = dist_x + dist_y;
    return squared_dist;
}

static uint8_t _get_closest_given_measurement(tracker_t *tracker,
                                              float *measurement,
                                              uint8_t target_class,
                                              float dist_thresh,
                                              uint8_t *min_index,
                                              clock_t cur_time)
{
    clock_t elapsed = clock() - cur_time;
    float t_diff = elapsed;
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->class != target_class || obj->updated_this_run)
            continue;

        float predicted_cx = obj->cx + t_diff * obj->vx;
        float predicted_cy = obj->cy + t_diff * obj->vy;
        float squared_dist = _get_squared_distance(predicted_cx, predicted_cy,
                                                   *get_x(measurement), *get_y(measurement));

        if (squared_dist < dist_thresh && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    return found;
}

static uint8_t _get_closest_given_object(tracker_t *tracker, obj_t *obj, uint8_t target_class, float dist_thresh, uint8_t *min_index, float *confidence)
{
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    float dist_sum = 0.0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *target_obj = &(tracker->objects[i]);
        if (!target_obj->is_valid || target_obj->class != target_class)
            continue;

        float squared_dist = _get_squared_distance(target_obj->cx, target_obj->cy, obj->cx, obj->cy);
        dist_sum += squared_dist;

        if (squared_dist < dist_thresh && squared_dist < min_dist)//target_obj->w * target_obj->h && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    // the confidence
    if(found && confidence){
        if(min_dist == dist_sum)
            *confidence = 1.;
        else
            *confidence = 1. - min_dist / dist_sum;
    }
    return found;
}

static void _predict(obj_t *obj, float t_diff)
{
    obj->cx += obj->vx * t_diff;
    obj->cy += obj->vy * t_diff;

    obj->pcx += P_P_NOISE;
    obj->pcy += P_P_NOISE;
    obj->ph += P_L_NOISE;
    obj->pw += P_L_NOISE;
    obj->pvx += P_V_NOISE;
    obj->pvy += P_V_NOISE;
}

static void _update_object(obj_t *obj, float *measurement, clock_t cur_time)
{
    static clock_t elapsed;
    static float t_diff;
    static float conf;
    static float x_mea;
    static float y_mea;
    static float w_mea;
    static float h_mea;
    static float vx_mea;
    static float vy_mea;
    static float r;
    static float x_kg;
    static float y_kg;
    static float w_kg;
    static float h_kg;
    static float vx_kg;
    static float vy_kg;

    elapsed = cur_time - obj->time_stamp;
    t_diff = elapsed;
    // 3
    conf = sigmoid(*measurement - 3); // NOTE
    x_mea = *get_x(measurement);
    y_mea = *get_y(measurement);
    w_mea = *get_w(measurement);
    h_mea = *get_h(measurement);
    vx_mea = (x_mea - obj->cx) / t_diff;
    vy_mea = (y_mea - obj->cy) / t_diff;

    // predict moved here
    _predict(obj, t_diff);

    // 4
    r = 1. - conf;
    x_kg = obj->pcx / (obj->pcx + r);
    y_kg = obj->pcy / (obj->pcy + r);
    w_kg = obj->pw / (obj->pw + r);
    h_kg = obj->ph / (obj->ph + r);
    vx_kg = obj->pvx / (obj->pvx + r);
    vy_kg = obj->pvy / (obj->pvy + r);

    // 5
    obj->cx += x_kg * (x_mea - obj->cx);
    obj->cy += y_kg * (y_mea - obj->cy);
    obj->w += w_kg * (w_mea - obj->w);
    obj->h += h_kg * (h_mea - obj->h);
    obj->vx += vx_kg * (vx_mea - obj->vx);
    obj->vy += vy_kg * (vy_mea - obj->vy);

    // 6
    obj->pcx *= (1. - x_kg);
    obj->pcy *= (1. - y_kg);
    obj->pw *= (1. - w_kg);
    obj->ph *= (1. - h_kg);
    obj->pvx *= (1. - vx_kg);
    obj->pvy *= (1. - vy_kg);

    // update
    obj->time_stamp = cur_time;
    obj->life = LIFE_INIT;
    obj->updated_this_run = 1;
}

static void _add_or_update_closest(tracker_t *tracker,
                                   float *measurement,
                                   uint8_t i_class,
                                   float dist_thresh,
                                   clock_t cur_time)
{
    uint8_t obj_index = 0;
    uint8_t found = _get_closest_given_measurement(tracker, measurement, i_class, dist_thresh, &obj_index, cur_time);
    if (!found)
        _add_object(tracker, measurement, i_class, cur_time); // may check ok or not
    else
    {
        _update_object(&(tracker->objects[obj_index]), measurement, cur_time);
    }
}

static void _obj_state_normal_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    uint16_t belongs_to_index = 0;
    float confidence;
    uint8_t found = _get_closest_given_object(tracker, obj, HUMAN, dist_thresh, &belongs_to_index, &confidence);
    if (found)
    {
        obj->belongs_to = belongs_to_index;
        obj->interaction_state = STATE_OWNED;
        // add confidence
        tracker->objects[belongs_to_index].prob_is_litterer = confidence;
    }
    else
    {
        obj->interaction_state = STATE_NOT_OWNED;
    }
}
static void _obj_state_owned_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    obj_t *owner = &(tracker->objects[obj->belongs_to]);
    float squared_dist = _get_squared_distance(owner->cx, owner->cy, obj->cx, obj->cy);
    if (squared_dist >= DIST_THRESH) // owner->w * owner->h)
    {
        owner->interaction_state = STATE_LITTERER;
        obj->interaction_state = STATE_DISCARDED;
    }
}
static void _obj_state_discarded_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    obj_t *owner = &(tracker->objects[obj->belongs_to]);
    float squared_dist = _get_squared_distance(owner->cx, owner->cy, obj->cx, obj->cy);
    if (squared_dist < DIST_THRESH - 100) //owner->w * owner->h)
    {
        if (owner->interaction_state == STATE_LITTERER)
        {
            owner->interaction_state = STATE_NORMAL;
        }
    }
}

/*
static void _obj_state_not_owned_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    uint16_t belongs_to_index = 0;
    uint8_t found = _get_closest_given_object(tracker, obj, HUMAN, dist_thresh, &belongs_to_index);
    if (found)
    {
        obj->belongs_to = belongs_to_index;
        obj->interaction_state = STATE_PICKED;
    }
}

static void _obj_state_picked_handler(tracker_t *tracker, float dist_thresh, obj_t *obj)
{
    obj_t *picker = &(tracker->objects[obj->belongs_to]);
    float squared_dist = _get_squared_distance(picker->cx, picker->cy, obj->cx, obj->cy);
    if (squared_dist >= picker->w * picker->h)
    {
        picker->interaction_state = STATE_LITTERER;
    }
}
*/

static void _obj_state_handler(tracker_t *tracker, float dist_thresh)
{
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->class == HUMAN)
            continue;

        if (obj->interaction_state == STATE_NORMAL)
        {
            _obj_state_normal_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_OWNED)
        {
            _obj_state_owned_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_DISCARDED)
        {
            _obj_state_discarded_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_NOT_OWNED)
        {
            //_obj_state_not_owned_handler(tracker, dist_thresh, obj);
        }
        else if (obj->interaction_state == STATE_PICKED)
        {
            //_obj_state_picked_handler(tracker, dist_thresh, obj);
        }
        else
            ;
    }
}

static void _handle_post_update(tracker_t *tracker)
{
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid)
            continue;

        if (--obj->life == 0){
            obj->is_valid = 0;
            --tracker->n_object;
        }

        obj->updated_this_run = 0;
    }
}

static void _handle_update(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t class,
                           float squared_dist_thresh,
                           clock_t cur_time)
{
    for (uint16_t i = 0; i < n_measurement; ++i)
    {
        _add_or_update_closest(tracker, measurements[i], class, squared_dist_thresh, cur_time);
    }
}

static void update_tracker(tracker_t *tracker,
                           float *measurements[],
                           uint16_t n_measurement,
                           uint8_t class,
                           float squared_dist_thresh)
{
    clock_t cur_time = clock();
    _handle_update(tracker, measurements, n_measurement, class, squared_dist_thresh, cur_time);
    //_obj_state_handler(tracker, squared_dist_thresh);
    //_handle_post_update(tracker);
}

/*
int test_tracker()
{
    float anchors[9][4] = {
        {7.500000, 7.500000, 54.900000, 28.500000},
        {7.500000, 7.500000, 110.100000, 57.300000},
        {7.500000, 7.500000, 220.500000, 114.900000},
        {7.500000, 7.500000, 38.100000, 38.100000},
        {7.500000, 7.500000, 76.500000, 76.500000},
        {7.500000, 7.500000, 153.300000, 153.300000},
        {7.500000, 7.500000, 26.100000, 52.500000},
        {7.500000, 7.500000, 52.500000, 105.300000},
        {7.500000, 7.500000, 105.300000, 210.900000}};

    uint16_t n_box = 5;
    float *boxes[n_box];
    float vector[45][15][20] = {0};
    vector[0][0][0] = 5;
    vector[1][0][0] = 3;
    vector[2][0][0] = 3;
    vector[3][0][0] = 4;
    vector[4][0][0] = 4;

    vector[0][1][1] = 4;
    vector[1][1][1] = 3;
    vector[2][1][1] = 3;
    vector[3][1][1] = 4;
    vector[4][1][1] = 4;

    vector[0][9][9] = 4;
    vector[1][9][9] = 3;
    vector[2][9][9] = 3;
    vector[3][9][9] = 4;
    vector[4][9][9] = 4;

    uint16_t n_result = get_boxes(boxes, n_box, vector, 15, 20, anchors, 9, 0.5, 0.5);

    uint16_t n_obj = 10;
    obj_t objects[10];
    tracker_t tracker;
    init_tracker(&tracker, objects, n_obj);

    uint16_t pick = 2;
    for (uint16_t _ = 0; _ < 20; ++_)
    {

        update_tracker(&tracker, boxes, pick, 1, 2048);
        pick = pick == 2 ? 1 : 2;

        uint16_t n_valid = 0;
        for (uint16_t i = 0; i < n_obj; ++i)
            if (objects[i].is_valid == 1)
                ++n_valid;
        if (n_valid != 2)
        {
            printf("test_tracker failed, n_valid = %ud\n", n_valid);
            return -1;
        }

        for (uint16_t i = 0; i < n_result; ++i)
        {
            *get_x(boxes[i]) += rand() % 15;
            *get_y(boxes[i]) += rand() % 15;
            *get_w(boxes[i]) += 0.5;
            *get_h(boxes[i]) += 0.5;
            printf("m: %f %f %f %f %f\n", *boxes[i], *get_x(boxes[i]), *get_y(boxes[i]), *get_w(boxes[i]), *get_h(boxes[i]));
        }
    }

    printf("test_tracker OK\n");
    return 0;
}
int main()
{
    return test_tracker();
}
*/
/* 8 */
static uint8_t _add_object_8(tracker_t *tracker, float *measurement, uint8_t class, clock_t cur_time)
{
    uint8_t ok = 0;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        if (!tracker->objects[i].is_valid)
        {
            obj_t *obj = &(tracker->objects[i]);
            obj->cx = *get_x_8(measurement);
            obj->cy = *get_y_8(measurement);
            obj->w = *get_w_8(measurement);
            obj->h = *get_h_8(measurement);
            obj->vx = 0.;
            obj->vy = 0.;
            obj->time_stamp = cur_time;

            obj->is_valid = 1;
            obj->id = tracker->new_object_id;
            obj->class = class;
            obj->track_state = STATE_NEW;
            obj->interaction_state = STATE_NORMAL;
            obj->belongs_to = 255; // actually max uint8
            obj->life = LIFE_INIT;
            obj->updated_this_run = 1;

            obj->pcx = P_INIT;
            obj->pcy = P_INIT;
            obj->pw = P_INIT;
            obj->ph = P_INIT;
            obj->pvx = P_INIT;
            obj->pvy = P_INIT;

            ok = 1;
            if(++tracker->new_object_id == 255)
                tracker->new_object_id = 1;
            ++tracker->n_object;
            break;
        }
    }
    return ok;
}

/*
static float _get_squared_distance(float xa, float ya, float xb, float yb)
{
    float dist_x = xb - xa;
    float dist_y = yb - ya;
    dist_x *= dist_x;
    dist_y *= dist_y;
    float squared_dist = dist_x + dist_y;
    return squared_dist;
}
*/

static uint8_t _get_closest_given_measurement_8(tracker_t *tracker,
                                                float *measurement,
                                                uint8_t target_class,
                                                float dist_thresh,
                                                uint8_t *min_index,
                                                clock_t cur_time)
{
    clock_t elapsed = clock() - cur_time;
    float t_diff = elapsed;
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!obj->is_valid || obj->class != target_class || obj->updated_this_run)
            continue;

        float predicted_cx = obj->cx + t_diff * obj->vx;
        float predicted_cy = obj->cy + t_diff * obj->vy;
        float squared_dist = _get_squared_distance(predicted_cx, predicted_cy,
                                                   *get_x_8(measurement), *get_y_8(measurement));

        if (squared_dist < dist_thresh && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    return found;
}

/*
static uint8_t _get_closest_given_object(tracker_t *tracker, obj_t *obj, uint8_t target_class, float dist_thresh, uint8_t *min_index)
{
    uint8_t found = 0;
    float min_dist = DIST_MAX;
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *target_obj = &(tracker->objects[i]);
        if (!target_obj->is_valid || target_obj->class != target_class)
            continue;

        float squared_dist = _get_squared_distance(target_obj->cx, target_obj->cy, obj->cx, obj->cy);

        if (squared_dist < target_obj->w * target_obj->h && squared_dist < min_dist)
        {
            min_dist = squared_dist;
            *min_index = i;
            found = 1;
        }
    }
    return found;
}

static void _predict(obj_t *obj, float t_diff)
{
    obj->cx += obj->vx * t_diff;
    obj->cy += obj->vy * t_diff;

    obj->pcx += P_P_NOISE;
    obj->pcy += P_P_NOISE;
    obj->ph += P_L_NOISE;
    obj->pw += P_L_NOISE;
    obj->pvx += P_V_NOISE;
    obj->pvy += P_V_NOISE;
}
*/

static void _update_object_8(obj_t *obj, float *measurement, clock_t cur_time)
{
    static clock_t elapsed;
    static float t_diff;
    static float conf;
    static float x_mea;
    static float y_mea;
    static float w_mea;
    static float h_mea;
    static float vx_mea;
    static float vy_mea;
    static float r;
    static float x_kg;
    static float y_kg;
    static float w_kg;
    static float h_kg;
    static float vx_kg;
    static float vy_kg;

    elapsed = cur_time - obj->time_stamp;
    t_diff = elapsed;
    // 3
    conf = sigmoid(*measurement - 3); // NOTE
    x_mea = *get_x_8(measurement);
    y_mea = *get_y_8(measurement);
    w_mea = *get_w_8(measurement);
    h_mea = *get_h_8(measurement);
    vx_mea = (x_mea - obj->cx) / t_diff;
    vy_mea = (y_mea - obj->cy) / t_diff;

    // predict moved here
    _predict(obj, t_diff);

    // 4
    r = 1. - conf;
    x_kg = obj->pcx / (obj->pcx + r);
    y_kg = obj->pcy / (obj->pcy + r);
    w_kg = obj->pw / (obj->pw + r);
    h_kg = obj->ph / (obj->ph + r);
    vx_kg = obj->pvx / (obj->pvx + r);
    vy_kg = obj->pvy / (obj->pvy + r);

    // 5
    obj->cx += x_kg * (x_mea - obj->cx);
    obj->cy += y_kg * (y_mea - obj->cy);
    obj->w += w_kg * (w_mea - obj->w);
    obj->h += h_kg * (h_mea - obj->h);
    obj->vx += vx_kg * (vx_mea - obj->vx);
    obj->vy += vy_kg * (vy_mea - obj->vy);

    // 6
    obj->pcx *= (1. - x_kg);
    obj->pcy *= (1. - y_kg);
    obj->pw *= (1. - w_kg);
    obj->ph *= (1. - h_kg);
    obj->pvx *= (1. - vx_kg);
    obj->pvy *= (1. - vy_kg);

    // update
    obj->time_stamp = cur_time;
    obj->life = LIFE_INIT;
    obj->updated_this_run = 1;
}

static void _add_or_update_closest_8(tracker_t *tracker,
                                     float *measurement,
                                     uint8_t i_class,
                                     float dist_thresh,
                                     clock_t cur_time)
{
    uint8_t obj_index = 0;
    uint8_t found = _get_closest_given_measurement_8(tracker, measurement, i_class, dist_thresh, &obj_index, cur_time);
    if (!found)
        _add_object_8(tracker, measurement, i_class, cur_time); // may check ok or not
    else
    {
        _update_object_8(&(tracker->objects[obj_index]), measurement, cur_time);
    }
}

static void _handle_update_8(tracker_t *tracker,
                             float *measurements[],
                             uint16_t n_measurement,
                             uint8_t class,
                             float squared_dist_thresh,
                             clock_t cur_time)
{
    for (uint16_t i = 0; i < n_measurement; ++i)
    {
        _add_or_update_closest_8(tracker, measurements[i], class, squared_dist_thresh, cur_time);
    }
}

static void update_tracker_8(tracker_t *tracker,
                             float *measurements[],
                             uint16_t n_measurement,
                             uint8_t class,
                             float squared_dist_thresh)
{
    clock_t cur_time = clock();
    _handle_update_8(tracker, measurements, n_measurement, class, squared_dist_thresh, cur_time);
    _obj_state_handler(tracker, squared_dist_thresh);
    _handle_post_update(tracker);
}
