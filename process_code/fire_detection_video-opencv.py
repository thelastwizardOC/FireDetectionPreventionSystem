__author__ = 'dell'
# -*- coding: utf-8 -*-
import cv2
import time
import numpy as np
import serial
#from PIL import Image, ImageEnhance


config = "yolov4-tiny_fire_person.cfg"
weights = "yolov4-tiny_fire_person_best.weights"
names = "fire_person.names"

def adjust_image_gamma_lookuptable(image, gamma=1.0):
    # build a lookup table mapping the pixel values [0, 255] to
    # their adjusted gamma values
    table = np.array([((i / 255.0) ** gamma) * 255
        for i in np.arange(0, 256)]).astype("uint8")
    # apply gamma correction using the lookup table
    return cv2.LUT(image, table)

def change_brightness(img, alpha, beta):
    img_new = np.asarray(alpha*img + beta, dtype=int)   # cast pixel values to int
    img_new[img_new>255] = 255
    img_new[img_new<0] = 0
    return Image.fromarray(img_new)

def pixel_transform(image, alpha = 1.0, beta = 0):
    out[pixel] = alpha * image[pixel] + beta
    output = np.zeros(image.shape, image.dtype)
    h, w, ch = image.shape
    for y in range(h):
        for x in range(w):
            for c in range(ch):
                output[y,x,c] = np.clip(alpha*image[y,x,c] + beta, 0, 255)

    return output
def get_output_layers(net):
    layer_names = net.getLayerNames()
    # output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]
    output_layers = [layer_names[i[0] - 1] for i in net.getUnconnectedOutLayers()]
    return output_layers


def draw_prediction(img, class_id, confidence, x, y, x_plus_w, y_plus_h):
    label = str(classes[class_id])
    #color = COLORS[class_id]
    cv2.rectangle(img, (x, y), (x_plus_w, y_plus_h), (0,255,0), 2)
    cv2.putText(img, label + str(confidence), (x - 10, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 2)



cap = cv2.VideoCapture(1)  
fps = 0
prev_frame_time = time.time()
ser = serial.Serial(port = 'COM7', baudrate = 115200,parity = serial.PARITY_NONE,stopbits = serial.STOPBITS_ONE,bytesize = serial.EIGHTBITS,timeout = 0.06)
while(cap.isOpened()):
    ret, frame = cap.read()
    width = frame.shape[1]
    height = frame.shape[0]
    #center_image_x= width/2
    new_frame_time = time.time()
    img = cv2.resize(frame, (640,480)) #(int(width*0.2), int(height*0.2)
    #img1= adjust_image_gamma_lookuptable(frame, 3)
    #img = cv2.medianBlur(img1,5)
    scale = 0.00392
    classes = None
    with open(names, 'r') as f:
        classes = [line.strip() for line in f.readlines()]
    #COLORS = np.random.uniform(0, 255, size=(len(classes), 3))

    net = cv2.dnn.readNet(weights, config)

    blob = cv2.dnn.blobFromImage(img, scale, (416, 416), (0, 0, 0), True, crop=False)

    net.setInput(blob)

    outs = net.forward(get_output_layers(net))

    class_ids = []
    confidences = []
    boxes = []
    conf_threshold = 0.5
    nms_threshold = 0.4

    start = time.time()
    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]

            if confidence>0.5:
                center_x = int(detection[0] * width)
                center_y = int(detection[1] * height)
                w = int(detection[2] * width)
                h = int(detection[3] * height)
                x = center_x - w / 2
                y = center_y - h / 2
                class_ids.append(class_id)
                confidences.append(float(confidence))
                boxes.append([x, y, w, h])
    indices = cv2.dnn.NMSBoxes(boxes, confidences, conf_threshold, nms_threshold)
    for i in indices:
        i = i[0]
        box = boxes[i]
        x = box[0]
        y = box[1]
        w = box[2]
        h = box[3]
        
        if (class_ids[i] == 0):
            if (0 < (x + w/2) < (width/2 - 16)):
                ser.write(b'-1')
                print('-1')
                time.sleep(1.1)
            elif ( (width/2 + 16) < (x + w/2) < width ):
                ser.write(b'1')
                print('1')
                time.sleep(1.1)
                
            else: 
                if (0 < (y + h/2) < (height/2 - 8)):
                    ser.write(b'0:-1')
                    print('0:-1')
                    time.sleep(1.1)
                elif ((height/2 + 8) < (y + h/2) < height ):
                    ser.write(b'0:1')
                    print('0:1')
                    time.sleep(1.1)
                else:
                    ser.write(b'0:0')
                    print('0:0')
                    time.sleep(1.1)
            

        draw_prediction(img, class_ids[i], confidences[i], round(x), round(y), round(x + w), round(y + h))
    new_frame_time = time.time()
    curr_fps = 1 / (new_frame_time - prev_frame_time)
    fps = curr_fps if (fps == 0) else (fps*0.95 + curr_fps*0.05)
    prev_frame_time = new_frame_time
    cv2.imshow('frame',img)
    print(fps)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
cap.release()
cv2.destroyAllWindows()