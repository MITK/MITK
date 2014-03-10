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

#ifndef __ANISOTROPICITERATIVECLOSESTPOINTREGISTRATION_H__
#define __ANISOTROPICITERATIVECLOSESTPOINTREGISTRATION_H__

//MITK
#include <mitkCommon.h>
#include <mitkVector.h>

//EXPORTS
#include "MitkSurfaceRegistrationExports.h"

// STL
#include <vector>

// ITK
#include <itkMatrix.h>

//#include <mitkSurface.h>

namespace mitk
{

class Surface;
class WeightedPointTransform;

class MitkSurfaceRegistration_EXPORT AnisotropicIterativeClosestPointRegistration : public itk::Object
{

protected:

  typedef itk::Matrix < double, 3, 3 > CovarianceMatrix;
  typedef std::vector< CovarianceMatrix > CovarianceMatrixList;
  typedef mitk::Vector3D Translation;
  typedef CovarianceMatrix Rotation;

  AnisotropicIterativeClosestPointRegistration();
  ~AnisotropicIterativeClosestPointRegistration();

  unsigned int m_MaxIterations;

  double m_Threshold;

  double m_FRENormalizationFactor;

  double m_SearchRadius;

  double m_MaxIterationsInWeightedPointTransform;

  double m_FRE;

  double m_TrimmFactor;

  unsigned int m_NumberOfIterations;

  itk::SmartPointer < Surface > m_MovingSurface;
  itk::SmartPointer < Surface > m_FixedSurface;

  itk::SmartPointer < WeightedPointTransform > m_WeightedPointTransform;

  CovarianceMatrixList m_CovarianceMatricesMovingSurface;
  CovarianceMatrixList m_CovarianceMatricesFixedSurface;

  Translation m_Translation;
  Rotation    m_Rotation;

public:

  itkSetMacro(MaxIterations, unsigned int)

  itkSetMacro(Threshold, double)

  itkSetMacro(FRENormalizationFactor, double)

  itkSetMacro(SearchRadius, double)

  itkSetMacro(MaxIterationsInWeightedPointTransform, double)

  itkGetMacro(FRE,double)

  itkGetMacro(NumberOfIterations, unsigned int)

  itkSetMacro(TrimmFactor,double)

  itkSetMacro(MovingSurface,itk::SmartPointer<Surface>)

  itkSetMacro(FixedSurface,itk::SmartPointer<Surface>)

  itkGetConstReferenceMacro(Translation,Translation)

  itkGetConstReferenceMacro(Rotation,Rotation)

  void SetCovarianceMatricesMovingSurface( CovarianceMatrixList& list )
  {
    m_CovarianceMatricesMovingSurface = list;
  }

  void SetCovarianceMatricesFixedSurface( CovarianceMatrixList& list )
  {
    m_CovarianceMatricesFixedSurface = list;
  }

  mitkClassMacro(AnisotropicIterativeClosestPointRegistration, itk::Object)
  itkNewMacro(Self)

  /** @brief Method which registers both point sets. */
  void Update();
};

}
#endif
