/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
// Please retain the following copyright notice
/******************************************************************
 *  based on mcxyz.c Oct2014
 *
 *  mcxyz.c, in ANSI Standard C programing language
 *
 *  created 2010, 2012 by
 *  Steven L. JACQUES
 *  Ting LI
 *  Oregon Health & Science University
 *
 *******************************************************************/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <chrono>

#include <vector>
#include <iostream>

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include "mitkImageCast.h"
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkCoreServices.h>
#include <mitkPropertyPersistence.h>
#include <mitkPropertyPersistenceInfo.h>

#include <mitkPAProbe.h>
#include <mitkPALightSource.h>
#include <mitkPAMonteCarloThreadHandler.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define ls          1.0E-7      /* Moving photon a little bit off the voxel face */
#define PI          3.1415926
#define ALIVE       1           /* if photon not yet terminated */
#define DEAD        0           /* if photon is to be terminated */
#define THRESHOLD   0.01        /* used in roulette */
#define CHANCE      0.1         /* used in roulette */
#define SQR(x)      (x*x)
#define SIGN(x)     ((x)>=0 ? 1:-1)
#define ONE_MINUS_COSZERO 1.0E-12   /* If 1-cos(theta) <= ONE_MINUS_COSZERO, fabs(theta) <= 1e-6 rad. */
 /* If 1+cos(theta) <= ONE_MINUS_COSZERO, fabs(PI-theta) <= 1e-6 rad. */

 /* Struct for storing x,y and z coordinates */
struct Location
{
  int x, y, z;
  double absorb;
};

struct Location initLocation(int x, int y, int z, double absorb)
{
  struct Location loc;
  loc.x = x;
  loc.y = y;
  loc.z = z;
  loc.absorb = absorb;
  return loc;
}

class DetectorVoxel
{
public:
  Location location;
  std::vector<Location>* recordedPhotonRoute = new std::vector<Location>();
  double* fluenceContribution;
  double m_PhotonNormalizationValue;
  long m_NumberPhotonsCurrent;

  DetectorVoxel(Location location, long totalNumberOfVoxels, double photonNormalizationValue)
  {
    this->location = location;
    this->fluenceContribution = (double *)malloc(totalNumberOfVoxels * sizeof(double));
    for (int j = 0; j < totalNumberOfVoxels; j++)   fluenceContribution[j] = 0; // ensure fluenceContribution[] starts empty.
    m_NumberPhotonsCurrent = 0;
    m_PhotonNormalizationValue = photonNormalizationValue;
  }
};

bool verbose(false);

class InputValues
{
private:
  std::string inputFilename;
  int tissueIterator;
  long long ix, iy, iz;
public:
  int    mcflag, launchflag, boundaryflag;
  double  xfocus, yfocus, zfocus;
  double  ux0, uy0, uz0;
  double  radius;
  double  waist;
  double  xs, ys, zs;    /* launch position */
  int Nx, Ny, Nz, numberOfTissueTypes; /* # of bins */

  char* tissueType;
  double* muaVector;
  double* musVector;
  double* gVector;
  double* normalizationVector;

  double xSpacing, ySpacing, zSpacing;
  double simulationTimeFromFile;
  long long Nphotons;
  long totalNumberOfVoxels;
  double* totalFluence;
  std::string myname;
  DetectorVoxel* detectorVoxel;
  mitk::Image::Pointer m_inputImage;
  mitk::Image::Pointer m_normalizationImage;

  InputValues()
  {
    tissueType = nullptr;
    muaVector = nullptr;
    musVector = nullptr;
    gVector = nullptr;
    detectorVoxel = nullptr;
    normalizationVector = nullptr;

    mcflag = 0;
    launchflag = 0;
    boundaryflag = 0;
  }

  double GetNormalizationValue(int x, int y, int z)
  {
    if (normalizationVector)
      return normalizationVector[z*Ny*Nx + x*Ny + y];
    else
      return 1;
  }

  void LoadValues(std::string localInputFilename, float yOffset, std::string normalizationFilename, bool simulatePVFC)
  {
    inputFilename = localInputFilename;
    try
    {
      if (verbose) std::cout << "Loading input image..." << std::endl;
      m_inputImage = mitk::IOUtil::Load<mitk::Image>(inputFilename);
      if (verbose) std::cout << "Loading input image... [Done]" << std::endl;
    }
    catch (...)
    {
      if (verbose) std::cout << "No .nrrd file found ... switching to legacy mode." << std::endl;
    }

    try
    {
      if (simulatePVFC && !normalizationFilename.empty())
        m_normalizationImage = mitk::IOUtil::Load<mitk::Image>(normalizationFilename);
    }
    catch (...)
    {
      if (verbose) std::cout << "No normalization .nrrd file found ... will not normalize PVFC" << std::endl;
    }

    if (m_normalizationImage.IsNotNull())
    {
      mitk::ImageReadAccessor readAccess3(m_normalizationImage, m_normalizationImage->GetVolumeData(0));
      normalizationVector = (double *)readAccess3.GetData();
    }

    if (m_inputImage.IsNotNull()) // load stuff from nrrd file
    {
      simulationTimeFromFile = 0;

      Nx = m_inputImage->GetDimensions()[1];
      Ny = m_inputImage->GetDimensions()[0];
      Nz = m_inputImage->GetDimensions()[2];

      xSpacing = m_inputImage->GetGeometry(0)->GetSpacing()[0];
      ySpacing = m_inputImage->GetGeometry(0)->GetSpacing()[1];
      zSpacing = m_inputImage->GetGeometry(0)->GetSpacing()[2];

      mcflag = std::stoi(m_inputImage->GetProperty("mcflag")->GetValueAsString().c_str()); // mcflag, 0 = uniform, 1 = Gaussian, 2 = iso-pt, 4 = monospectral fraunhofer setup
      launchflag = std::stoi(m_inputImage->GetProperty("launchflag")->GetValueAsString().c_str());// 0 = let mcxyz calculate trajectory, 1 = manually set launch vector
      boundaryflag = std::stoi(m_inputImage->GetProperty("boundaryflag")->GetValueAsString().c_str());// 0 = no boundaries, 1 = escape at boundaries, 2 = escape at surface only

      xs = std::stod(m_inputImage->GetProperty("launchPointX")->GetValueAsString().c_str());
      ys = std::stod(m_inputImage->GetProperty("launchPointY")->GetValueAsString().c_str()) + yOffset;
      zs = std::stod(m_inputImage->GetProperty("launchPointZ")->GetValueAsString().c_str());

      xfocus = std::stod(m_inputImage->GetProperty("focusPointX")->GetValueAsString().c_str());
      yfocus = std::stod(m_inputImage->GetProperty("focusPointY")->GetValueAsString().c_str());
      zfocus = std::stod(m_inputImage->GetProperty("focusPointZ")->GetValueAsString().c_str());

      ux0 = std::stod(m_inputImage->GetProperty("trajectoryVectorX")->GetValueAsString().c_str());
      uy0 = std::stod(m_inputImage->GetProperty("trajectoryVectorY")->GetValueAsString().c_str());
      uz0 = std::stod(m_inputImage->GetProperty("trajectoryVectorZ")->GetValueAsString().c_str());

      radius = std::stod(m_inputImage->GetProperty("radius")->GetValueAsString().c_str());
      waist = std::stod(m_inputImage->GetProperty("waist")->GetValueAsString().c_str());

      totalNumberOfVoxels = Nx*Ny*Nz;
      if (verbose) std::cout << totalNumberOfVoxels << " = sizeof totalNumberOfVoxels" << std::endl;

      muaVector = (double *)malloc(totalNumberOfVoxels * sizeof(double));  /* tissue structure */
      musVector = (double *)malloc(totalNumberOfVoxels * sizeof(double));  /* tissue structure */
      gVector = (double *)malloc(totalNumberOfVoxels * sizeof(double));  /* tissue structure */

      mitk::ImageReadAccessor readAccess0(m_inputImage, m_inputImage->GetVolumeData(0));
      muaVector = (double *)readAccess0.GetData();
      mitk::ImageReadAccessor readAccess1(m_inputImage, m_inputImage->GetVolumeData(1));
      musVector = (double *)readAccess1.GetData();
      mitk::ImageReadAccessor readAccess2(m_inputImage, m_inputImage->GetVolumeData(2));
      gVector = (double *)readAccess2.GetData();
    }
    else
    {
      mitkThrow() << "No longer support loading of binary tissue files.";
    }
  }
};

class ReturnValues
{
private:
  long i1 = 0, i2 = 31; // used Random Generator
  long ma[56];   // used Random Generator /* ma[0] is not used. */
  long mj, mk;
  short i, ii;
public:
  long long Nphotons;
  double* totalFluence;
  std::string myname;
  DetectorVoxel* detectorVoxel;

  ReturnValues()
  {
    detectorVoxel = nullptr;
    Nphotons = 0;
    totalFluence = nullptr;
  }

  /* SUBROUTINES */

  /**************************************************************************
   *  RandomGen
   *      A random number generator that generates uniformly
   *      distributed random numbers between 0 and 1 inclusive.
   *      The algorithm is based on:
   *      W.H. Press, S.A. Teukolsky, W.T. Vetterling, and B.P.
   *      Flannery, "Numerical Recipes in C," Cambridge University
   *      Press, 2nd edition, (1992).
   *      and
   *      D.E. Knuth, "Seminumerical Algorithms," 2nd edition, vol. 2
   *      of "The Art of Computer Programming", Addison-Wesley, (1981).
   *
   *      When Type is 0, sets Seed as the seed. Make sure 0<Seed<32000.
   *      When Type is 1, returns a random number.
   *      When Type is 2, gets the status of the generator.
   *      When Type is 3, restores the status of the generator.
   *
   *      The status of the generator is represented by Status[0..56].
   *
   *      Make sure you initialize the seed before you get random
   *      numbers.
   ****/
#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC 1.0E-9

  double RandomGen(char Type, long Seed, long *Status) {
    if (Type == 0) {              /* set seed. */
      if (verbose) std::cout << "Initialized random generator " << this << " with seed: " << Seed << std::endl;
      mj = MSEED - (Seed < 0 ? -Seed : Seed);
      mj %= MBIG;
      ma[55] = mj;
      mk = 1;
      for (i = 1; i <= 54; i++) {
        ii = (21 * i) % 55;
        ma[ii] = mk;
        mk = mj - mk;
        if (mk < MZ)
          mk += MBIG;
        mj = ma[ii];
      }
      for (ii = 1; ii <= 4; ii++)
        for (i = 1; i <= 55; i++) {
          ma[i] -= ma[1 + (i + 30) % 55];
          if (ma[i] < MZ)
            ma[i] += MBIG;
        }
      i1 = 0;
      i2 = 31;
    }
    else if (Type == 1) {       /* get a number. */
      if (++i1 == 56)
        i1 = 1;
      if (++i2 == 56)
        i2 = 1;
      mj = ma[i1] - ma[i2];
      if (mj < MZ)
        mj += MBIG;
      ma[i1] = mj;
      return (mj * FAC);
    }
    else if (Type == 2) {       /* get status. */
      for (i = 0; i < 55; i++)
        Status[i] = ma[i + 1];
      Status[55] = i1;
      Status[56] = i2;
    }
    else if (Type == 3) {       /* restore status. */
      for (i = 0; i < 55; i++)
        ma[i + 1] = Status[i];
      i1 = Status[55];
      i2 = Status[56];
    }
    else
      puts("Wrong parameter to RandomGen().");
    return (0);
  }
#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

  /***********************************************************
   *  Determine if the two position are located in the same voxel
   *  Returns 1 if same voxel, 0 if not same voxel.
   ****/
  bool SameVoxel(double x1, double y1, double z1, double x2, double y2, double z2, double dx, double dy, double dz)
  {
    double xmin = min2((floor)(x1 / dx), (floor)(x2 / dx))*dx;
    double ymin = min2((floor)(y1 / dy), (floor)(y2 / dy))*dy;
    double zmin = min2((floor)(z1 / dz), (floor)(z2 / dz))*dz;
    double xmax = xmin + dx;
    double ymax = ymin + dy;
    double zmax = zmin + dz;
    bool sv = 0;

    sv = (x1 <= xmax && x2 <= xmax && y1 <= ymax && y2 <= ymax && z1 < zmax && z2 <= zmax);
    return (sv);
  }

  /***********************************************************
   * max2
   ****/
  double max2(double a, double b) {
    double m;
    if (a > b)
      m = a;
    else
      m = b;
    return m;
  }

  /***********************************************************
   * min2
   ****/
  double min2(double a, double b) {
    double m;
    if (a >= b)
      m = b;
    else
      m = a;
    return m;
  }
  /***********************************************************
   * min3
   ****/
  double min3(double a, double b, double c) {
    double m;
    if (a <= min2(b, c))
      m = a;
    else if (b <= min2(a, c))
      m = b;
    else
      m = c;
    return m;
  }

  /********************
   * my version of FindVoxelFace for no scattering.
   * s = ls + FindVoxelFace2(x,y,z, tempx, tempy, tempz, dx, dy, dz, ux, uy, uz);
   ****/
  double FindVoxelFace2(double x1, double y1, double z1, double /*x2*/, double /*y2*/, double /*z2*/, double dx, double dy, double dz, double ux, double uy, double uz)
  {
    int ix1 = floor(x1 / dx);
    int iy1 = floor(y1 / dy);
    int iz1 = floor(z1 / dz);

    int ix2, iy2, iz2;
    if (ux >= 0)
      ix2 = ix1 + 1;
    else
      ix2 = ix1;

    if (uy >= 0)
      iy2 = iy1 + 1;
    else
      iy2 = iy1;

    if (uz >= 0)
      iz2 = iz1 + 1;
    else
      iz2 = iz1;

    double xs = fabs((ix2*dx - x1) / ux);
    double ys = fabs((iy2*dy - y1) / uy);
    double zs = fabs((iz2*dz - z1) / uz);

    double s = min3(xs, ys, zs);

    return s;
  }

  /***********************************************************
   *  FRESNEL REFLECTANCE
   * Computes reflectance as photon passes from medium 1 to
   * medium 2 with refractive indices n1,n2. Incident
   * angle a1 is specified by cosine value ca1 = cos(a1).
   * Program returns value of transmitted angle a1 as
   * value in *ca2_Ptr = cos(a2).
   ****/
  double RFresnel(double n1,    /* incident refractive index.*/
    double n2,    /* transmit refractive index.*/
    double ca1,    /* cosine of the incident */
    /* angle a1, 0<a1<90 degrees. */
    double *ca2_Ptr)   /* pointer to the cosine */
/* of the transmission */
/* angle a2, a2>0. */
  {
    double r;

    if (n1 == n2) { /** matched boundary. **/
      *ca2_Ptr = ca1;
      r = 0.0;
    }
    else if (ca1 > (1.0 - 1.0e-12)) { /** normal incidence. **/
      *ca2_Ptr = ca1;
      r = (n2 - n1) / (n2 + n1);
      r *= r;
    }
    else if (ca1 < 1.0e-6) {  /** very slanted. **/
      *ca2_Ptr = 0.0;
      r = 1.0;
    }
    else {            /** general. **/
      double sa1, sa2; /* sine of incident and transmission angles. */
      double ca2;      /* cosine of transmission angle. */
      sa1 = sqrt(1 - ca1*ca1);
      sa2 = n1*sa1 / n2;
      if (sa2 >= 1.0) {
        /* double check for total internal reflection. */
        *ca2_Ptr = 0.0;
        r = 1.0;
      }
      else {
        double cap, cam;  /* cosines of sum ap or diff am of the two */
        /* angles: ap = a1 + a2, am = a1 - a2. */
        double sap, sam;  /* sines. */
        *ca2_Ptr = ca2 = sqrt(1 - sa2*sa2);
        cap = ca1*ca2 - sa1*sa2; /* c+ = cc - ss. */
        cam = ca1*ca2 + sa1*sa2; /* c- = cc + ss. */
        sap = sa1*ca2 + ca1*sa2; /* s+ = sc + cs. */
        sam = sa1*ca2 - ca1*sa2; /* s- = sc - cs. */
        r = 0.5*sam*sam*(cam*cam + cap*cap) / (sap*sap*cam*cam);
        /* rearranged for speed. */
      }
    }
    return(r);
  } /******** END SUBROUTINE **********/

  /***********************************************************
   * the boundary is the face of some voxel
   * find the the photon's hitting position on the nearest face of the voxel and update the step size.
   ****/
  double FindVoxelFace(double x1, double y1, double z1, double x2, double y2, double z2, double dx, double dy, double dz, double ux, double uy, double uz)
  {
    double x_1 = x1 / dx;
    double y_1 = y1 / dy;
    double z_1 = z1 / dz;
    double x_2 = x2 / dx;
    double y_2 = y2 / dy;
    double z_2 = z2 / dz;
    double fx_1 = floor(x_1);
    double fy_1 = floor(y_1);
    double fz_1 = floor(z_1);
    double fx_2 = floor(x_2);
    double fy_2 = floor(y_2);
    double fz_2 = floor(z_2);
    double x = 0, y = 0, z = 0, x0 = 0, y0 = 0, z0 = 0, s = 0;

    if ((fx_1 != fx_2) && (fy_1 != fy_2) && (fz_1 != fz_2)) { //#10
      fx_2 = fx_1 + SIGN(fx_2 - fx_1);//added
      fy_2 = fy_1 + SIGN(fy_2 - fy_1);//added
      fz_2 = fz_1 + SIGN(fz_2 - fz_1);//added

      x = (max2(fx_1, fx_2) - x_1) / ux;
      y = (max2(fy_1, fy_2) - y_1) / uy;
      z = (max2(fz_1, fz_2) - z_1) / uz;
      if (x == min3(x, y, z)) {
        x0 = max2(fx_1, fx_2);
        y0 = (x0 - x_1) / ux*uy + y_1;
        z0 = (x0 - x_1) / ux*uz + z_1;
      }
      else if (y == min3(x, y, z)) {
        y0 = max2(fy_1, fy_2);
        x0 = (y0 - y_1) / uy*ux + x_1;
        z0 = (y0 - y_1) / uy*uz + z_1;
      }
      else {
        z0 = max2(fz_1, fz_2);
        y0 = (z0 - z_1) / uz*uy + y_1;
        x0 = (z0 - z_1) / uz*ux + x_1;
      }
    }
    else if ((fx_1 != fx_2) && (fy_1 != fy_2)) { //#2
      fx_2 = fx_1 + SIGN(fx_2 - fx_1);//added
      fy_2 = fy_1 + SIGN(fy_2 - fy_1);//added
      x = (max2(fx_1, fx_2) - x_1) / ux;
      y = (max2(fy_1, fy_2) - y_1) / uy;
      if (x == min2(x, y)) {
        x0 = max2(fx_1, fx_2);
        y0 = (x0 - x_1) / ux*uy + y_1;
        z0 = (x0 - x_1) / ux*uz + z_1;
      }
      else {
        y0 = max2(fy_1, fy_2);
        x0 = (y0 - y_1) / uy*ux + x_1;
        z0 = (y0 - y_1) / uy*uz + z_1;
      }
    }
    else if ((fy_1 != fy_2) && (fz_1 != fz_2)) { //#3
      fy_2 = fy_1 + SIGN(fy_2 - fy_1);//added
      fz_2 = fz_1 + SIGN(fz_2 - fz_1);//added
      y = (max2(fy_1, fy_2) - y_1) / uy;
      z = (max2(fz_1, fz_2) - z_1) / uz;
      if (y == min2(y, z)) {
        y0 = max2(fy_1, fy_2);
        x0 = (y0 - y_1) / uy*ux + x_1;
        z0 = (y0 - y_1) / uy*uz + z_1;
      }
      else {
        z0 = max2(fz_1, fz_2);
        x0 = (z0 - z_1) / uz*ux + x_1;
        y0 = (z0 - z_1) / uz*uy + y_1;
      }
    }
    else if ((fx_1 != fx_2) && (fz_1 != fz_2)) { //#4
      fx_2 = fx_1 + SIGN(fx_2 - fx_1);//added
      fz_2 = fz_1 + SIGN(fz_2 - fz_1);//added
      x = (max2(fx_1, fx_2) - x_1) / ux;
      z = (max2(fz_1, fz_2) - z_1) / uz;
      if (x == min2(x, z)) {
        x0 = max2(fx_1, fx_2);
        y0 = (x0 - x_1) / ux*uy + y_1;
        z0 = (x0 - x_1) / ux*uz + z_1;
      }
      else {
        z0 = max2(fz_1, fz_2);
        x0 = (z0 - z_1) / uz*ux + x_1;
        y0 = (z0 - z_1) / uz*uy + y_1;
      }
    }
    else if (fx_1 != fx_2) { //#5
      fx_2 = fx_1 + SIGN(fx_2 - fx_1);//added
      x0 = max2(fx_1, fx_2);
      y0 = (x0 - x_1) / ux*uy + y_1;
      z0 = (x0 - x_1) / ux*uz + z_1;
    }
    else if (fy_1 != fy_2) { //#6
      fy_2 = fy_1 + SIGN(fy_2 - fy_1);//added
      y0 = max2(fy_1, fy_2);
      x0 = (y0 - y_1) / uy*ux + x_1;
      z0 = (y0 - y_1) / uy*uz + z_1;
    }
    else { //#7
      z0 = max2(fz_1, fz_2);
      fz_2 = fz_1 + SIGN(fz_2 - fz_1);//added
      x0 = (z0 - z_1) / uz*ux + x_1;
      y0 = (z0 - z_1) / uz*uy + y_1;
    }
    //s = ( (x0-fx_1)*dx + (y0-fy_1)*dy + (z0-fz_1)*dz )/3;
    //s = sqrt( SQR((x0-x_1)*dx) + SQR((y0-y_1)*dy) + SQR((z0-z_1)*dz) );
    //s = sqrt(SQR(x0-x_1)*SQR(dx) + SQR(y0-y_1)*SQR(dy) + SQR(z0-z_1)*SQR(dz));
    s = sqrt(SQR((x0 - x_1)*dx) + SQR((y0 - y_1)*dy) + SQR((z0 - z_1)*dz));
    return (s);
  }
};

/* DECLARE FUNCTIONS */

void runMonteCarlo(InputValues* inputValues, ReturnValues* returnValue, int thread, mitk::pa::MonteCarloThreadHandler::Pointer threadHandler);

int detector_x = -1;
int detector_z = -1;
bool interpretAsTime = true;
bool simulatePVFC = false;
int requestedNumberOfPhotons = 100000;
float requestedSimulationTime = 0; // in minutes
int concurentThreadsSupported = -1;
float yOffset = 0; // in mm
bool saveLegacy = false;
std::string normalizationFilename;
std::string inputFilename;
std::string outputFilename;

mitk::pa::Probe::Pointer m_PhotoacousticProbe;

int main(int argc, char * argv[]) {
  mitkCommandLineParser parser;
  // set general information
  parser.setCategory("MITK-Photoacoustics");
  parser.setTitle("Mitk MCxyz");
  parser.setDescription("Runs Monte Carlo simulations on inputed tissues.");
  parser.setContributor("CAI, DKFZ based on code by Jacques and Li");

  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
  parser.beginGroup("Required I/O parameters");
  parser.addArgument(
    "input", "i", mitkCommandLineParser::File,
    "Input tissue file", "input tissue file (*.nrrd)",
    us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "output", "o", mitkCommandLineParser::File,
    "Output fluence file", "where to save the simulated fluence (*.nrrd)",
    us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();
  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool,
    "Verbose Output", "Whether to produce verbose, or rather debug output");
  parser.addArgument(
    "detector-x", "dx", mitkCommandLineParser::Int,
    "Detector voxel x position", "Determines the x position of the detector voxel (default: -1 = dont use detector voxel)", -1);
  parser.addArgument(
    "detector-z", "dz", mitkCommandLineParser::Int,
    "Detector voxel z position", "Determines the z position of the detector voxel (default: -1 = dont use detector voxel)", -1);
  parser.addArgument(
    "number-of-photons", "n", mitkCommandLineParser::Int,
    "Number of photons", "Specifies the number of photons (default: 100000). Simulation stops after that number. Use -t --timer to define a timer instead");
  parser.addArgument(
    "timer", "t", mitkCommandLineParser::Float,
    "Simulation time in min", "Specifies the amount of time for simutation (default: 0). Simulation stops after that number of minutes. -n --number-of-photons is the override and default behavior and defines the maximum number of photons instead. If no simulation time or number of photons is specified the file time is taken.");
  parser.addArgument(
    "y-offset", "yo", mitkCommandLineParser::Float,
    "Probe Y-Offset in mm", "Specifies an offset of the photoacoustic probe in the y direction depending on the initial probe position (default: 0) in mm.");
  parser.addArgument(
    "jobs", "j", mitkCommandLineParser::Int,
    "Number of jobs", "Specifies the number of jobs for simutation (default: -1 which starts as many jobs as supported).");
  parser.addArgument(
    "probe-xml", "p", mitkCommandLineParser::File,
    "Xml definition of the probe", "Specifies the absolute path of the location of the xml definition file of the probe design.", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("normalization-file", "nf", mitkCommandLineParser::File,
    "Input normalization file", "The input normalization file is used for normalization of the number of photons in the PVFC calculations.", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.endGroup();

  // parse arguments, this method returns a mapping of long argument names and their values
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;
  // parse, cast and set required arguments
  inputFilename = us::any_cast<std::string>(parsedArgs["input"]);
  // strip ending
  inputFilename = inputFilename.substr(0, inputFilename.find("_H.mci"));
  inputFilename = inputFilename.substr(0, inputFilename.find("_T.bin"));

  outputFilename = us::any_cast<std::string>(parsedArgs["output"]);
  // add .nrrd if not there
  std::string suffix = ".nrrd";
  if (outputFilename.compare(outputFilename.size() - suffix.size(), suffix.size(), suffix) != 0)
    outputFilename = outputFilename + suffix;

  // default values for optional arguments
  // parse, cast and set optional arguments if given
  if (parsedArgs.count("verbose"))
  {
    verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  if (parsedArgs.count("detector-x"))
  {
    detector_x = us::any_cast<int>(parsedArgs["detector-x"]);
  }
  if (parsedArgs.count("detector-z"))
  {
    detector_z = us::any_cast<int>(parsedArgs["detector-z"]);
  }
  if (parsedArgs.count("timer"))
  {
    requestedSimulationTime = us::any_cast<float>(parsedArgs["timer"]);
    if (requestedSimulationTime > 0) interpretAsTime = true;
  }
  if (parsedArgs.count("y-offset"))
  {
    yOffset = us::any_cast<float>(parsedArgs["y-offset"]);
  }
  if (parsedArgs.count("number-of-photons"))
  {
    requestedNumberOfPhotons = us::any_cast<int>(parsedArgs["number-of-photons"]);
    if (requestedNumberOfPhotons > 0) interpretAsTime = false;
  }
  if (parsedArgs.count("jobs"))
  {
    concurentThreadsSupported = us::any_cast<int>(parsedArgs["jobs"]);
  }
  if (parsedArgs.count("probe-xml"))
  {
    std::string inputXmlProbeDesign = us::any_cast<std::string>(parsedArgs["probe-xml"]);
    m_PhotoacousticProbe = mitk::pa::Probe::New(inputXmlProbeDesign, verbose);
    if (!m_PhotoacousticProbe->IsValid())
    {
      std::cerr << "Xml File was not valid. Simulation failed." << std::endl;
      return EXIT_FAILURE;
    }
  }
  if (parsedArgs.count("normalization-file"))
  {
    normalizationFilename = us::any_cast<std::string>(parsedArgs["normalization-file"]);
  }

  if (concurentThreadsSupported == 0 || concurentThreadsSupported == -1)
  {
    concurentThreadsSupported = std::thread::hardware_concurrency();
    if (concurentThreadsSupported == 0)
    {
      std::cout << "Could not determine number of available threads. Launching only one." << std::endl;
      concurentThreadsSupported = 1;
    }
  }

  if (detector_x != -1 && detector_z != -1)
  {
    if (verbose)
      std::cout << "Performing PVFC calculation for x=" << detector_x << " and z=" << detector_z << std::endl;
    simulatePVFC = true;
  }
  else
  {
    if (verbose)
      std::cout << "Will not perform PVFC calculation due to x=" << detector_x << " and/or z=" << detector_z << std::endl;
  }

  InputValues allInput = InputValues();
  allInput.LoadValues(inputFilename, yOffset, normalizationFilename, simulatePVFC);

  std::vector<ReturnValues> allValues(concurentThreadsSupported);
  auto* threads = new std::thread[concurentThreadsSupported];

  for (long i = 0; i < concurentThreadsSupported; i++)
  {
    auto* tmp = new ReturnValues();
    allValues.push_back(*tmp);
  }

  if (verbose) std::cout << "Initializing MonteCarloThreadHandler" << std::endl;

  long timeMetric;
  if (interpretAsTime)
  {
    if (requestedSimulationTime < mitk::eps)
      requestedSimulationTime = allInput.simulationTimeFromFile;

    timeMetric = requestedSimulationTime * 60 * 1000;
  }
  else
  {
    timeMetric = requestedNumberOfPhotons;
  }

  mitk::pa::MonteCarloThreadHandler::Pointer threadHandler = mitk::pa::MonteCarloThreadHandler::New(timeMetric, interpretAsTime);

  if (simulatePVFC)
    threadHandler->SetPackageSize(1000);

  if (verbose) std::cout << "\nStarting simulation ...\n" << std::endl;

  auto simulationStartTime = std::chrono::system_clock::now();

  for (int i = 0; i < concurentThreadsSupported; i++)
  {
    threads[i] = std::thread(runMonteCarlo, &allInput, &allValues[i], (i + 1), threadHandler);
  }

  for (int i = 0; i < concurentThreadsSupported; i++)
  {
    threads[i].join();
  }

  auto simulationFinishTime = std::chrono::system_clock::now();
  auto simulationTimeElapsed = simulationFinishTime - simulationStartTime;

  if (verbose) std::cout << "\n\nFinished simulation\n\n" << std::endl;
  std::cout << "total time for simulation: "
    << (int)std::chrono::duration_cast<std::chrono::seconds>(simulationTimeElapsed).count() << "sec " << std::endl;

  /**** SAVE
   Convert data to relative fluence rate [cm^-2] and save.
   *****/

  if (!simulatePVFC)
  {
    if (verbose) std::cout << "Allocating memory for normal simulation result ... ";
    auto* finalTotalFluence = (double *)malloc(allInput.totalNumberOfVoxels * sizeof(double));
    if (verbose) std::cout << "[OK]" << std::endl;
    if (verbose) std::cout << "Cleaning memory for normal simulation result ...";
    for (int i = 0; i < allInput.totalNumberOfVoxels; i++) {
      finalTotalFluence[i] = 0;
    }
    if (verbose) std::cout << "[OK]" << std::endl;

    if (verbose) std::cout << "Calculating resulting fluence ... ";
    double tdx = 0, tdy = 0, tdz = 0;
    long long tNphotons = 0;
    for (int t = 0; t < concurentThreadsSupported; t++)
    {
      tdx = allInput.xSpacing;
      tdy = allInput.ySpacing;
      tdz = allInput.zSpacing;
      tNphotons += allValues[t].Nphotons;
      for (int voxelNumber = 0; voxelNumber < allInput.totalNumberOfVoxels; voxelNumber++) {
        finalTotalFluence[voxelNumber] += allValues[t].totalFluence[voxelNumber];
      }
    }
    if (verbose) std::cout << "[OK]" << std::endl;
    std::cout << "total number of photons simulated: "
      << tNphotons << std::endl;

    // Normalize deposition (A) to yield fluence rate (F).
    double temp = tdx*tdy*tdz*tNphotons;
    for (int i = 0; i < allInput.totalNumberOfVoxels; i++) {
      finalTotalFluence[i] /= temp*allInput.muaVector[i];
    }

    if (verbose) std::cout << "Saving normal simulated fluence result to " << outputFilename << " ... ";

    mitk::Image::Pointer resultImage = mitk::Image::New();
    mitk::PixelType TPixel = mitk::MakeScalarPixelType<double>();
    auto* dimensionsOfImage = new unsigned int[3];

    // Copy dimensions
    dimensionsOfImage[0] = allInput.Ny;
    dimensionsOfImage[1] = allInput.Nx;
    dimensionsOfImage[2] = allInput.Nz;

    resultImage->Initialize(TPixel, 3, dimensionsOfImage);

    mitk::Vector3D spacing;
    spacing[0] = allInput.ySpacing;
    spacing[1] = allInput.xSpacing;
    spacing[2] = allInput.zSpacing;
    resultImage->SetSpacing(spacing);
    resultImage->SetImportVolume(finalTotalFluence, 0, 0, mitk::Image::CopyMemory);

    resultImage->GetPropertyList()->SetFloatProperty("y-offset", yOffset);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("y-offset"));

    mitk::IOUtil::Save(resultImage, outputFilename);

    if (verbose) std::cout << "[OK]" << std::endl;

    if (verbose)
    {
      std::cout << "x spacing = " << tdx << std::endl;
      std::cout << "y spacing = " << tdy << std::endl;
      std::cout << "z spacing = " << tdz << std::endl;
      std::cout << "total number of voxels = " << allInput.totalNumberOfVoxels << std::endl;
      std::cout << "number of photons = " << (int)tNphotons << std::endl;
    }
  }
  else // if simulate PVFC
  {
    if (verbose) std::cout << "Allocating memory for PVFC simulation result ... ";
    double* detectorFluence = ((double*)malloc(allInput.totalNumberOfVoxels * sizeof(double)));
    if (verbose) std::cout << "[OK]" << std::endl;
    if (verbose) std::cout << "Cleaning memory for PVFC simulation result ...";
    for (int i = 0; i < allInput.totalNumberOfVoxels; i++) {
      detectorFluence[i] = 0;
    }
    if (verbose) std::cout << "[OK]" << std::endl;

    if (verbose) std::cout << "Calculating resulting PVFC fluence ... ";
    double tdx = 0, tdy = 0, tdz = 0;
    long long tNphotons = 0;
    long pvfcPhotons = 0;
    for (int t = 0; t < concurentThreadsSupported; t++)
    {
      tdx = allInput.xSpacing;
      tdy = allInput.ySpacing;
      tdz = allInput.zSpacing;
      tNphotons += allValues[t].Nphotons;
      pvfcPhotons += allValues[t].detectorVoxel->m_NumberPhotonsCurrent;
      for (int voxelNumber = 0; voxelNumber < allInput.totalNumberOfVoxels; voxelNumber++) {
        detectorFluence[voxelNumber] +=
          allValues[t].detectorVoxel->fluenceContribution[voxelNumber];
      }
    }
    if (verbose) std::cout << "[OK]" << std::endl;
    std::cout << "total number of photons simulated: "
      << tNphotons << std::endl;

    // Normalize deposition (A) to yield fluence rate (F).
    double temp = tdx*tdy*tdz*tNphotons;
    for (int i = 0; i < allInput.totalNumberOfVoxels; i++) {
      detectorFluence[i] /= temp*allInput.muaVector[i];
    }

    if (verbose) std::cout << "Saving PVFC ...";

    std::stringstream detectorname("");
    double detectorX = allValues[0].detectorVoxel->location.x;
    double detectorY = allValues[0].detectorVoxel->location.y;
    double detectorZ = allValues[0].detectorVoxel->location.z;
    detectorname << detectorX << "," << detectorY << ","
      << detectorZ << "FluenceContribution.nrrd";
    // Save the binary file
    std::string outputFileBase = outputFilename.substr(0, outputFilename.find(".nrrd"));
    outputFilename = outputFileBase + "_p" + detectorname.str().c_str();

    mitk::Image::Pointer pvfcImage = mitk::Image::New();
    auto* dimensionsOfPvfcImage = new unsigned int[3];

    // Copy dimensions
    dimensionsOfPvfcImage[0] = allInput.Ny;
    dimensionsOfPvfcImage[1] = allInput.Nx;
    dimensionsOfPvfcImage[2] = allInput.Nz;

    pvfcImage->Initialize(mitk::MakeScalarPixelType<double>(), 3, dimensionsOfPvfcImage);

    mitk::Vector3D pvfcSpacing;
    pvfcSpacing[0] = allInput.ySpacing;
    pvfcSpacing[1] = allInput.xSpacing;
    pvfcSpacing[2] = allInput.zSpacing;
    pvfcImage->SetSpacing(pvfcSpacing);
    pvfcImage->SetImportVolume(detectorFluence, 0, 0, mitk::Image::CopyMemory);

    pvfcImage->GetPropertyList()->SetFloatProperty("detector-x", detectorX);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("detector-x"));
    pvfcImage->GetPropertyList()->SetFloatProperty("detector-y", detectorY);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("detector-y"));
    pvfcImage->GetPropertyList()->SetFloatProperty("detector-z", detectorZ);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("detector-z"));
    pvfcImage->GetPropertyList()->SetFloatProperty("normalization-factor", allValues[0].detectorVoxel->m_PhotonNormalizationValue);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("normalization-factor"));
    pvfcImage->GetPropertyList()->SetFloatProperty("simulated-photons", pvfcPhotons);
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("simulated-photons"));

    mitk::IOUtil::Save(pvfcImage, outputFilename);

    if (verbose) std::cout << "[OK]" << std::endl;

    if (verbose)
    {
      std::cout << "x spacing = " << tdx << std::endl;
      std::cout << "y spacing = " << tdy << std::endl;
      std::cout << "z spacing = " << tdz << std::endl;
      std::cout << "total number of voxels = " << allInput.totalNumberOfVoxels << std::endl;
      std::cout << "number of photons = " << (int)tNphotons << std::endl;
    }
  }

  exit(EXIT_SUCCESS);
} /* end of main */

/* CORE FUNCTION */
void runMonteCarlo(InputValues* inputValues, ReturnValues* returnValue, int thread, mitk::pa::MonteCarloThreadHandler::Pointer threadHandler)
{
  if (verbose) std::cout << "Thread " << thread << ": Locking Mutex ..." << std::endl;
  if (verbose) std::cout << "[OK]" << std::endl;
  if (verbose) std::cout << "Initializing ... ";

  /* Propagation parameters */
  double  x, y, z;        /* photon position */
  double  ux, uy, uz;     /* photon trajectory as cosines */
  double  uxx, uyy, uzz;  /* temporary values used during SPIN */
  double  s;              /* step sizes. s = -log(RND)/mus [cm] */
  double  sleft;          /* dimensionless */
  double  costheta;       /* cos(theta) */
  double  sintheta;       /* sin(theta) */
  double  cospsi;         /* cos(psi) */
  double  sinpsi;         /* sin(psi) */
  double  psi;            /* azimuthal angle */
  long    photonIterator = 0;       /* current photon */
  double  W;              /* photon weight */
  double  absorb;         /* weighted deposited in a step due to absorption */
  short   photon_status;  /* flag = ALIVE=1 or DEAD=0 */
  bool    sv;             /* Are they in the same voxel? */

  /* dummy variables */
  double  rnd;         /* assigned random value 0-1 */
  double  r, phi;      /* dummy values */
  long    i, j;         /* dummy indices */
  double  tempx, tempy, tempz; /* temporary variables, used during photon step. */
  int     ix, iy, iz;  /* Added. Used to track photons */
  double  temp;        /* dummy variable */
  int     bflag;       /* boundary flag:  0 = photon inside volume. 1 = outside volume */
  int     CNT = 0;

  returnValue->totalFluence = (double *)malloc(inputValues->totalNumberOfVoxels * sizeof(double));  /* relative fluence rate [W/cm^2/W.delivered] */

  if (detector_x != -1 && detector_z != -1)
  {
    if (detector_x<0 || detector_x>inputValues->Nx)
    {
      std::cout << "Requested detector x position not valid. Needs to be >= 0 and <= " << inputValues->Nx << std::endl;
      exit(EXIT_FAILURE);
    }
    if (detector_z<1 || detector_z>inputValues->Nz)
    {
      std::cout << "Requested detector z position not valid. Needs to be > 0 and <= " << inputValues->Nz << std::endl;
      exit(EXIT_FAILURE);
    }

    double photonNormalizationValue = 1 / inputValues->GetNormalizationValue(detector_x, inputValues->Ny / 2, detector_z);
    returnValue->detectorVoxel = new DetectorVoxel(initLocation(detector_x, inputValues->Ny / 2, detector_z, 0), inputValues->totalNumberOfVoxels, photonNormalizationValue);
  }

  /**** ======================== MAJOR CYCLE ============================ *****/

  auto duration = std::chrono::system_clock::now().time_since_epoch();
  returnValue->RandomGen(0, (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() + thread) % 32000, nullptr); /* initiate with seed = 1, or any long integer. */
  for (j = 0; j < inputValues->totalNumberOfVoxels; j++) returnValue->totalFluence[j] = 0; // ensure F[] starts empty.

  /**** RUN Launch N photons, initializing each one before progation. *****/

  long photonsToSimulate = 0;

  do {
    photonsToSimulate = threadHandler->GetNextWorkPackage();
    if (returnValue->detectorVoxel != nullptr)
    {
      photonsToSimulate = photonsToSimulate * returnValue->detectorVoxel->m_PhotonNormalizationValue;
    }

    if (verbose)
      MITK_INFO << "Photons to simulate: " << photonsToSimulate;

    photonIterator = 0L;

    do {
      /**** LAUNCH Initialize photon position and trajectory. *****/

      photonIterator += 1;        /* increment photon count */
      W = 1.0;                    /* set photon weight to one */
      photon_status = ALIVE;      /* Launch an ALIVE photon */
      CNT = 0;

      /**** SET SOURCE* Launch collimated beam at x,y center.****/
      /****************************/
      /* Initial position. */

      if (m_PhotoacousticProbe.IsNotNull())
      {
        double rnd1 = -1;
        double rnd2 = -1;
        double rnd3 = -1;
        double rnd4 = -1;
        double rnd5 = -1;
        double rnd6 = -1;
        double rnd7 = -1;
        double rnd8 = -1;

        while ((rnd1 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd2 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd3 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd4 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd5 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd6 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd7 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        while ((rnd8 = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

        mitk::pa::LightSource::PhotonInformation info = m_PhotoacousticProbe->GetNextPhoton(rnd1, rnd2, rnd3, rnd4, rnd5, rnd6, rnd7, rnd8);
        x = info.xPosition;
        y = yOffset + info.yPosition;
        z = info.zPosition;
        ux = info.xAngle;
        uy = info.yAngle;
        uz = info.zAngle;
        if (verbose)
          std::cout << "Created photon at position (" << x << "|" << y << "|" << z << ") with angles (" << ux << "|" << uy << "|" << uz << ")." << std::endl;
      }
      else
      {
        /* trajectory */
        if (inputValues->launchflag == 1) // manually set launch
        {
          x = inputValues->xs;
          y = inputValues->ys;
          z = inputValues->zs;
          ux = inputValues->ux0;
          uy = inputValues->uy0;
          uz = inputValues->uz0;
        }
        else // use mcflag
        {
          if (inputValues->mcflag == 0) // uniform beam
          {
            // set launch point and width of beam
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0); // avoids rnd = 0
            r = inputValues->radius*sqrt(rnd); // radius of beam at launch point
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0); // avoids rnd = 0
            phi = rnd*2.0*PI;
            x = inputValues->xs + r*cos(phi);
            y = inputValues->ys + r*sin(phi);
            z = inputValues->zs;
            // set trajectory toward focus
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0); // avoids rnd = 0
            r = inputValues->waist*sqrt(rnd); // radius of beam at focus
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0); // avoids rnd = 0
            phi = rnd*2.0*PI;

            // !!!!!!!!!!!!!!!!!!!!!!! setting input values will braek

            inputValues->xfocus = r*cos(phi);
            inputValues->yfocus = r*sin(phi);
            temp = sqrt((x - inputValues->xfocus)*(x - inputValues->xfocus)
              + (y - inputValues->yfocus)*(y - inputValues->yfocus) + inputValues->zfocus*inputValues->zfocus);
            ux = -(x - inputValues->xfocus) / temp;
            uy = -(y - inputValues->yfocus) / temp;
            uz = sqrt(1 - ux*ux + uy*uy);
          }
          else if (inputValues->mcflag == 5) // Multispectral DKFZ prototype
          {
            // set launch point and width of beam
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            //offset in x direction in cm (random)
            x = (rnd*2.5) - 1.25;

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
            double b = ((rnd)-0.5);
            y = (b > 0 ? yOffset + 1.5 : yOffset - 1.5);
            z = 0.1;
            ux = 0;

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            //Angle of beam in y direction
            uy = sin((rnd*0.42) - 0.21 + (b < 0 ? 1.0 : -1.0) * 0.436);

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            // angle of beam in x direction
            ux = sin((rnd*0.42) - 0.21);
            uz = sqrt(1 - ux*ux - uy*uy);
          }
          else if (inputValues->mcflag == 4) // Monospectral prototype DKFZ
          {
            // set launch point and width of beam
            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            //offset in x direction in cm (random)
            x = (rnd*2.5) - 1.25;

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
            double b = ((rnd)-0.5);
            y = (b > 0 ? yOffset + 0.83 : yOffset - 0.83);
            z = 0.1;
            ux = 0;

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            //Angle of beam in y direction
            uy = sin((rnd*0.42) - 0.21 + (b < 0 ? 1.0 : -1.0) * 0.375);

            while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);

            // angle of beam in x direction
            ux = sin((rnd*0.42) - 0.21);
            uz = sqrt(1 - ux*ux - uy*uy);
          }
          else { // isotropic pt source
            costheta = 1.0 - 2.0 * returnValue->RandomGen(1, 0, nullptr);
            sintheta = sqrt(1.0 - costheta*costheta);
            psi = 2.0 * PI * returnValue->RandomGen(1, 0, nullptr);
            cospsi = cos(psi);
            if (psi < PI)
              sinpsi = sqrt(1.0 - cospsi*cospsi);
            else
              sinpsi = -sqrt(1.0 - cospsi*cospsi);
            x = inputValues->xs;
            y = inputValues->ys;
            z = inputValues->zs;
            ux = sintheta*cospsi;
            uy = sintheta*sinpsi;
            uz = costheta;
          }
        } // end  use mcflag
      }
      /****************************/

      /* Get tissue voxel properties of launchpoint.
     * If photon beyond outer edge of defined voxels,
     * the tissue equals properties of outermost voxels.
     * Therefore, set outermost voxels to infinite background value.
     */
      ix = (int)(inputValues->Nx / 2 + x / inputValues->xSpacing);
      iy = (int)(inputValues->Ny / 2 + y / inputValues->ySpacing);
      iz = (int)(z / inputValues->zSpacing);
      if (ix >= inputValues->Nx) ix = inputValues->Nx - 1;
      if (iy >= inputValues->Ny) iy = inputValues->Ny - 1;
      if (iz >= inputValues->Nz) iz = inputValues->Nz - 1;
      if (ix < 0)   ix = 0;
      if (iy < 0)   iy = 0;
      if (iz < 0)   iz = 0;
      /* Get the tissue type of located voxel */
      i = (long)(iz*inputValues->Ny*inputValues->Nx + ix*inputValues->Ny + iy);

      bflag = 1; // initialize as 1 = inside volume, but later check as photon propagates.

      if (returnValue->detectorVoxel != nullptr)
        returnValue->detectorVoxel->recordedPhotonRoute->clear();

      /* HOP_DROP_SPIN_CHECK
    Propagate one photon until it dies as determined by ROULETTE.
    *******/
      do {
        /**** HOP
      Take step to new position
      s = dimensionless stepsize
      x, uy, uz are cosines of current photon trajectory
      *****/
        while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);   /* yields 0 < rnd <= 1 */
        sleft = -log(rnd);        /* dimensionless step */
        CNT += 1;

        do {  // while sleft>0
          s = sleft / inputValues->musVector[i];        /* Step size [cm].*/
          tempx = x + s*ux;        /* Update positions. [cm] */
          tempy = y + s*uy;
          tempz = z + s*uz;

          sv = returnValue->SameVoxel(x, y, z, tempx, tempy, tempz, inputValues->xSpacing, inputValues->ySpacing, inputValues->zSpacing);
          if (sv) /* photon in same voxel */
          {
            x = tempx;  /* Update positions. */
            y = tempy;
            z = tempz;

            /**** DROP
          Drop photon weight (W) into local bin.
          *****/

            absorb = W*(1 - exp(-inputValues->muaVector[i] * s));  /* photon weight absorbed at this step */
            W -= absorb;          /* decrement WEIGHT by amount absorbed */
            // If photon within volume of heterogeneity, deposit energy in F[].
            // Normalize F[] later, when save output.
            if (bflag)
            {
              i = (long)(iz*inputValues->Ny*inputValues->Nx + ix*inputValues->Ny + iy);
              returnValue->totalFluence[i] += absorb;
              // only save data if blag==1, i.e., photon inside simulation cube

              //For each detectorvoxel
              if (returnValue->detectorVoxel != nullptr)
              {
                //Add photon position to the recorded photon route
                returnValue->detectorVoxel->recordedPhotonRoute->push_back(initLocation(ix, iy, iz, absorb));

                //If the photon is currently at the detector position
                if ((returnValue->detectorVoxel->location.x == ix)
                  && ((returnValue->detectorVoxel->location.y == iy)
                    || (returnValue->detectorVoxel->location.y - 1 == iy))
                  && (returnValue->detectorVoxel->location.z == iz))
                {
                  //For each voxel in the recorded photon route
                  for (unsigned int routeIndex = 0; routeIndex < returnValue->detectorVoxel->recordedPhotonRoute->size(); routeIndex++)
                  {
                    //increment the fluence contribution at that particular position
                    i = (long)(returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).z*inputValues->Ny*inputValues->Nx
                      + returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).x*inputValues->Ny
                      + returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).y);
                    returnValue->detectorVoxel->fluenceContribution[i] += returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).absorb;
                  }

                  //Clear the recorded photon route
                  returnValue->detectorVoxel->m_NumberPhotonsCurrent++;
                  returnValue->detectorVoxel->recordedPhotonRoute->clear();
                }
              }
            }

            /* Update sleft */
            sleft = 0;    /* dimensionless step remaining */
          }
          else /* photon has crossed voxel boundary */
          {
            /* step to voxel face + "littlest step" so just inside new voxel. */
            s = ls + returnValue->FindVoxelFace2(x, y, z, tempx, tempy, tempz, inputValues->xSpacing, inputValues->ySpacing, inputValues->zSpacing, ux, uy, uz);

            /**** DROP
          Drop photon weight (W) into local bin.
          *****/
            absorb = W*(1 - exp(-inputValues->muaVector[i] * s));   /* photon weight absorbed at this step */
            W -= absorb;                  /* decrement WEIGHT by amount absorbed */
            // If photon within volume of heterogeneity, deposit energy in F[].
            // Normalize F[] later, when save output.
            if (bflag)
            {
              // only save data if bflag==1, i.e., photon inside simulation cube

              //For each detectorvoxel
              if (returnValue->detectorVoxel != nullptr)
              {
                //Add photon position to the recorded photon route
                returnValue->detectorVoxel->recordedPhotonRoute->push_back(initLocation(ix, iy, iz, absorb));

                //If the photon is currently at the detector position
                if ((returnValue->detectorVoxel->location.x == ix)
                  && ((returnValue->detectorVoxel->location.y == iy)
                    || (returnValue->detectorVoxel->location.y - 1 == iy))
                  && (returnValue->detectorVoxel->location.z == iz))
                {
                  //For each voxel in the recorded photon route
                  for (unsigned int routeIndex = 0; routeIndex < returnValue->detectorVoxel->recordedPhotonRoute->size(); routeIndex++)
                  {
                    //increment the fluence contribution at that particular position
                    i = (long)(returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).z*inputValues->Ny*inputValues->Nx
                      + returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).x*inputValues->Ny
                      + returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).y);
                    returnValue->detectorVoxel->fluenceContribution[i] += returnValue->detectorVoxel->recordedPhotonRoute->at(routeIndex).absorb;
                  }

                  //Clear the recorded photon route
                  returnValue->detectorVoxel->m_NumberPhotonsCurrent++;
                  returnValue->detectorVoxel->recordedPhotonRoute->clear();
                }
              }

              i = (long)(iz*inputValues->Ny*inputValues->Nx + ix*inputValues->Ny + iy);
              returnValue->totalFluence[i] += absorb;
            }

            /* Update sleft */
            sleft -= s*inputValues->musVector[i];  /* dimensionless step remaining */
            if (sleft <= ls) sleft = 0;

            /* Update positions. */
            x += s*ux;
            y += s*uy;
            z += s*uz;

            // pointers to voxel containing optical properties
            ix = (int)(inputValues->Nx / 2 + x / inputValues->xSpacing);
            iy = (int)(inputValues->Ny / 2 + y / inputValues->ySpacing);
            iz = (int)(z / inputValues->zSpacing);

            bflag = 1;  // Boundary flag. Initialize as 1 = inside volume, then check.
            if (inputValues->boundaryflag == 0) { // Infinite medium.
              // Check if photon has wandered outside volume.
              // If so, set tissue type to boundary value, but let photon wander.
              // Set blag to zero, so DROP does not deposit energy.
              if (iz >= inputValues->Nz) { iz = inputValues->Nz - 1; bflag = 0; }
              if (ix >= inputValues->Nx) { ix = inputValues->Nx - 1; bflag = 0; }
              if (iy >= inputValues->Ny) { iy = inputValues->Ny - 1; bflag = 0; }
              if (iz < 0) { iz = 0;    bflag = 0; }
              if (ix < 0) { ix = 0;    bflag = 0; }
              if (iy < 0) { iy = 0;    bflag = 0; }
            }
            else if (inputValues->boundaryflag == 1) { // Escape at boundaries
              if (iz >= inputValues->Nz) { iz = inputValues->Nz - 1; photon_status = DEAD; sleft = 0; }
              if (ix >= inputValues->Nx) { ix = inputValues->Nx - 1; photon_status = DEAD; sleft = 0; }
              if (iy >= inputValues->Ny) { iy = inputValues->Ny - 1; photon_status = DEAD; sleft = 0; }
              if (iz < 0) { iz = 0;    photon_status = DEAD; sleft = 0; }
              if (ix < 0) { ix = 0;    photon_status = DEAD; sleft = 0; }
              if (iy < 0) { iy = 0;    photon_status = DEAD; sleft = 0; }
            }
            else if (inputValues->boundaryflag == 2) { // Escape at top surface, no x,y bottom z boundaries
              if (iz >= inputValues->Nz) { iz = inputValues->Nz - 1; bflag = 0; }
              if (ix >= inputValues->Nx) { ix = inputValues->Nx - 1; bflag = 0; }
              if (iy >= inputValues->Ny) { iy = inputValues->Ny - 1; bflag = 0; }
              if (iz < 0) { iz = 0;    photon_status = DEAD; sleft = 0; }
              if (ix < 0) { ix = 0;    bflag = 0; }
              if (iy < 0) { iy = 0;    bflag = 0; }
            }

            // update pointer to tissue type
            i = (long)(iz*inputValues->Ny*inputValues->Nx + ix*inputValues->Ny + iy);
          } //(sv) /* same voxel */
        } while (sleft > 0); //do...while

        /**** SPIN
       Scatter photon into new trajectory defined by theta and psi.
       Theta is specified by cos(theta), which is determined
       based on the Henyey-Greenstein scattering function.
       Convert theta and psi into cosines ux, uy, uz.
       *****/
       /* Sample for costheta */
        while ((rnd = returnValue->RandomGen(1, 0, nullptr)) <= 0.0);
        if (inputValues->gVector[i] == 0.0)
        {
          costheta = 2.0 * rnd - 1.0;
        }
        else
        {
          double temp = (1.0 - inputValues->gVector[i] * inputValues->gVector[i])
            / (1.0 - inputValues->gVector[i] + 2 * inputValues->gVector[i] * rnd);
          costheta = (1.0 + inputValues->gVector[i] * inputValues->gVector[i] - temp*temp) / (2.0*inputValues->gVector[i]);
        }
        sintheta = sqrt(1.0 - costheta*costheta); /* sqrt() is faster than sin(). */

        /* Sample psi. */
        psi = 2.0*PI*returnValue->RandomGen(1, 0, nullptr);
        cospsi = cos(psi);
        if (psi < PI)
          sinpsi = sqrt(1.0 - cospsi*cospsi);     /* sqrt() is faster than sin(). */
        else
          sinpsi = -sqrt(1.0 - cospsi*cospsi);

        /* New trajectory. */
        if (1 - fabs(uz) <= ONE_MINUS_COSZERO) {      /* close to perpendicular. */
          uxx = sintheta * cospsi;
          uyy = sintheta * sinpsi;
          uzz = costheta * SIGN(uz);   /* SIGN() is faster than division. */
        }
        else {          /* usually use this option */
          temp = sqrt(1.0 - uz * uz);
          uxx = sintheta * (ux * uz * cospsi - uy * sinpsi) / temp + ux * costheta;
          uyy = sintheta * (uy * uz * cospsi + ux * sinpsi) / temp + uy * costheta;
          uzz = -sintheta * cospsi * temp + uz * costheta;
        }

        /* Update trajectory */
        ux = uxx;
        uy = uyy;
        uz = uzz;

        /**** CHECK ROULETTE
      If photon weight below THRESHOLD, then terminate photon using Roulette technique.
      Photon has CHANCE probability of having its weight increased by factor of 1/CHANCE,
      and 1-CHANCE probability of terminating.
      *****/
        if (W < THRESHOLD) {
          if (returnValue->RandomGen(1, 0, nullptr) <= CHANCE)
            W /= CHANCE;
          else photon_status = DEAD;
        }
      } while (photon_status == ALIVE);  /* end STEP_CHECK_HOP_SPIN */
      /* if ALIVE, continue propagating */
      /* If photon DEAD, then launch new photon. */
    } while (photonIterator < photonsToSimulate);  /* end RUN */

    returnValue->Nphotons += photonsToSimulate;
  } while (photonsToSimulate > 0);

  if (verbose) std::cout << "------------------------------------------------------" << std::endl;
  if (verbose) std::cout << "Thread " << thread << " is finished." << std::endl;
}
