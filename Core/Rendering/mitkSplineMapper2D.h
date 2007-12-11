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


#ifndef MITKSplineMapper2D_H_HEADER_INCLUDED
#define MITKSplineMapper2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"
#include "mitkPointSetMapper2D.h"
#include <vtkSystemIncludes.h>

namespace mitk
{

class BaseRenderer;
class PointSet;

//##Documentation
//## @brief OpenGL-based mapper to display a spline based on a mitk::PointSet in a 2D window.
//##
//## @ingroup Mapper
class SplineMapper2D : public PointSetMapper2D
{
public:
  mitkClassMacro ( SplineMapper2D, PointSetMapper2D );

  itkNewMacro ( Self );

  virtual void Paint ( mitk::BaseRenderer * renderer );

  virtual void ApplyProperties ( mitk::BaseRenderer* renderer );

  itkSetMacro ( MaxProjectionDistance, vtkFloatingPointType );

  itkGetMacro ( MaxProjectionDistance, vtkFloatingPointType );

protected:
  SplineMapper2D();

  virtual ~SplineMapper2D();

  vtkFloatingPointType m_MaxProjectionDistance;
};

} // namespace mitk



#endif /* MITKSplineMapper2D_H_HEADER_INCLUDED */
