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


#ifndef MITKLANDMARKBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKLANDMARKBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkAbstractTransformGeometry.h"
#include "mitkPointSet.h"

namespace mitk {

//##Documentation
//## @brief Superclass of AbstractTransformGeometry sub-classes defined
//## by a set of landmarks.
//## 
//## @ingroup Geometry
class MITK_CORE_EXPORT LandmarkBasedCurvedGeometry : public AbstractTransformGeometry
{
public:
  mitkClassMacro(LandmarkBasedCurvedGeometry, AbstractTransformGeometry);

  //##Documentation
  //## @brief Set the landmarks through which the geometry shall pass
  itkSetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);
  //##Documentation
  //## @brief Get the landmarks through which the geometry shall pass
  itkGetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);

  virtual void ComputeGeometry() = 0;

protected:
  LandmarkBasedCurvedGeometry();

  virtual ~LandmarkBasedCurvedGeometry();
  
  void InitializeGeometry(Self * newGeometry) const;

  mitk::PointSet::DataType::PointsContainer::ConstPointer m_TargetLandmarks;

};

} // namespace mitk

#endif /* MITKLANDMARKBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
