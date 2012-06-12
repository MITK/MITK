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
#ifndef _BUILDFIBRES
#define _BUILDFIBRES


#include <MitkDiffusionImagingExports.h>
#include <itkOrientationDistributionFunction.h>

#include "mitkParticleGrid.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCleanPolyData.h>

#include <itkVector.h>
#include <itkImage.h>

using namespace std;

namespace mitk
{

class MitkDiffusionImaging_EXPORT FiberBuilder
{
public:
  Particle *particles;
  int pcnt;
  int attrcnt;
  typedef vector< Particle* > ParticleContainerType;
  typedef vector< ParticleContainerType* > FiberContainerType;

  vtkSmartPointer<vtkCellArray> m_VtkCellArray;
  vtkSmartPointer<vtkPoints>    m_VtkPoints;

  typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
  typedef itk::Image<OdfVectorType, 3>        ItkQBallImgType;
  ItkQBallImgType::Pointer                    m_ItkQBallImage;
  float m_FiberLength;
  itk::Point<float> m_LastPoint;

  FiberBuilder(float *points, int numPoints, double spacing[], ItkQBallImgType::Pointer image);

  ~FiberBuilder();

  vtkSmartPointer<vtkPolyData> iterate(int minFiberLength);

  void AddPoint(Particle *dp, vtkSmartPointer<vtkPolyLine> container);

  void labelPredecessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container);

  void labelSuccessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container);
};

}

#endif
