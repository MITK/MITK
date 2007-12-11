/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
class ThinPlateSplineCurvedGeometry : public LandmarkProjectorBasedCurvedGeometry
{
public:
  mitkClassMacro(ThinPlateSplineCurvedGeometry, LandmarkProjectorBasedCurvedGeometry);
  
  itkNewMacro(Self);
  
  virtual void ComputeGeometry();

  virtual AffineGeometryFrame3D::Pointer Clone() const;

  vtkThinPlateSplineTransform* GetThinPlateSplineTransform() const  
  {
    return m_ThinPlateSplineTransform;
  }

  virtual void SetSigma(float sigma);
  virtual float GetSigma() const;

  virtual bool IsValid() const;

protected:
  ThinPlateSplineCurvedGeometry();
  virtual ~ThinPlateSplineCurvedGeometry();
  
  vtkThinPlateSplineTransform* m_ThinPlateSplineTransform;

  vtkPoints* m_VtkTargetLandmarks;
  vtkPoints* m_VtkProjectedLandmarks;

  void InitializeGeometry(Self * newGeometry) const;
};

} // namespace mitk

#endif /* MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
