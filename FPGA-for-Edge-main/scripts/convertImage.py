"""
Image to Array
This script will convert the input image into an array.
This array can be used to store the image in Vivado HLS scripts
"""
# Importing Required Modules
import cv2
import time

# File Names
file_head = "output/image.h"
image_name = "testImages/t1.jpg"

# Function File C Header Writer
def file_header(file, data, h, w, c):
    """
    This function will write the C header file to store images.
    file - path with filename
    data - image data
    norm - Normalization
    h    - height
    w    - width
    c    - channel
    """
    # Opening File
    with open(file, 'w', encoding='utf-8') as file:
        # headers for the file is written
        file.write("#ifndef __IMAGE_H__\n")
        file.write("#define __IMAGE_H__\n\n")

        # Array Creation for ImageData 
        file.write("float imageData [] = {\n")

        count = 0
        for k in range(0, h):
            for i in range(0, w*c):
                if i == 0:
                    file.write(str(data[k][i]) + ", ")
                elif i == w*c-1:
                    file.write(str(data[k][i]))
                    if k != h-1:
                        file.write(", ")
                else:
                    file.write(str(data[k][i]) + ", ")
                count += 1
        print(count)
        file.write("};")
        file.write("\n\n#endif\n")

# Execution Time
start = time.time()

# Reading Inputs for conversion
print("Image to Array Conversion")
print("RGB to 1d Array")

# Image Read and pre-processing
img = cv2.imread(image_name)
img = cv2.resize(img, (64, 64))
h, w, c = img.shape

# Empty list
img_linear, row_array = [], []

# Converting 3d Array to 1d list
for i in range(0, h):
    img_linear = []
    for j in range(0, w):
        for k in range(0, c):
            img_linear.append(img[i][j][k])
    row_array.append(img_linear)

# Image array is stored as list
print(len(row_array)*len(row_array[0]))

# Function call to complete Tasks
file_header(file_head, row_array, h, w, c)

# Execution Time
end = time.time()

elapsedTime = int((end - start) * 1000)
print("Conversion Completed in " + str(elapsedTime) + " ms")

cv2.imshow("result", img)
cv2.waitKey(0)
cv2.destroyAllWindows()
