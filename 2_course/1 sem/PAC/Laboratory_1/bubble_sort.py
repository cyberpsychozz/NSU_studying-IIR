import random
import argparse

parser = argparse.ArgumentParser(description = "console processing")

parser.add_argument("mass_len", type = int, help = "lenght of massive")

args = parser.parse_args()

lenght = args.mass_len

mass = []

for i in range(lenght):
    mass.append(random.random())
    print (mass[i])

buff = 0
print("\n")

for i in range(lenght - 1):
    for j in range(lenght - 1, i, -1): 
        if(mass[j - 1] > mass[j]):
            mass[j - 1], mass[j] = mass[j], mass[j - 1]

for i in range(lenght): 
    print(mass[i])        
