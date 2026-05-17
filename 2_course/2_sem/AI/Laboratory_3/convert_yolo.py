import os
import shutil

def convert_to_yolo():
    base_path = "/home/cyberpsychoz/NSU_studying-IIR/2_course/2_sem/AI/Laboratory_3"
    dataset_path = os.path.join(base_path, "dataset")
    
    # Files to process
    splits = {
        "train": "train.txt",
        "val": "val.txt",
        "test": "test.txt"
    }
    
    # Create directories if they don't exist
    for split in splits:
        os.makedirs(os.path.join(dataset_path, split, "images"), exist_ok=True)
        os.makedirs(os.path.join(dataset_path, split, "labels"), exist_ok=True)
        
    for split, filename in splits.items():
        txt_file = os.path.join(base_path, filename)
        if not os.path.exists(txt_file):
            print(f"File {txt_file} not found, skipping.")
            continue
            
        with open(txt_file, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            line = line.strip()
            if not line:
                continue
            
            # The paths in train.txt look like 'data/plane/img/AMST_001.jpg'
            # We need to map them to the local 'img' directory
            img_filename = os.path.basename(line)
            label_filename = os.path.splitext(img_filename)[0] + ".txt"
            
            src_img = os.path.join(base_path, "img", img_filename)
            src_label = os.path.join(base_path, "img", label_filename)
            
            dst_img = os.path.join(dataset_path, split, "images", img_filename)
            dst_label = os.path.join(dataset_path, split, "labels", label_filename)
            
            if os.path.exists(src_img):
                shutil.copy2(src_img, dst_img)
            else:
                print(f"Warning: Image {src_img} not found.")
                
            if os.path.exists(src_label):
                shutil.copy2(src_label, dst_label)
            else:
                print(f"Warning: Label {src_label} not found.")

if __name__ == "__main__":
    convert_to_yolo()
    print("Conversion completed!")
