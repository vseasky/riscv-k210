import sensor, image, lcd, time
import KPU as kpu

color_R = (255, 0, 0)
color_G = (0, 255, 0)
color_B = (0, 0, 255)

class_IDs = ['no_mask', 'mask']

def drawConfidenceText(image, rol, classid, value):
    text = ""
    _confidence = int(value * 100)

    if classid == 1:
        text = 'mask: ' + str(_confidence) + '%'
        color_text=color_G
    else:
        text = 'no_mask: ' + str(_confidence) + '%'
        color_text=color_R
    image.draw_string(rol[0], rol[1], text, color=color_text, scale=1.0)



lcd.init(freq=20000000,invert=True)
sensor.reset(dual_buff=True)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_hmirror(1)
sensor.set_vflip(1)
sensor.run(1)

task = kpu.load(0x300000)

anchor = (0.1606, 0.3562, 0.4712, 0.9568, 0.9877, 1.9108, 1.8761, 3.5310, 3.4423, 5.6823)
kpu.init_yolo2(task, 0.5, 0.3, 5, anchor)
img_lcd = image.Image()

clock = time.clock()
while (True):
    clock.tick()
    img = sensor.snapshot()
    code = kpu.run_yolo2(task, img)
    if code:
        totalRes = len(code)
        for item in code:
            confidence = float(item.value())
            itemROL = item.rect()
            classID = int(item.classid())
            if confidence < 0.52:
                img.draw_rectangle(itemROL, color=color_B, tickness=5)
                continue
            if classID == 1 and confidence > 0.65:
                img.draw_rectangle(itemROL, color_G, tickness=5)
                if totalRes == 1:
                    drawConfidenceText(img, (item.x(), item.y()-12), 1, confidence)
            else:
                img.draw_rectangle(itemROL, color=color_R, tickness=5)
                if totalRes == 1:
                    drawConfidenceText(img, (item.x(), item.y()-12), 0, confidence)

    lcd.display(img)

    print(clock.fps())

kpu.deinit(task)
