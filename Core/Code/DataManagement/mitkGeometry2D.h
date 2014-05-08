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

#ifndef GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0
#define GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0

#include <MitkCoreExports.h>
#include <mitkCommon.h>

#include "mitkPlaneGeometry.h"

class vtkLinearTransform;

namespace mitk {
  //##Documentation
  //## @brief Describes the geometry of a two-dimensional object
  //##
  //## @ingroup Geometry
  /** \deprecatedSince{2014_06} This class is deprecated. Please use PlaneGeometry instead. */
  class MITK_CORE_EXPORT Geometry2D : public PlaneGeometry
  {
  public:
    mitkClassMacro(Geometry2D, mitk::PlaneGeometry);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

  protected:
    Geometry2D();
    Geometry2D(const Geometry2D& other);

    virtual ~Geometry2D();
  };
} // namespace mitk

#endif /* GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0 */
