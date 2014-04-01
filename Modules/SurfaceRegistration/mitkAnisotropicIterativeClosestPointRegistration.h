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

class vtkPoints;
class vtkKdTreePointLocator;

namespace mitk
{

class Surface;
class WeightedPointTransform;

class MitkSurfaceRegistration_EXPORT AnisotropicIterativeClosestPointRegistration : public itk::Object
{

protected:

  /** Definition of a 3x3 covariance matrix.*/
  typedef itk::Matrix < double, 3, 3 > CovarianceMatrix;
  /** Definition of a list of covariance matrices.*/
  typedef std::vector< CovarianceMatrix > CovarianceMatrixList;
  /** Definition of a translation vector.*/
  typedef mitk::Vector3D Translation;
  /** Definition of a 3x3 rotation matrix.*/
  typedef CovarianceMatrix Rotation;
  /** Definition of a correspondeces, index and distance.*/
  typedef std::pair < unsigned int, double > Correspondence;
  /** Definition of a list of correspondences.*/
  typedef std::vector < Correspondence > CorrespondenceList;

  AnisotropicIterativeClosestPointRegistration();
  ~AnisotropicIterativeClosestPointRegistration();

  /** Max amount of iterations.*/
  unsigned int m_MaxIterations;

  /** Threshold used for termination.*/
  double m_Threshold;

  /** Normalization factor for the feducial registration error.*/
  double m_FRENormalizationFactor;

  /** Search radius for the correspondence search.*/
  double m_SearchRadius;

  /** The maximum number of iterations in the weighted point based registration.*/
  double m_MaxIterationsInWeightedPointTransform;

  /** The fiducial registration error.*/
  double m_FRE;

  /** Trimmfactor for partial overlapping registration.*/
  double m_TrimmFactor;

  /** Amount of iterations used by the algorithm.*/
  unsigned int m_NumberOfIterations;

  /** Moving surface that is transformed on the fixed surface.*/
  itk::SmartPointer < Surface > m_MovingSurface;
  /** The fixed / target surface.*/
  itk::SmartPointer < Surface > m_FixedSurface;

  /** The weighted point based registration algorithm.*/
  itk::SmartPointer < WeightedPointTransform > m_WeightedPointTransform;

  /** */
  CovarianceMatrixList m_CovarianceMatricesMovingSurface;
  CovarianceMatrixList m_CovarianceMatricesFixedSurface;

  Translation m_Translation;
  Rotation    m_Rotation;

  void ComputeCorrespondences ( vtkPoints* X,
                                vtkPoints* Z,
                                vtkKdTreePointLocator *Y,
                                const CovarianceMatrixList& sigma_X,
                                const CovarianceMatrixList& sigma_Y,
                                CovarianceMatrixList& sigma_Z,
                                CorrespondenceList &correspondences,
                                const double radius
                               );


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

  void Update();
};

}
#endif
