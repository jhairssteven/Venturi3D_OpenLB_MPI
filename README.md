# OpenLB_MPI
Ejecución y análisis de rendimiento *weak sacling* y *strong scaling* de los ejemplos Venturi3d y Cavity3d de la librería OpenLB utilizando MPI en C++

# Requerimientos
- python
- [openlb](https://www.openlb.net/download/)
- MPI

```shell
$ sudo apt update
$ sudo apt install mpich
```
- pip: Instalar las dependencias para poder plotear las métricas con

```shell
    $ pip install -r cavity3d/requirements.txt
```

# Start up
- Con openlb instalado, clonar este repositorio en el directorio `openlb/examples/`
 
# Configuración de openlb para soporte MPI
Reemplazar el contenido del archivo `config.mk` en el directorio principal de **openlb** por el del archivo `cpu_gcc_openmpi.mk` ubicado en **openlb/config/cpu_gcc_openmpi.mk**. Posteriormente, **en el root de openlb, realizar la compilación de toda la librería openlb con:**

```shell
openlb/ $ make clean; make
```
# Ejecución
## Ejemplo venturi3d
```shell
$ cd Venturi3D_OpenLB_MPI/src/
$ make
$ mpirun -n 1 ./venturi3dMPI
```

## Ejemplo cavity3d usando MPI
> *Tenga en cuenta que para obtener resultados acordes debe haber configurado openlb para soporte mpi conforme [soporte MPI](#configuración-de-openlb-para-soporte-mpi).*
El archivo `MyMakefile.mk` se encarga de compilar el programa, calcular las métricas de *strong* y *weak scaling* y de plotear los resultados. Ejecutelo con:

```shell
$ make -f MyMakefile.mk
```

Los resultados se pueden ver en los archivos en el directorio [output](/cavity3d/output/PDFs/) creado automáticamente. Ejemplo de los resultados:


| ![Eficiencia paralela](/cavity3d/images/Parallel%20Efficiency.png)  | ![Speedup](/cavity3d/images/speedup.png) | ![Strong scaling](/cavity3d/images/strong_scaling.png) |
|:---:|:---:|:---:|
