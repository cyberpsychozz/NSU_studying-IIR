import random
import argparse

def pascal_triangle(rows):
    triangle = []
    
    for i in range(rows):
        
        row = [1] * (i + 1)
        if i > 1: 
            prev_row = triangle[i - 1]
            for j in range(1, i):
                row[j] = prev_row[j - 1] + prev_row[j]
        triangle.append(row)
    return triangle



parser = argparse.ArgumentParser(description = "console processing")

parser.add_argument("triangle_height", type = int, help = "lenght of massive")

args = parser.parse_args()

height = args.triangle_height

triangle = pascal_triangle(height)

for row in triangle:
    print(" ".join(map(str, row)))