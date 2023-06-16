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

def plot_vectors(filename):
    datos=np.genfromtxt(filename)

    uniform_dis = datos[0,:]
    normal_dis = datos[1,:]

    fig, axs = plt.subplots(2)
    fig.suptitle("Error (%) vs Num. Dato")

    axs[0].hist(uniform_dis, bins=10)
    axs[0].set_title("Dist. Uniforme")
    
    axs[1].hist(normal_dis, bins=30)
    axs[1].axvline(x=-10, linestyle='--', linewidth=1, color='grey')
    axs[1].set_title("\nDist. Normal")

    fig.savefig( OUTPUT_PDF_DIR + "/" + "histogramas.pdf")

setup_dirs()
plot_vectors("data.dat")