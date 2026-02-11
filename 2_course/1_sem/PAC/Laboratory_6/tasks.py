import os
import cv2
import numpy as np
import random

class DeGenerator:
    def __init__(self, image_dir, mask_dir, batch_size=4, res_size=(400, 400)):
        self.image_dir = image_dir
        self.mask_dir = mask_dir
        self.batch_size = batch_size
        self.res_size = res_size
        
        
        self.image_files = [os.path.join(image_dir, f) for f in os.listdir(image_dir) if f.endswith('.jpg')]
        self.mask_files = [os.path.join(mask_dir, f) for f in os.listdir(mask_dir) if f.endswith('.jpg')]
        
        if len(self.image_files) != len(self.mask_files):
            raise ValueError("Количество изображений и масок не совпадает")
        
        self.indices = list(range(len(self.image_files)))
        self.on_epoch_end()

    def on_epoch_end(self):
        
        random.shuffle(self.indices)

    def apply_augmentation(self, image, mask):
        if np.random.random() > 0.5:
            # 1. Поворот на случайный угол
            angle = np.random.uniform(-30, 30)
            M = cv2.getRotationMatrix2D((image.shape[1] // 2, image.shape[0] // 2), angle, 1.0)
            image = cv2.warpAffine(image, M, (image.shape[1], image.shape[0]))
            mask = cv2.warpAffine(mask, M, (mask.shape[1], mask.shape[0]), flags=cv2.INTER_NEAREST)

        if np.random.random() > 0.5:
            # 2. Отражение
            if np.random.random() > 0.5:
                image = cv2.flip(image, 1)  
                mask = cv2.flip(mask, 1)
            if np.random.random() > 0.5:
                image = cv2.flip(image, 0)  
                mask = cv2.flip(mask, 0)
        
        if np.random.random() > 0.5:
            # 3. Вырезание куска
            h, w = image.shape[:2]
            crop_h = int(h * np.random.uniform(0.7, 1.0))
            crop_w = int(w * np.random.uniform(0.7, 1.0))
            top = np.random.randint(0, h - crop_h)
            left = np.random.randint(0, w - crop_w)
            image = image[top:top + crop_h, left:left + crop_w]
            mask = mask[top:top + crop_h, left:left + crop_w]
        
        if np.random.random() > 0.5:
            # 4. Хентай
            image = cv2.GaussianBlur(image, (3, 3), 0)  

        
        image = cv2.resize(image, self.res_size)
        mask = cv2.resize(mask, self.res_size, interpolation=cv2.INTER_NEAREST)

        return image, mask

    def apply_mask(self, image, mask):
        
        mask_color = (0, 0, 255)  
        alpha = 0.5  
        mask_3ch = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)  
        mask_normalized = mask.astype(float) / 255.0  

        
        for c in range(0, 3):
            image[:, :, c] = (1 - mask_normalized * alpha) * image[:, :, c] + \
                           (mask_normalized * alpha) * mask_color[c]

        return image

    def __getitem__(self, index):
        batch_indices = self.indices[index * self.batch_size:(index + 1) * self.batch_size]
        batch_results = []

        for i in batch_indices:
            img_path = self.image_files[i]
            mask_path = self.mask_files[i]

            image = cv2.imread(img_path)
            mask = cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)

            if image is None or mask is None:
                raise ValueError(f"Не удалось загрузить {img_path} или {mask_path}")

            aug_image, aug_mask = self.apply_augmentation(image, mask)
            result = self.apply_mask(aug_image, aug_mask)

            
            result = result / 255.0

            batch_results.append(result)

        return np.array(batch_results)

    def __len__(self):
        return len(self.indices) // self.batch_size

def main():
    image_dir = "archive/images/"
    mask_dir = "archive/labels/"
    generator = DeGenerator(image_dir, mask_dir, batch_size=4, res_size=(400, 400))
    
    
    for i in range(len(generator)):
        batch = generator[i]
    
        cv2.imshow('res', batch[0])
        if cv2.waitKey(0) & 0xFF == 27:  
            break

    cv2.destroyAllWindows()


main()