import os
import cv2
import numpy as np

def apply_mask(image, label):
    mask_color = (255, 0, 0)
    alpha = 0.5
    mask = np.zeros_like(image)
    mask[:,:, 0] = label * (mask_color[0] / 255)
    mask[:,:, 1] = label * (mask_color[1] / 255)
    mask[:,:, 2] = label * (mask_color[2] / 255)

    mask_normalized = label / 255.0

    result = image.copy()

    for c in range(3):
        result[:, :, c] = (1 - mask_normalized * alpha) * image[:, :, c] + mask_normalized * alpha * mask[:, :, c]

    result = cv2.cvtColor(result, cv2.COLOR_BGR2RGB)
    
    return result

def cont(image, label):
    _, thresh_label = cv2.threshold(label, 200, 255, cv2.THRESH_BINARY) 
    contours, hierarchy = cv2.findContours(thresh_label, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    result = image.copy()

    for i in range(len(contours)):
        # if cv2.contourArea(contours[i]) > 100:
        result = cv2.drawContours(result, contours, i, (0, 255, 0), 3)
    

    result = cv2.cvtColor(result, cv2.COLOR_BGR2RGB)
    
    return result

def task_1():
    images_folder_path = "archive/images/"
    labels_folder_path = "archive/labels/"

    images = os.listdir(images_folder_path)

    for name in images: 
        image = cv2.imread(images_folder_path + name, cv2.IMREAD_COLOR_RGB)
        label = cv2.imread(labels_folder_path + name, cv2.IMREAD_GRAYSCALE)

        result = apply_mask(image, label)
        cnt = cont(image, label)


        cv2.imshow("contours", cnt)
        cv2.imshow("result", result)

        if(cv2.waitKey(0) == 27):
            break

def task_4():
    cap = cv2.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        cv2.imshow("video", frame)
        
        if cv2.waitKey(10) == 27:
            break

    cap.release()
    cv2.destroyAllWindows()

def main():
    task_1()
    task_4()

main()