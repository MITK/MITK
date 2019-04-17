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

//#ifndef __itkRandomPhantomFilter_cpp
//#define __itkRandomPhantomFilter_cpp

#include "itkRandomPhantomFilter.h"

#include <itkFibersFromPlanarFiguresFilter.h>
#include <cmath>

namespace itk{

RandomPhantomFilter::RandomPhantomFilter()
  : m_NumTracts(1)
  , m_MinStreamlineDensity(25)
  , m_MaxStreamlineDensity(200)
  , m_StartRadiusMin(5)
  , m_StartRadiusMax(30)
  , m_CurvynessMin(10)
  , m_CurvynessMax(45)
  , m_StepSizeMin(15)
  , m_StepSizeMax(30)
  , m_MinTwist(10)
  , m_MaxTwist(30)
  , m_FixSeed(-1)
{
  m_VolumeSize[0] = 500;
  m_VolumeSize[1] = 500;
  m_VolumeSize[2] = 500;
}

RandomPhantomFilter::~RandomPhantomFilter()
{

}


void RandomPhantomFilter::TransformPlanarFigure(mitk::PlanarEllipse* pe, mitk::Vector3D translation, mitk::Vector3D rotation, double twistangle, double radius1, double radius2)
{
  mitk::BaseGeometry* geom = pe->GetGeometry();

  // translate
  geom->Translate(translation);

  // calculate rotation matrix
  double x = rotation[0]*itk::Math::pi/180;
  double y = rotation[1]*itk::Math::pi/180;
  double z = rotation[2]*itk::Math::pi/180;

  itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
  rotX[1][1] = cos(x);
  rotX[2][2] = rotX[1][1];
  rotX[1][2] = -sin(x);
  rotX[2][1] = -rotX[1][2];

  itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
  rotY[0][0] = cos(y);
  rotY[2][2] = rotY[0][0];
  rotY[0][2] = sin(y);
  rotY[2][0] = -rotY[0][2];

  itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
  rotZ[0][0] = cos(z);
  rotZ[1][1] = rotZ[0][0];
  rotZ[0][1] = -sin(z);
  rotZ[1][0] = -rotZ[0][1];

  itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

  // rotate fiducial
  geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());

  // adjust control points
  auto p0 = pe->GetControlPoint(0);
  auto p1 = pe->GetControlPoint(1);
  auto p2 = pe->GetControlPoint(2);
  auto p3 = pe->GetControlPoint(3);

  auto v1 = p1 - p0;
  auto v2 = p2 - p0;
  auto dot = std::cos(itk::Math::pi*twistangle/180.0);

  vnl_matrix_fixed<double, 2, 2> tRot;
  tRot[0][0] = dot;
  tRot[1][1] = tRot[0][0];
  tRot[1][0] = sin(acos(tRot[0][0]));
  tRot[0][1] = -tRot[1][0];
  if (twistangle<0)
    tRot = tRot.transpose();
  vnl_vector_fixed<double, 2> vt; vt[0]=1; vt[1]=0;
  vnl_vector_fixed<double, 2> v3 = tRot*vt;

  v1.Normalize();
  v2.Normalize();

  p1 = p0 + radius1;
  p2 = p0 + radius2;

  p3 = p0 + mitk::Vector2D(v3);
  pe->SetControlPoint(1, p1);
  pe->SetControlPoint(2, p2);
  pe->SetControlPoint(3, p3);


  pe->Modified();
}

mitk::PlanarEllipse::Pointer RandomPhantomFilter::CreatePlanarFigure()
{
  mitk::PlaneGeometry::Pointer pl = mitk::PlaneGeometry::New();
  pl->SetIdentity();
  //  mitk::Point3D o; o.Fill(10.0);
  //  pl->SetOrigin(o);
  mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();
  figure->ResetNumberOfControlPoints(0);
  figure->SetPlaneGeometry(pl);

  mitk::Point2D p0; p0.Fill(0.0);
  mitk::Point2D p1; p1.Fill(0.0); p1[0] = 1;
  mitk::Point2D p2; p2.Fill(0.0); p2[1] = 1;

  figure->PlaceFigure(p0);
  figure->AddControlPoint(p0);
  figure->AddControlPoint(p1);
  figure->AddControlPoint(p2);
  figure->AddControlPoint(p2);
  figure->SetProperty("initiallyplaced", mitk::BoolProperty::New(true));
  return figure;
}

void RandomPhantomFilter::SetNumTracts(unsigned int NumTracts)
{
  m_NumTracts = NumTracts;
}

void RandomPhantomFilter::GetPfOnBoundingPlane(mitk::Vector3D& pos, mitk::Vector3D& rot)
{
  auto plane = randGen->GetIntegerVariate(5) + 1;
  MITK_INFO << "Plane: " << plane;
  switch(plane)
  {
  case 1:
  {
    pos[0] = randGen->GetUniformVariate(0, m_VolumeSize[0]);
    pos[1] = randGen->GetUniformVariate(0, m_VolumeSize[1]);
    pos[2] = 0;

    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0;
    break;
  }
  case 2:
  {
    pos[0] = 0;
    pos[1] = randGen->GetUniformVariate(0, m_VolumeSize[1]);
    pos[2] = randGen->GetUniformVariate(0, m_VolumeSize[2]);

    rot[0] = 0;
    rot[1] = 90;
    rot[2] = 0;
    break;
  }
  case 3:
  {
    pos[0] = randGen->GetUniformVariate(0, m_VolumeSize[0]);
    pos[1] = 0;
    pos[2] = randGen->GetUniformVariate(0, m_VolumeSize[2]);

    rot[0] = -90;
    rot[1] = 0;
    rot[2] = 0;
    break;
  }
  case 4:
  {
    pos[0] = randGen->GetUniformVariate(0, m_VolumeSize[0]);
    pos[1] = m_VolumeSize[1];
    pos[2] = randGen->GetUniformVariate(0, m_VolumeSize[2]);

    rot[0] = 90;
    rot[1] = 0;
    rot[2] = 0;
    break;
  }
  case 5:
  {
    pos[0] = m_VolumeSize[0];
    pos[1] = randGen->GetUniformVariate(0, m_VolumeSize[1]);
    pos[2] = randGen->GetUniformVariate(0, m_VolumeSize[2]);

    rot[0] = 0;
    rot[1] = -90;
    rot[2] = 0;
    break;
  }
  case 6:
  {
    pos[0] = randGen->GetUniformVariate(0, m_VolumeSize[0]);
    pos[1] = randGen->GetUniformVariate(0, m_VolumeSize[1]);
    pos[2] = m_VolumeSize[2];

    rot[0] = 180;
    rot[1] = 0;
    rot[2] = 0;
    break;
  }
  }
}

bool RandomPhantomFilter::IsInVolume(mitk::Vector3D pos)
{
  if (pos[0]>=0 &&
      pos[0]<=m_VolumeSize[0] &&
      pos[1]>=0 &&
      pos[1]<=m_VolumeSize[1] &&
      pos[2]>=0 &&
      pos[2]<=m_VolumeSize[2])
    return true;
  return false;
}

void RandomPhantomFilter::SetFixSeed(int FixSeed)
{
  m_FixSeed = FixSeed;
}

void RandomPhantomFilter::SetMinTwist(unsigned int MinTwist)
{
  m_MinTwist = MinTwist;
}

void RandomPhantomFilter::SetMaxStreamlineDensity(unsigned int MaxStreamlineDensity)
{
  m_MaxStreamlineDensity = MaxStreamlineDensity;
}

void RandomPhantomFilter::SetMinStreamlineDensity(unsigned int MinStreamlinDensity)
{
  m_MinStreamlineDensity = MinStreamlinDensity;
}

void RandomPhantomFilter::SetMaxTwist(unsigned int MaxTwist)
{
  m_MaxTwist = MaxTwist;
}

void RandomPhantomFilter::SetVolumeSize(const mitk::Vector3D &VolumeSize)
{
  m_VolumeSize = VolumeSize;
}

void RandomPhantomFilter::SetStepSizeMax(unsigned int StepSizeMax)
{
  m_StepSizeMax = StepSizeMax;
}

void RandomPhantomFilter::SetStepSizeMin(unsigned int StepSizeMin)
{
  m_StepSizeMin = StepSizeMin;
}

void RandomPhantomFilter::SetCurvynessMax(unsigned int CurvynessMax)
{
  m_CurvynessMax = CurvynessMax;
}

void RandomPhantomFilter::SetCurvynessMin(unsigned int CurvynessMin)
{
  m_CurvynessMin = CurvynessMin;
}

void RandomPhantomFilter::SetStartRadiusMax(unsigned int StartRadiusMax)
{
  m_StartRadiusMax = StartRadiusMax;
}

void RandomPhantomFilter::SetStartRadiusMin(unsigned int StartRadiusMin)
{
  m_StartRadiusMin = StartRadiusMin;
}

void RandomPhantomFilter::GenerateData()
{
  randGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
  if (m_FixSeed>=0)
    randGen->SetSeed(m_FixSeed);
  else
    randGen->SetSeed();

  for (unsigned int i=0; i<m_NumTracts; ++i)
  {
    MITK_INFO << "Generating Bundle_" << i+1;
    FiberGenerationParameters       fiber_params;
    std::vector< mitk::PlanarEllipse::Pointer > bundle_waypoints;

    double curvyness = randGen->GetUniformVariate(m_CurvynessMin, m_CurvynessMax);
    int twistdir = static_cast<int>(randGen->GetIntegerVariate(2)) - 1;
    double dtwist = randGen->GetUniformVariate(m_MinTwist, m_MaxTwist);

    mitk::Vector3D pos; pos.Fill(0.0);
    mitk::Vector3D rot; rot.Fill(0.0);
    double twist = 0;
    double radius1 = randGen->GetUniformVariate(m_StartRadiusMin, m_StartRadiusMax);
    double radius2 = randGen->GetUniformVariate(m_StartRadiusMin, m_StartRadiusMax);

    GetPfOnBoundingPlane(pos, rot);
    rot[0] += randGen->GetUniformVariate(-curvyness, curvyness);
    rot[1] += randGen->GetUniformVariate(-curvyness, curvyness);
    rot[2] += randGen->GetUniformVariate(-curvyness, curvyness);

    mitk::PlanarEllipse::Pointer start = CreatePlanarFigure();
    TransformPlanarFigure(start, pos, rot, twist, radius1, radius2);
    bundle_waypoints.push_back(start);

    double c_area = itk::Math::pi*radius1*radius2;

    int sizestrategy = static_cast<int>(randGen->GetIntegerVariate(2)) - 1;

    MITK_INFO << "Twist: " << dtwist;
    MITK_INFO << "Twist direction: " << twistdir;
    MITK_INFO << "Curvyness: " << curvyness;
    MITK_INFO << "Size strategy: " << sizestrategy;

    int c = 1;
    while(IsInVolume(pos))
    {
      pos += bundle_waypoints.at(c-1)->GetPlaneGeometry()->GetNormal() * randGen->GetUniformVariate(m_StepSizeMin, m_StepSizeMax);
      rot[0] += randGen->GetUniformVariate(-curvyness, curvyness);
      rot[1] += randGen->GetUniformVariate(-curvyness, curvyness);
      rot[2] += randGen->GetUniformVariate(-curvyness, curvyness);

      twist += dtwist * twistdir;
      if (randGen->GetUniformVariate(0.0, 1.0) < 0.25)
      {
        int temp = static_cast<int>(randGen->GetIntegerVariate(2)) - 1;
        if (temp!=twistdir)
        {
          twistdir = temp;
          MITK_INFO << "Twist direction change: " << twistdir;
        }
      }

      if (randGen->GetUniformVariate(0.0, 1.0) < 0.25)
      {
        int temp = static_cast<int>(randGen->GetIntegerVariate(2)) - 1;
        if (temp!=sizestrategy)
        {
          sizestrategy = temp;
          MITK_INFO << "Size strategy change: " << sizestrategy;
        }
      }

      double minradius = 0.5*static_cast<double>(m_StartRadiusMin);
      double dsize = 0.5*minradius;
      double maxradius = 2.0*static_cast<double>(m_StartRadiusMax);
      if (sizestrategy==0)
      {
        radius1 += randGen->GetUniformVariate(-dsize, dsize);
        radius2 += randGen->GetUniformVariate(-dsize, dsize);
        while (radius1 < 5)
          radius1 += randGen->GetUniformVariate(-dsize, dsize);
        while (radius2 < 5)
          radius2 += randGen->GetUniformVariate(-dsize, dsize);
      }
      else if (sizestrategy==1)
      {
        radius1 += randGen->GetUniformVariate(0, dsize);
        radius2 += randGen->GetUniformVariate(0, dsize);
        if (radius1 > maxradius)
        {
          radius1 = maxradius;
          sizestrategy = static_cast<int>(randGen->GetIntegerVariate(1)) - 1;
        }
        if (radius2 > maxradius)
        {
          radius2 = maxradius;
          sizestrategy = static_cast<int>(randGen->GetIntegerVariate(1)) - 1;
        }
      }
      else if (sizestrategy==-1)
      {
        radius1 += randGen->GetUniformVariate(-dsize, 0);
        radius2 += randGen->GetUniformVariate(-dsize, 0);
        if (radius1 < minradius)
        {
          radius1 = minradius;
          sizestrategy = static_cast<int>(randGen->GetIntegerVariate(1));
        }
        if (radius2 < minradius)
        {
          radius2 = minradius;
          sizestrategy = static_cast<int>(randGen->GetIntegerVariate(1));
        }
      }

      c_area += itk::Math::pi*radius1*radius2;

      mitk::PlanarEllipse::Pointer pf = CreatePlanarFigure();
      TransformPlanarFigure(pf, pos, rot, twist, radius1, radius2);

      bundle_waypoints.push_back(pf);
      ++c;
    }
    c_area /= c;
    c_area /= 100;
    MITK_INFO << "Average crossectional area: " << c_area << "cm²";

    fiber_params.m_Fiducials.push_back(bundle_waypoints);
    auto density = randGen->GetUniformVariate(m_MinStreamlineDensity, m_MaxStreamlineDensity);
    MITK_INFO << "Density: " << density;
    MITK_INFO << "Num. fibers: " << fiber_params.m_Density;

    fiber_params.m_Density = static_cast<unsigned int>(std::ceil(c_area*density));
    if (randGen->GetIntegerVariate(1)==0)
    {
      fiber_params.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
      MITK_INFO << "Distribution: uniform";
    }
    else
    {
      fiber_params.m_Distribution = FiberGenerationParameters::DISTRIBUTE_GAUSSIAN;
      MITK_INFO << "Distribution: Gaussian";
    }

    MITK_INFO << "Num. fiducials: " << c;
    MITK_INFO << "------------\n";

    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());       // <-- redirect
    itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
    filter->SetParameters(fiber_params);
    filter->SetFixSeed(m_FixSeed);
    filter->Update();
    m_FiberBundles.push_back(filter->GetFiberBundles().at(0));
    std::cout.rdbuf (old);              // <-- restore
  }
}

}
//#endif // __itkRandomPhantomFilter_cpp

