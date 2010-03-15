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
#include "MitkExtExports.h"
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
/** This mapper can actually more than just draw a number of points of a 
* mitk::PointSet. If you set the right properties of the mitk::DataNode, 
* which contains the point set, then this mapper will also draw lines 
* connecting the points, and calculate and display distances and angles 
* between adjacent points. Here is a complete list of boolean properties, 
* which might be of interest:
*
* - \b "show contour": Draw not only the points but also the connections between 
*     them (default false)
* - \b "line width": IntProperty which gives the width of the contour lines
* - \b "show points": Wheter or not to draw the actual points (default true)
* - \b "show distances": Wheter or not to calculate and print the distance 
*     between adjacent points (default false)
* - \b "show angles": Wheter or not to calculate and print the angle between 
*     adjacent points (default false)
* - \b "show distant lines": When true, the mapper will also draw contour 
*     lines that are far away form the current slice (default true)
* - \b "label": StringProperty with a label for this point set
*/
//## @ingroup Mapper
class MitkExt_EXPORT SplineMapper2D : public PointSetMapper2D
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
