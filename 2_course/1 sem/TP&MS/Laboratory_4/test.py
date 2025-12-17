import numpy as np
import matplotlib.pyplot as plt
# %matplotlib_inline

def bernoulli_to_poisson(Lambda, n, N):
    p = min(Lambda/n, 1)
    q = 1 - p
    
    succeses = np.zeros(N, dtype= int)
    distances = []
    last_succes_idx = None
    rng = np.random.default_rng()

    for i in range(N):
        line = rng.binomial(n = 1, p = p, size= n)
        # print(line)
        for j in range(len(line)):
            if line[j] == 1 and last_succes_idx is not None:
                distances.append(j - last_succes_idx-1)
                last_succes_idx = j
            elif last_succes_idx is None and line[j] == 1:
                last_succes_idx = j

        last_succes_idx = None
        succeses[i] = line.sum()
    return succeses, np.array(distances)

def histogram(data, show_plot=True):
    
    n = len(data)
    
    # Количество столбцов 
    m = 1 + np.floor(np.log2(n))
    
    # Разбиение [min, max] на m равных отрезков
    data_sorted = np.sort(data)
    min_val = data_sorted[0]
    max_val = data_sorted[-1]
    delta = (max_val - min_val) / m
    
    # Границы бинов
    bin_edges = np.linspace(min_val, max_val, int(m) + 1)
    
    # Частоты (количество k точек на различных равных отрезках)
    hist, _ = np.histogram(data, bins=bin_edges)
    
    # h = (k / n) / delta
    heights = (hist / n) / delta
    
    # Центры столбцов    bin_edges[:-1] - левые концы бинов, bin_edges[1:] - правые концы бинов
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    
    if show_plot:
        plt.bar(bin_centers, heights, width=delta, align='center', edgecolor='black')
        plt.xlabel('Значения')
        plt.ylabel('Плотность')
        plt.title(f'Гистограмма по правилу Стёрджа (m={int(m)}, n={n})')
        plt.show()
    
    return bin_centers, heights

def poisson_process(Lambda, N):
    
    distances = []
    for i in range(N):
        X = np.random.poisson(Lambda)

        if X > 1:
            pts = np.sort(np.random.uniform(0, 1, X))
            dist = np.diff(pts)
            distances.extend(dist)
        
    return np.array(distances)

def poisson_exponential(Lambda, N):
    successes = np.empty(N, dtype= int)

    for i in range(N):

        pos = []
        T = 0.0
        while True:
            delta_T = np.random.exponential(scale= 1 / Lambda)
            T_next = T + delta_T

            if T_next > 1.0:
                break

            pos.append(T_next)
            T = T_next
        successes[i] = len(pos)
    return successes



lam = 5.0
n = 10
N = 5

res, dist = bernoulli_to_poisson(lam, n, N)
histogram(res)
histogram(dist)

pois_dists = poisson_process(1, 1000)
histogram(pois_dists, show_plot=True)

exp_pois_counts = poisson_exponential(1, 1000)
histogram(exp_pois_counts, show_plot=True)
