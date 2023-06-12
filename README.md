# Venturi3D_OpenLB_MPI
Simulación de un venturi 3d utilizando OpenLB y análisis de rendimiento utilizando MPI en C++

# Requerimientos
- [openlb](https://www.openlb.net/download/)
- MPI

```shell
$ sudo apt update
$ sudo apt install mpich
```

# Start up
- Clonar este repositorio en el directorio `openlb/examples/`

```shell
$ cd Venturi3D_OpenLB_MPI/src/
$ make
$ mpirun -n 3 --oversubscribe ./venturi3dMPI
```
