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

#ifndef MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjectorBasedCurvedGeometry.h"

class vtkPoints;
class vtkThinPlateSplineTransform;

namespace mitk {
  //##Documentation
  //## @brief Thin-plate-spline-based landmark-based curved geometry
  //##
  //## @ingroup Geometry
  class MITK_CORE_EXPORT ThinPlateSplineCurvedGeometry : public LandmarkProjectorBasedCurvedGeometry
  {
  public:
  mitkClassMacro(ThinPlateSplineCurvedGeometry, LandmarkProjectorBasedCurvedGeometry);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void ComputeGeometry();

  virtual itk::LightObject::Pointer InternalClone() const;

  vtkThinPlateSplineTransform* GetThinPlateSplineTransform() const
  {
    return m_ThinPlateSplineTransform;
  }

  virtual void SetSigma(double sigma);
  virtual double GetSigma() const;

  virtual bool IsValid() const;

  protected:
  ThinPlateSplineCurvedGeometry();
  ThinPlateSplineCurvedGeometry(const ThinPlateSplineCurvedGeometry& other );

  virtual ~ThinPlateSplineCurvedGeometry();

  vtkThinPlateSplineTransform* m_ThinPlateSplineTransform;

  vtkPoints* m_VtkTargetLandmarks;
  vtkPoints* m_VtkProjectedLandmarks;
  };
} // namespace mitk

#endif /* MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
