import random
import argparse
import numpy as np
import time

def parse_console():
    parser = argparse.ArgumentParser(description="Console processing")
    parser.add_argument("filename1", type=str, help="Name of file 1")
    parser.add_argument("filename2", type=str, help="Name of file 2")
    parser.add_argument("probability", type=float, help="Probability of elements")
    args = parser.parse_args()
    filename1, filename2, p = args.filename1, args.filename2, args.probability


    if not (0 <= p <= 1):
        print("Error: Probability must be between 0 and 1")
        exit(1)


    array1, array2 = [], []
    try:
        with open(filename1, 'r') as f:
            line = f.readline()
            array1 = list(map(int, line.split()))
    except FileNotFoundError:
        print(f"Error: File '{filename1}' not found")
        exit(1)
    except ValueError:
        print(f"Error: Invalid data in '{filename1}'")
        exit(1)

    try:
        with open(filename2, 'r') as f:
            line = f.readline()
            array2 = list(map(int, line.split()))
    except FileNotFoundError:
        print(f"Error: File '{filename2}' not found")
        exit(1)
    except ValueError:
        print(f"Error: Invalid data in '{filename2}'")
        exit(1)

    if len(array1) != len(array2):
        print("Error: Arrays must have the same length")
        exit(1)

    return array1, array2, p

def shuffle_1(array1, array2, probability):
    """Метод 1: Использование маски вероятностей"""
    mask = np.random.random(len(array1)) < probability
    result = np.where(mask, array2, array1)  
    return result

def shuffle_2(array1, array2, probability):
    """Метод 2: Векторизованный выбор с np.random.choice"""
    choices = np.column_stack((array1, array2))  
    
    result = np.random.choice([0, 1], size=len(array1), p=[1-probability, probability])
    return np.where(result, array2, array1).astype(int)

def shuffle_3(array1, array2, probability):
    """Метод 3: Использование np.random.permutation для комбинации"""
    
    indices = np.arange(len(array1))
    n_select = int(probability * len(array1))
    select_indices = np.random.permutation(indices)[:n_select]
    # mask = np.zeros(len(array1), dtype=bool)
    # mask[select_indices] = True
    result = array1
    result[select_indices] = array2[select_indices]
    return result.astype(int)
    

def main():

    np.random.seed(int(time.time()))

    arr1, arr2, p = parse_console()

    arr1 = np.array(arr1, dtype=int)
    arr2 = np.array(arr2, dtype=int)

    result1 = shuffle_1(arr1, arr2, p)
    result2 = shuffle_2(arr1, arr2, p)
    result3 = shuffle_3(arr1, arr2, p)


    print("Method 1:", result1)
    print("\nMethod 2:", result2)
    print("\nMethod 3:", result3)

main()