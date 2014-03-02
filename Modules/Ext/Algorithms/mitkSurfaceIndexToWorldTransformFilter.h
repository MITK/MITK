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

#ifndef _mitkSurfaceIndexToWorldTransformFilter_h__
#define _mitkSurfaceIndexToWorldTransformFilter_h__

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkSurface.h"

namespace mitk {

  //##Documentation
  //## @brief Transforms a surface object from index to world coordinates.
  //## Transposes the surface point coordinates with the object's mitk::Geometry3D to display
  //## correct scene coordinates.
  //##
  //## @ingroup Algorithms

  class MitkExt_EXPORT SurfaceIndexToWorldTransformFilter : public SurfaceToSurfaceFilter
{
public:

  /**
  * Standard mitk class macro
  */
  mitkClassMacro(SurfaceIndexToWorldTransformFilter, SurfaceToSurfaceFilter);
  itkNewMacro(Self);

protected:

  /**
  * Protected constructor. Use ::New instead()
  */
  SurfaceIndexToWorldTransformFilter();

  /**
  * Protected destructor. Instances are destroyed when reference count is zero
  */
  virtual ~SurfaceIndexToWorldTransformFilter();

  /**
  * method executed when calling Update();
  */
  virtual void GenerateData();

};

} // namespace mitk

#endif /* _mitkSurfaceIndexToWorldTransformFilter_h__*/
