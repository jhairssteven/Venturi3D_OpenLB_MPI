#make clean; make
# small test values
# weak_scaling_resolution=10 # resolution: number of voxels per characteristic physical length (charPhysL)
# max_processes=2
# num_processes_strong_scaling=14
# resolutions=(10 50 70 100 1000 10000)

## real values
weak_scaling_resolution=100 # resolution: number of voxels per characteristic physical length (charPhysL)
max_processes=17
num_processes_strong_scaling=4
resolutions=(10 50 70 100 1000 10000)
echo "Weak scaling"
echo "size MLUPs np time" >> weak_scaling.txt
for th in `seq $max_processes`; do 
    echo "Executing  np" $th "size" $weak_scaling_resolution
    
    mpirun -np $th --oversubscribe ./cavity3d --steps $weak_scaling_resolution >> weak_scaling.txt
done

# strong scaling
echo "Strong scaling"
echo "size MLUPs np time" >> strong_scaling.txt
for size in ${resolutions[@]}; do
    echo "Executing np" $num_processes_strong_scaling "size" $size
    mpirun -np $num_processes_strong_scaling --oversubscribe ./cavity3d --steps $size >> strong_scaling.txt
done
