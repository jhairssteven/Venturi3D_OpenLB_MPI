import numpy as np
import matplotlib.pylab as plt


DAT_FILES_DIR = "./output"
OUTPUT_PDF_DIR = DAT_FILES_DIR + "/PDFs"

import os
""" Crea si no existen los directorios de datos y guardado de PDFs """
def setup_dirs():
    def f(dn):
        dir_exists = os.path.exists(dn)
        if not dir_exists:
            os.makedirs(dn)
    f(DAT_FILES_DIR)
    f(OUTPUT_PDF_DIR)

def read_results(filename):
    datos=np.genfromtxt(filename)
    steps = datos[:, 0][1:]
    MLUPs = datos[:, 1][1:]
    np_values = [int(value) for value in datos[:, 2][1:]]
    time = datos[:, 3][1:]
    return datos, steps, MLUPs, np_values, time

def plot_strong_scaling(filename):
    datos, steps, MLUPs, np_values, time = read_results(filename)

    plt.plot(steps, time)
    plt.title("Number of processes: 4")
    plt.xlabel("Resolution")
    plt.ylabel("Time")
    plt.savefig( OUTPUT_PDF_DIR + "/" + "strong_scaling.pdf")

def plot_weak_scaling(filename):
    datos, steps, MLUPs, np_values, time = read_results(filename)
    seq_time = time[0]
    speedup = seq_time / time
    parallel_efficiency = speedup / np_values
    
    # Plot speedup vs np_values
    plt.figure()
    plt.plot(np_values, speedup, marker='o')
    plt.plot(np_values, np_values, linestyle='--', color='gray')  # Plot np_values vs np_values
    plt.xlabel('np_values')
    plt.ylabel('Speedup')
    plt.title('Speedup vs np_values')
    plt.savefig( OUTPUT_PDF_DIR + "/" + "speedup.pdf")
    # Plot parallel_efficiency vs np_values
    plt.figure()
    plt.plot(np_values, parallel_efficiency, marker='o')
    plt.axhline(y=1, color='r', linestyle='--')  # Horizontal line at parallel_efficiency=1
    plt.axhline(y=0.6, color='g', linestyle='--')  # Horizontal line at parallel_efficiency=0.6
    plt.xlabel('np_values')
    plt.ylabel('Parallel Efficiency')
    plt.title('Parallel Efficiency vs np_values')

    plt.savefig( OUTPUT_PDF_DIR + "/" + "Parallel Efficiency.pdf")

setup_dirs()
plot_strong_scaling("strong_scaling.txt")
plot_weak_scaling("weak_scaling.txt")