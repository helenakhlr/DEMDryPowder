# DEMDryPowder

This repository contains a DEM (Discrete Element Method) Simulations of a schematic Bulk Flow Measurement of a Powder, for which MercuryDPM was used.

## Requirements

The following external dependencies need to be installed in order to run MercuryDPM:

### For Linux Users:

```
sudo apt install build-essential g++ gfortran git cmake python3 python3-dev gdb
sudo apt install doxygen graphviz
sudo apt install libx11-dev libxt-dev libxres-dev libxtst-dev 
sudo apt install paraview 
sudo apt install openmpi-bin mpi-default-dev 
sudo apt install ninja-build
```

### For MacOS Users:

```
brew install --cask cmake
brew install git
brew install python
brew install gcc
```

Optional Dependencies:
```
brew install --cask xquartz
brew install doxygen
brew install graphviz
```

## How-to:

Now, to run it in the MercuryDPM framework, the following steps need to be followed:
1. 
```
git clone https://bitbucket.org/mercurydpm/mercurydpm.git MercurySource
```
2.
```
mkdir MercuryBuild
cd MercuryBuild
```
Generate the makefiles using cmake:
```
cmake ../MercurySource
```	
Note: you can also build in Release mode:
```	
cmake ../MercurySource  -DCMAKE_BUILD_TYPE=Release
```	
Compile and test the installation:
```	
make fullTest
```	

3. The file contents of DEMBuild/dem_drypowder.cpp can then be copied into MercurySource/Drivers/Tutorials/Tutorial11_AxisymmetricWalls.cpp
4. from
```	
cd MercurySource/Drivers/Tutorials
```	
run
```	
./Tutorial11_AxisymmetricWalls
```	
and output files of type .vtu will be generated in the folder



