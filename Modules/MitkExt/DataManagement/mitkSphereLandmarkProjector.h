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


#ifndef MITKSPHERELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C
#define MITKSPHERELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjector.h"
#include "MitkExtExports.h"
#include "mitkPointSet.h"

class vtkTransform;
class vtkSphericalTransform;
class vtkGeneralTransform;
class vtkTransformPolyDataFilter;

namespace mitk {

//##Documentation
//## @brief Thin-plate-spline-based landmark-based curved geometry
//##
//## @ingroup Geometry
class MitkExt_EXPORT SphereLandmarkProjector : public LandmarkProjector
{
public:
  mitkClassMacro(SphereLandmarkProjector, LandmarkProjector);

  itkNewMacro(Self);

  virtual void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer* targetLandmarks);
protected:
  SphereLandmarkProjector();
  virtual ~SphereLandmarkProjector();

  virtual void ComputeCompleteAbstractTransform();

  vtkSphericalTransform*       m_SphericalTransform;
  vtkTransform*                m_SphereRotation;
  vtkTransform*                m_SpatialPlacementTransform;
  vtkGeneralTransform*         m_PlaneToSphericalTransform;

  mitk::PlaneGeometry::Pointer m_SphereParameterPlane;
};

} // namespace mitk

#endif /* MITKSPHERELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C */
