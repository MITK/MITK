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


#ifndef MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkBasedCurvedGeometry.h"
#include "mitkLandmarkProjector.h"

namespace mitk {

//##Documentation
//## @brief Superclass of AbstractTransformGeometry sub-classes defined
//## by a set of landmarks.
//## 
//## @ingroup Geometry
class MITK_CORE_EXPORT LandmarkProjectorBasedCurvedGeometry : public LandmarkBasedCurvedGeometry
{
public:
  mitkClassMacro(LandmarkProjectorBasedCurvedGeometry, LandmarkBasedCurvedGeometry);

  void SetLandmarkProjector(mitk::LandmarkProjector* aLandmarkProjector);
  itkGetConstObjectMacro(LandmarkProjector, mitk::LandmarkProjector);

  virtual void SetFrameGeometry(const mitk::Geometry3D* frameGeometry);

  virtual void ComputeGeometry();

  itkGetConstMacro(InterpolatingAbstractTransform, vtkAbstractTransform*);
protected:
  LandmarkProjectorBasedCurvedGeometry();

  virtual ~LandmarkProjectorBasedCurvedGeometry();
  
  void InitializeGeometry(Self * newGeometry) const;

  mitk::LandmarkProjector::Pointer m_LandmarkProjector;

  vtkAbstractTransform* m_InterpolatingAbstractTransform;
};

} // namespace mitk

#endif /* MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
