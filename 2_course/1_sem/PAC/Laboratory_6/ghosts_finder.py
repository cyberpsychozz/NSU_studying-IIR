import cv2
import numpy as np
from sklearn.cluster import DBSCAN


def find_unique_points(image, original_image):
    im_gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    base_gray = cv2.cvtColor(original_image, cv2.COLOR_BGR2GRAY)
    mir_im = im_gray.copy()
    mir_im = cv2.flip(mir_im, 1)

    sift = cv2.SIFT_create()

    keypoints_1, descriptors_1 = sift.detectAndCompute(base_gray, None)
    keypoints_2, descriptors_2 = sift.detectAndCompute(im_gray, None)
    keypoints_3, descriptors_3 = sift.detectAndCompute(mir_im, None)

    bf = cv2.BFMatcher(cv2.NORM_L2, crossCheck=True) 
    
    matches1 = bf.match(descriptors_1, descriptors_2)
    matches2 = bf.match(descriptors_1, descriptors_3)

   
    matches1 = sorted(matches1, key = lambda x:x.distance)
    matches2 = sorted(matches2, key = lambda x:x.distance)


    src_pts_1 = np.float32([keypoints_1[m.queryIdx].pt for m in matches1]).reshape(-1, 2)
    dst_pts_1 = np.float32([keypoints_2[m.trainIdx].pt for m in matches1]).reshape(-1, 2)

    src_pts_2 = np.float32([keypoints_1[m.queryIdx].pt for m in matches2]).reshape(-1, 2)
    dst_pts_2 = np.float32([keypoints_3[m.trainIdx].pt for m in matches2]).reshape(-1, 2)

    all_src_pts = np.vstack([src_pts_1, src_pts_2])
    all_dst_pts = np.vstack([dst_pts_1, dst_pts_2])

    if len(all_src_pts) < 4 or len(all_dst_pts) < 4:
        print("Недостаточно соответствий для вычисления гомографии")
        return original_image.copy(), original_image.copy()

    
    dbscan = DBSCAN(eps= 100, min_samples=4)
    clusters = dbscan.fit_predict(all_src_pts)

    result = original_image.copy()

    for cluster_id in np.unique(clusters):
        if cluster_id == -1: continue  
        cluster_mask = clusters == cluster_id
        src_cluster = all_src_pts[cluster_mask]
        dst_cluster = all_dst_pts[cluster_mask]

        if len(src_cluster) < 18: continue  

        H, mask = cv2.findHomography(src_cluster, dst_cluster, cv2.RANSAC, 5.0)

        
        if H is None or H.shape[0] < 3 or H.shape[1] < 3:
            print(f"Некорректная гомография для кластера {cluster_id}")
            continue

        h, w = image.shape[:2]
        corners = np.float32([[0, 0], [w - 1, 0], [w - 1, h - 1], [0, h-1]]).reshape(-1, 1, 2)
        H_inv = np.linalg.inv(H)
        transformed_corners = cv2.perspectiveTransform(corners, H_inv)

        top_left = (int(min(transformed_corners[:, 0, 0])), int(min(transformed_corners[:, 0, 1])))
        bottom_right = (int(max(transformed_corners[:, 0, 0])), int(max(transformed_corners[:, 0, 1])))

        cv2.rectangle(result, top_left, bottom_right, (0, 255, 0), 5)

    return result

scene = cv2.imread('images/lab7.png')  
candy_ghost = cv2.imread('images/candy_ghost.png') 
pampkin_ghost = cv2.imread('images/pampkin_ghost.png')
scary_ghost = cv2.imread('images/scary_ghost.png')

result = scene.copy()
result = find_unique_points(candy_ghost, result)
result = find_unique_points(pampkin_ghost, result)
result = find_unique_points(scary_ghost, result)

x, y = result.shape[:2]
result = cv2.resize(result, (y//2, x//2))



cv2.imshow('res', result)

cv2.waitKey(0)
cv2.destroyAllWindows()
