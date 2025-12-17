def check_matrix_dimensions(matrix_a, matrix_b, op_type):
    
    if not matrix_a or not matrix_b:
        print("Error: Empty matrix")
        return False

    if len(set(len(row) for row in matrix_a)) != 1:
        print("Error: rows of matrix A has diffirent lenght")
        return False


    if len(set(len(row) for row in matrix_b)) != 1:
        print("Error: rows of matrix B has diffirent lenght")
        return False
    
    if op_type == 1:
        if len(matrix_a[0]) != len(matrix_b):
            print(f"Error: Can't multiply matrices of size {len(matrix_a)}x{len(matrix_a[0])} and {len(matrix_b)}x{len(matrix_b[0])}")
            return False
    
    
    elif op_type == 2:
        hm, wm = len(matrix_a), len(matrix_a[0])
        hk, wk = len(matrix_b), len(matrix_b[0])
        if hk > hm or wk > wm:
            print(f"Error: Kernel size {hk}x{wk} is larger than matrix size {hm}x{wm}")
            return False
        if hm - hk + 1 <= 0 or wm - wk + 1 <= 0:
            print(f"Error: Resulting convolution dimensions ({hm - hk + 1}x{wm - wk + 1}) are invalid")
            return False
    
    return True

def convolution(matrix, kernel):
    print(matrix)
    hm, wm = len(matrix), len(matrix[0])
    hk, wk = len(kernel), len(kernel[0])


    ho, wo = (hm -  hk + 1), (wm - wk + 1)

    result = [[0 for i in range(wo)] for i in range(ho)]
    
    for i in range(ho):
        for j in range(wo):
            summ = 0
            for m in range(hk):
                for n in range(wk):
                    summ += matrix[i + m][j + n] * kernel[m][n]
            result[i][j] = summ
            
    return result


def matrix_multiply(matrix_a, matrix_b, n, m, k):
    matrix_result = [[0 for i in range(m)] for i in range(n)]
    for i in range(n):
        for j in range(k):
            for c in range (m):
                matrix_result[i][j] += matrix_a[i][c] * matrix_b[c][j]
    return matrix_result
    

def file_processing(name_input, name_output, op_type):
    matrix_a, matrix_b = [], []
    fmend = 0
    try:
        with open (name_input, 'r') as f:
            lines = f.readlines()
            # print (lines)
            for s in lines:
                row = list(map(int, s.split()))
                # print(row)
                if(len(s) != 1 and not fmend):
                    matrix_a.append(row)
                    # print (matrix_a)
                elif(len(s) == 1):
                    fmend = 1
                else:
                    matrix_b.append(row)
        if not check_matrix_dimensions(matrix_a, matrix_b, op_type):
            return
    except FileNotFoundError:
        print(f"Error: File '{name_input}' not found")
    # print(matrix_a)
    # print(matrix_b)

    if op_type == 1:
        n,m,k = len(matrix_a), len(matrix_b[0]), len(matrix_b)
        result = matrix_multiply(matrix_a, matrix_b, n, m, k)
    elif(op_type ==2):
        result = convolution(matrix_a, matrix_b)
    
    try:
        with open(name_output, 'w') as f:
            f.writelines("\n".join(map(str, result)))
            print("program completed")

    except FileNotFoundError:
        print(f"Error: File '{name_output}' not found")
        
        


def main(): 
    # input_file_path = input("Enter path to the input file:")
    # output_file_path = input("Enter path to the output file:")
    input_file_path = "matrix.txt"
    output_file_path = "res.txt"
    op_type = int(input("Enter operation type(1 - mult, 2 - conv):"))
    file_processing(input_file_path, output_file_path, op_type)

main()