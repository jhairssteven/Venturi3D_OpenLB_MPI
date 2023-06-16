#include "mpi.h"
#include "olb3D.h"
#include "olb3D.hh"

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;

using T          = FLOATING_POINT_TYPE;
using DESCRIPTOR = D3Q19<>;

T maxPhysT = 10.0; // max. simulation time in s, SI unit

/* Reads geometry characteristics from a file and discretizes the 
geometry with a given voxel resolution. Also, distributes efficiently
the geometry voxels deppending the number of used processes. */
SuperGeometry<T, 3> prepareGeometry()
{

  OstreamManager clout(std::cout, "prepareGeometry");
  clout << "Prepare Geometry ..." << std::endl;

  std::string fName("venturi3dMPI.xml");
  XMLreader   config(fName);

  // Read properties from config file
  std::shared_ptr<IndicatorF3D<T>> inflow = createIndicatorCylinder3D<T>(
      config["Geometry"]["Inflow"]["IndicatorCylinder3D"], false);
  std::shared_ptr<IndicatorF3D<T>> outflow0 = createIndicatorCylinder3D<T>(
      config["Geometry"]["Outflow0"]["IndicatorCylinder3D"], false);
  std::shared_ptr<IndicatorF3D<T>> outflow1 = createIndicatorCylinder3D<T>(
      config["Geometry"]["Outflow1"]["IndicatorCylinder3D"], false);

  std::shared_ptr<IndicatorF3D<T>> venturi =
      createIndicatorF3D<T>(config["Geometry"]["Venturi"], false);

  // Build CoboidGeometry from IndicatorF (weights are set, remove and shrink is done)
  /* N: number of voxels per Characteristic Physical Length (a real length). 
  Used to control the level of discretization for the simulation. Higher N brings
  better accuracy, but higher computational cost. */
  int N = config["Application"]["Discretization"]["Resolution"].get<int>();

  /*singleton::mpi().getSize(): The number of processes used for the simulation. 
  This determines the load balancing of the geometry.*/
  CuboidGeometry3D<T> cuboidGeometry =
      new CuboidGeometry3D<T>(*venturi, 1. / N, singleton::mpi().getSize());

  // Build LoadBalancer from CuboidGeometry (weights are respected)
  /* The load balancer is responsible for distributing the computational 
  load across multiple processes efficiently, respecting the weights of 
  the cuboid geometry.*/
  
  /*HeuristicLoadBalancer<T>* loadBalancer =
      new HeuristicLoadBalancer<T>(*cuboidGeometry);*/
  BlockLoadBalancer<T> loadBalancer(singleton::mpi().getRank(), singleton::mpi().getSize(), cuboidGeometry->getNc(), 0);
  // Default instantiation of superGeometry
  // '3' argument is the simulation's dimensionality (i.e.: Geometry dimension)
  SuperGeometry<T, 3> superGeometry(cuboidGeometry, loadBalancer, 3);

  // Set boundary voxels by rename material numbers
  superGeometry.rename(0, 2, venturi);
  superGeometry.rename(2, 1, {1, 1, 1});
  superGeometry.rename(2, 3, 1, inflow);
  superGeometry.rename(2, 4, 1, outflow0);
  superGeometry.rename(2, 5, 1, outflow1);

  // Removes all not needed boundary voxels outside the surface
  superGeometry.clean();
  // Removes all not needed boundary voxels inside the surface
  superGeometry.innerClean();
  superGeometry.checkForErrors();

  superGeometry.getStatistics().print();
  superGeometry.communicate();

  clout << "Prepare Geometry ... OK" << std::endl;
  return superGeometry;
}
/* Set material properties for each numbered material*/
void prepareLattice(SuperLattice<T, DESCRIPTOR>&        sLattice,
                    UnitConverter<T, DESCRIPTOR> const& converter,
                    SuperGeometry<T, 3>&                superGeometry)
{

  OstreamManager clout(std::cout, "prepareLattice");
  clout << "Prepare Lattice ..." << std::endl;
  
  // The rate at which the fluid in the simulation relaxes towards equilibrium.
  const T omega = converter.getLatticeRelaxationFrequency();

  // Material=1 -->bulk dynamics
  sLattice.defineDynamics<RLBdynamics>(superGeometry, 1);

  // Material=2 -->bounce back
  // When fluid particles encounter this boundary, they rebound in the opposite direction.
  setBounceBackBoundary(sLattice, superGeometry, 2);

  // Setting of the boundary conditions
  setInterpolatedVelocityBoundary<T, DESCRIPTOR>(sLattice, omega, superGeometry,
                                                 3);
  setInterpolatedPressureBoundary<T, DESCRIPTOR>(sLattice, omega, superGeometry,
                                                 4);
  setInterpolatedPressureBoundary<T, DESCRIPTOR>(sLattice, omega, superGeometry,
                                                 5);

  sLattice.setParameter<descriptors::OMEGA>(omega);

  sLattice.initialize();

  clout << "Prepare Lattice ... OK" << std::endl;
}

// Generates a slowly increasing sinuidal inflow for the first iTMax timesteps
void setBoundaryValues(SuperLattice<T, DESCRIPTOR>&        sLattice,
                       UnitConverter<T, DESCRIPTOR> const& converter, int iT,
                       SuperGeometry<T, 3>& superGeometry)
{
  OstreamManager clout(std::cout, "setBoundaryValues");

  // No. of time steps for smooth start-up
  int iTmaxStart = converter.getLatticeTime(maxPhysT * 0.8);
  int iTperiod   = 50;

  if (iT % iTperiod == 0 && iT <= iTmaxStart) {
    clout << "Set Boundary Values ..." << std::endl;

    //SinusStartScale<T,int> startScale(iTmaxStart, (T) 1);
    PolynomialStartScale<T, int> startScale(iTmaxStart, T(1));
    int                          iTvec[1] = {iT};
    T                            frac     = T();
    startScale(&frac, iTvec);

    // Creates and sets the Poiseuille inflow profile using functors
    CirclePoiseuille3D<T> poiseuilleU(
        superGeometry, 3, frac * converter.getCharLatticeVelocity(), T(),
        converter.getConversionFactorLength());
    sLattice.defineU(superGeometry, 3, poiseuilleU);

    clout << "step=" << iT << "; scalingFactor=" << frac << std::endl;

    sLattice.setProcessingContext<
        Array<momenta::FixedVelocityMomentumGeneric::VELOCITY>>(
        ProcessingContext::Simulation);
  }
  //clout << "Set Boundary Values ... ok" << std::endl;
}

void getResults(SuperLattice<T, DESCRIPTOR>&  sLattice,
                UnitConverter<T, DESCRIPTOR>& converter, int iT,
                SuperGeometry<T, 3>& superGeometry, util::Timer<T>& timer)
{

  OstreamManager      clout(std::cout, "getResults");
  SuperVTMwriter3D<T> vtmWriter("venturi3dMPI");

  if (iT == 0) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeGeometry3D<T, DESCRIPTOR> geometry(sLattice, superGeometry);
    SuperLatticeCuboid3D<T, DESCRIPTOR>   cuboid(sLattice);
    SuperLatticeRank3D<T, DESCRIPTOR>     rank(sLattice);
    vtmWriter.write(geometry);
    vtmWriter.write(cuboid);
    vtmWriter.write(rank);
    vtmWriter.createMasterFile();
  }

  // Writes the vtm files
  if (iT % converter.getLatticeTime(1.) == 0) {
    sLattice.setProcessingContext(ProcessingContext::Evaluation);

    // Create the data-reading functors...
    SuperLatticePhysVelocity3D<T, DESCRIPTOR> velocity(sLattice, converter);
    SuperLatticePhysPressure3D<T, DESCRIPTOR> pressure(sLattice, converter);
    vtmWriter.addFunctor(velocity);
    vtmWriter.addFunctor(pressure);
    vtmWriter.write(iT);

    SuperEuklidNorm3D<T>  normVel(velocity);
    BlockReduction3D2D<T> planeReduction(normVel, {0, 0, 1});

    // write output as JPEG
    heatmap::write(planeReduction, iT);

    // write output as JPEG and changing properties
    heatmap::plotParam<T> jpeg_Param;
    jpeg_Param.name         = "outflow";
    jpeg_Param.contourlevel = 5;
    jpeg_Param.colour       = "blackbody";
    jpeg_Param.zoomOrigin   = {0.6, 0.3};
    jpeg_Param.zoomExtend   = {0.4, 0.7};
    heatmap::write(planeReduction, iT, jpeg_Param);
  }

  // Writes output on the console
  if (iT % converter.getLatticeTime(1.) == 0) {
    timer.update(iT);
    timer.printStep();
    sLattice.getStatistics().print(iT, converter.getPhysTime(iT));
  }
  
}

int main(int argc, char* argv[])
{

  // === 1st Step: Initialization ===
  //MPI_Init(&argc, &argv);  
 /*  int pid, np;
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid); */

  olbInit(&argc, &argv);
  singleton::directories().setOutputDir("./tmp/");
  OstreamManager clout(std::cout, "main");
  // display messages from every single mpi process
  // clout.setMultiOutput(true);

  std::string fName("venturi3dMPI.xml");
  XMLreader   config(fName);

  UnitConverter<T, DESCRIPTOR>* converter =
      createUnitConverter<T, DESCRIPTOR>(config);

  // Prints the converter log as console output
  converter->print();
  // Writes the converter log in a file
  converter->write("venturi3dMPI");

  // === 2nd Step: Prepare Geometry ===

  SuperGeometry<T, 3> superGeometry(prepareGeometry());

  // === 3rd Step: Prepare Lattice ===

  SuperLattice<T, DESCRIPTOR> sLattice(superGeometry);

  prepareLattice(sLattice, *converter, superGeometry);

  util::Timer<T> timer(converter->getLatticeTime(maxPhysT),
                       superGeometry.getStatistics().getNvoxel());
  timer.start();
  getResults(sLattice, *converter, 0, superGeometry, timer);

  // === 4th Step: Main Loop with Timer ===
  for (std::size_t iT = 0; iT <= converter->getLatticeTime(maxPhysT); ++iT) {

    // === 5th Step: Definition of Initial and Boundary Conditions ===
    setBoundaryValues(sLattice, *converter, iT, superGeometry);

    // === 6th Step: Collide and Stream Execution ===
    sLattice.collideAndStream();

    // === 7th Step: Computation and Output of the Results ===
    getResults(sLattice, *converter, iT, superGeometry, timer);

    if (singleton::mpi().isMainProcessor()) {
      std::cout << singleton::mpi().getSize() << std::endl;
    }
  }

  timer.stop();
  timer.printSummary();
  //MPI_Finalize();  
  /* std::ofstream outputFile("./scaling/timer_summary.txt", std::ios::app);
  std::streambuf* oldBuffer = std::cout.rdbuf(outputFile.rdbuf());
    clout << np << " " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()/1000.0 << std::endl;
  std::cout.rdbuf(oldBuffer);
  outputFile.close();
  */
   
  return 0;
}
