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

#ifndef BaseGeometry_h
#define BaseGeometry_h

//MITK
#include <MitkExports.h>
#include <mitkCommon.h>
#include "mitkOperationActor.h"
#include "mitkVector.h"

// To be replaced
#include <mitkSlicedGeometry3D.h>

// STL
#include <vector>

//ITK
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include <itkObject.h>

namespace mitk {


  class MITK_CORE_EXPORT BaseGeometry : public itk::Object, public OperationActor
  {

  public:
    mitkClassMacro(BaseGeometry, itk::Object);

    virtual void Transform( Transform3D& transformation )
    {};
    //##Documentation
    //## @brief Get the position of a corner (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    Point3D GetCornerPointInWorldSpace(bool xFront=true, bool yFront=true, bool zFront=true) const
    {return m_Geo->GetCornerPoint(xFront,yFront,zFront);}


    mitk::Geometry3D::Pointer m_Geo;


  protected:

    BaseGeometry();
    virtual ~BaseGeometry();

    BoundingBox::Pointer m_BoundingBox;
  }; // end class BaseGeometry

} // end namespace MITK
#endif // BaseGeometry_h