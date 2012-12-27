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


#ifndef MITKEXTERNABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKEXTERNABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkAbstractTransformGeometry.h"

namespace mitk {

//##Documentation
//## @brief Identical with AbstractTransformGeometry, except that
//## it can be externally configured.
//##
//## In contrast to its superclass (AbstractTransformGeometry), this class
//## provides write access to the vtkAbstractTransform and m_Plane.
//## @note The PlaneGeometry is cloned, @em not linked/referenced.
//## @note The bounds of the PlaneGeometry are used as the parametric bounds.
//## @sa AbstractTransformGeometry
//## @ingroup Geometry
class MitkExt_EXPORT ExternAbstractTransformGeometry : public AbstractTransformGeometry
{
public:
  mitkClassMacro(ExternAbstractTransformGeometry, AbstractTransformGeometry);

  itkNewMacro(Self);

  //##Documentation
  //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual void SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform);

  //##Documentation
  //## @brief Set the rectangular area that is used for transformation by
  //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
  //## ExternAbstractTransformGeometry
  //##
  //## @note The bounds of the PlaneGeometry are used as the parametric bounds.
  //## @note The PlaneGeometry is cloned, @em not linked/referenced.
  virtual void SetPlane(const mitk::PlaneGeometry* aPlane);

  virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds);

  virtual AffineGeometryFrame3D::Pointer Clone() const;

protected:

  ExternAbstractTransformGeometry();
  ExternAbstractTransformGeometry(const ExternAbstractTransformGeometry& other);
  virtual ~ExternAbstractTransformGeometry();

};

} // namespace mitk
#endif /* MITKEXTERNABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
