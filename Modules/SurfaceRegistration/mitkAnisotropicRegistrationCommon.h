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

#ifndef __ANISOTROPICREGISTRATIONCOMMON_H__
#define __ANISOTROPICREGISTRATIONCOMMON_H__

#include <mitkCommon.h>
#include <itkMatrix.h>
#include <mitkVector.h>


#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <MitkSurfaceRegistrationExports.h>
//class vtkPoints;

namespace mitk
{

class PointSet;

class MitkSurfaceRegistration_EXPORT  AnisotropicRegistrationCommon
{

protected:

  typedef itk::Matrix <double, 3, 3> WeightMatrix;
  typedef WeightMatrix Rotation;
  typedef mitk::Vector3D Translation;
  typedef std::vector< WeightMatrix > MatrixList;

  AnisotropicRegistrationCommon(){}
  ~AnisotropicRegistrationCommon(){}

public:

static WeightMatrix CalculateWeightMatrix(const WeightMatrix &sigma_X, const WeightMatrix &sigma_Y );

static void TransformPoints(vtkPoints* src, vtkPoints* dst,
                            const Rotation& rotation, const Translation& translation);

static void PropagateMatrices(const MatrixList &src, MatrixList& dst, const Rotation &rotation);

static double ComputeTargetRegistrationError( const mitk::PointSet* movingTargets,
                                              const mitk::PointSet* fixedTargets,
                                              const Rotation& rotation,
                                              const Translation& translation
                                            );

};

}

#endif
