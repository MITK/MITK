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

#ifndef VTKAnnotation3D_H
#define VTKAnnotation3D_H

#include "mitkVtkAnnotation.h"
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * @brief The VtkAnnotation3D class is the basis for all VTK based Annotation which create
   * any 3D element as a vtkProp that will be drawn on the renderer.
   */
  class MITKANNOTATION_EXPORT VtkAnnotation3D : public VtkAnnotation
  {
  public:
    void SetPosition3D(const Point3D &position3D);

    Point3D GetPosition3D() const;

    void SetOffsetVector(const Point3D &OffsetVector);

    Point3D GetOffsetVector() const;

    mitkClassMacro(VtkAnnotation3D, VtkAnnotation);

  protected:
    void UpdateVtkAnnotation(BaseRenderer *renderer) override = 0;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit VtkAnnotation3D();

    /** \brief virtual destructor in order to derive from this class */
    ~VtkAnnotation3D() override;

  private:
    /** \brief copy constructor */
    VtkAnnotation3D(const VtkAnnotation3D &);

    /** \brief assignment operator */
    VtkAnnotation3D &operator=(const VtkAnnotation3D &);
  };

} // namespace mitk
#endif // Annotation_H
