// This file contains elements of Tutorial 11 of MercuryDPM project (https://www.mercurydpm.org).


#include "Mercury3D.h"
#include "Walls/IntersectionOfWalls.h"
#include "Walls/AxisymmetricIntersectionOfWalls.h"
#include "Species/LinearViscoelasticSpecies.h"
#include "Species/LinearViscoelasticFrictionSpecies.h"
#include <cmath>
#include <limits>

class Powder : public Mercury3D
{
public:
    ParticleSpecies* particleSpecies;
    ParticleSpecies* smoothWallSpecies;
    ParticleSpecies* roughFloorSpecies;
    InfiniteWall* wallInNeck;
    Powder(const Mdouble width, const Mdouble height){
        logger(INFO, "DEM Dry Powder");
        setName("DemDryPowder");
        setFileType(FileType::ONE_FILE);
        restartFile.setFileType(FileType::ONE_FILE);
        fStatFile.setFileType(FileType::NO_FILE);
        eneFile.setFileType(FileType::NO_FILE);
        setParticlesWriteVTK(true);
        wallHandler.setWriteVTK(FileType::ONE_FILE);
        setXBallsAdditionalArguments("-v0 -solidf");

        //specify body forces
        setGravity(Vec3D(0.0, 0.0, -9.8));

        //set domain accordingly (domain boundaries are not walls!)
        setXMin(0.0);
        setXMax(width);
        setYMin(0.0);
        setYMax(width);
        setZMin(0.0);
        setZMax(height);
    }

    //Initial conditions
        void setupInitialConditions() override {
        Vec3D mid = {
                (getXMin() + getXMax()) / 2.0,
                (getYMin() + getYMax()) / 2.0,
                (getZMin() + getZMax()) / 2.0};
        const Mdouble halfWidth = (getXMax() - getXMin()) / 2.0;

        //Finite Cylinder - only in upper half (above the cone)
        IntersectionOfWalls w1;
        w1.setSpecies(smoothWallSpecies);

        const int numSides = 12;  // 12-sided polygon for smooth approximation
        const Mdouble radius = (getXMax() - getXMin()) / 2.0;

        // Cylindrical side walls approximation
        for (int i = 0; i < numSides; i++) {
            Mdouble angle = 2.0 * constants::pi * i / numSides;
            Vec3D normal(cos(angle), sin(angle), 0); 
            Vec3D point(mid.X + radius * cos(angle), mid.Y + radius * sin(angle), 0);
            w1.addObject(normal, point);
        }
        // Top wall
        w1.addObject(Vec3D(0, 0, 1), Vec3D(mid.X, mid.Y, getZMax()));
         
        wallHandler.copyAndAddObject(w1);

        //Cone
        AxisymmetricIntersectionOfWalls w2;
        w2.setSpecies(smoothWallSpecies);
        w2.setPosition(Vec3D(mid.X, mid.Y, 0));
        w2.setAxis(Vec3D(0, 0, 1));
        std::vector<Vec3D> points(3);
        //define the neck as a prism through corners of the prismatic wall in clockwise direction
        points[0] = Vec3D(halfWidth, 0.0, mid.Z + contractionHeight + 20e-2);
        points[1] = Vec3D(halfWidth - contractionWidth, 0.0, mid.Z + 10e-2);
        points[2] = Vec3D(halfWidth, 0.0, mid.Z + 10e-2); 
        w2.createOpenPrism(points);
        // Display the neck only until reaching the cylinder
        std::vector<Mdouble> displayedSegmentsPrism = {0, mid.Z + contractionHeight + 20e-2};
        w2.setDisplayedSegments(displayedSegmentsPrism);
        wallHandler.copyAndAddObject(w2);

        //Flat surface
        InfiniteWall w0;
        w0.setSpecies(roughFloorSpecies);
        w0.set(Vec3D(0, 0, -1), Vec3D(0, 0, mid.Z + 10e-2));
        wallInNeck = wallHandler.copyAndAddObject(w0);

        const int N = 4500; //Number of particles
        SphericalParticle p0;
        p0.setSpecies(particleSpecies);
        for (Mdouble z = mid.Z + contractionHeight + 20e-2 + maxParticleRadius;
             particleHandler.getNumberOfObjects() <= N;
             z += 2.0 * maxParticleRadius)
        {
            for (Mdouble r = halfWidth - maxParticleRadius; r > 0; r -= 1.999 * maxParticleRadius)
            {
                for (Mdouble c = 2.0 * maxParticleRadius; c <= 2 * constants::pi * r; c += 2.0 * maxParticleRadius)
                {
                    p0.setRadius(random.getRandomNumber(minParticleRadius, maxParticleRadius));
                    p0.setPosition(Vec3D(mid.X + r * mathsFunc::sin(c / r),
                                         mid.Y + r * mathsFunc::cos(c / r),
                                         z + p0.getRadius()));
                    p0.setVelocity(Vec3D(0.0,0.0,0.0));
                    particleHandler.copyAndAddObject(p0);
                }
            }
        }
    }
 
    void actionsAfterTimeStep() override
{
    // below this height we kill horizontal motion to simulate friction of powder particles with the bottom floor
    const Mdouble zFreeze = getZMin() + 0.02; 

    for (auto p : particleHandler)
    {
        const Vec3D pos = p->getPosition();
        if (pos.Z < zFreeze)
        {
            Vec3D v = p->getVelocity();
            v.X = 0.0;
            v.Y = 0.0;
            p->setVelocity(v);
        }
    }

    if (getTime() < 0.9 && getTime() + getTimeStep() > 0.9)
        {
            //This removes the bottom wall once the particles have settled in the cone
            logger(INFO, "Shifting bottom wall downward");
            wallInNeck->set(Vec3D(0, 0, -1), Vec3D(0, 0, getZMin()));
        }
}



    Mdouble contractionWidth{};
    Mdouble contractionHeight{};
    Mdouble minParticleRadius{};
    Mdouble maxParticleRadius{};
};


int main(int argc, char *argv[])
{
    Mdouble width = 30e-2; // 30cm
    Mdouble height = 60e-2; // 60cm
 
    //Point the object HG to class
    Powder HG(width,height);
 
    //Specify particle radius:
    HG.minParticleRadius = 6e-3; // 6mm
    HG.maxParticleRadius = 7e-3; //7mm

 
    //specify how big the wedge of the contraction should be
    const Mdouble contractionWidth = 13.2e-2; //13.2cm
    const Mdouble contractionHeight = 5e-2; //5cm
    HG.contractionWidth = contractionWidth;
    HG.contractionHeight = contractionHeight;

    //make the species of the particle and wall
   // Particle species (moderate friction)
    LinearViscoelasticFrictionSpecies particleSpec;
    particleSpec.setDensity(2000);

    particleSpec.setDissipation(9); // initial guess
    particleSpec.setStiffness(1e5); // initial guess

    // Set frictional properties for particles as needed
    particleSpec.setSlidingFrictionCoefficient(0.0);
    particleSpec.setSlidingStiffness(2.0/7.0 * particleSpec.getStiffness());
    particleSpec.setSlidingDissipation(2.0/7.0 * particleSpec.getDissipation());

    particleSpec.setRollingFrictionCoefficient(0.00);
    particleSpec.setRollingStiffness(2.0/7.0 * particleSpec.getStiffness());
    particleSpec.setRollingDissipation(2.0/7.0 * particleSpec.getDissipation());

    // Smooth wall species
    LinearViscoelasticFrictionSpecies smoothWallSpec = particleSpec;
    smoothWallSpec.setSlidingFrictionCoefficient(0.0);  // almost smooth

    // Rough floor species (for bottom wall only)
    LinearViscoelasticFrictionSpecies roughFloorSpec = particleSpec;
    roughFloorSpec.setSlidingFrictionCoefficient(0.0);   // much higher
    roughFloorSpec.setRollingFrictionCoefficient(0.0); // some rolling resistance if available
    roughFloorSpec.setRollingStiffness(2.0/7.0 * particleSpec.getStiffness());         
    roughFloorSpec.setRollingDissipation(2.0/5.0 * particleSpec.getDissipation());    

    auto* pSpecies      = HG.speciesHandler.copyAndAddObject(particleSpec);
    auto* smoothWall    = HG.speciesHandler.copyAndAddObject(smoothWallSpec);
    auto* roughFloor    = HG.speciesHandler.copyAndAddObject(roughFloorSpec);

    HG.particleSpecies   = pSpecies;
    HG.smoothWallSpecies = smoothWall;
    HG.roughFloorSpecies = roughFloor;


    //test normal forces
    const Mdouble minParticleMass = particleSpec.getDensity() * 4.0 / 3.0 * constants::pi * mathsFunc::cubic(HG.minParticleRadius);
    //Calculates collision time for two copies of a particle of given dissipation_, k, effective mass
    logger(INFO, "minParticleMass = %", minParticleMass);
    //Calculates collision time for two copies of a particle of given dissipation_, k, effective mass
    const Mdouble tc = particleSpec.getCollisionTime(minParticleMass);
    logger(INFO, "tc  = %", tc);
    //Calculates restitution coefficient for two copies of given dissipation_, k, effective mass
    const Mdouble r = particleSpec.getRestitutionCoefficient(minParticleMass);
    logger(INFO, "restitution coefficient = %", r);

    //time integration parameters
    HG.setTimeStep(tc / 10.0);
    HG.setTimeMax(30);
    HG.setSaveCount(500);

    HG.solve(argc, argv);
    return 0;
}
