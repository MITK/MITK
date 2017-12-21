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

#ifndef VTKAnnotation2D_H
#define VTKAnnotation2D_H

#include "mitkVtkAnnotation.h"
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkProperty2D;

namespace mitk
{
  /**
   * @brief The VtkAnnotation2D class is the basis for all VTK based Annotation which create
   * a vtkActor2D element that will be drawn on the renderer.
   */
  class MITKANNOTATION_EXPORT VtkAnnotation2D : public VtkAnnotation
  {
  public:
    mitkClassMacro(VtkAnnotation2D, VtkAnnotation);
    Annotation::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const override;
    void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds &bounds) override;

    void SetPosition2D(const Point2D &position2D);

    Point2D GetPosition2D() const;

    void SetOffsetVector(const Point2D &OffsetVector);

    Point2D GetOffsetVector() const;

  protected:
    vtkProp *GetVtkProp(BaseRenderer *renderer) const override;
    void UpdateVtkAnnotation(BaseRenderer *renderer) override;
    virtual void UpdateVtkAnnotation2D(BaseRenderer *renderer) = 0;
    virtual vtkActor2D *GetVtkActor2D(BaseRenderer *renderer) const = 0;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit VtkAnnotation2D();

    /** \brief virtual destructor in order to derive from this class */
    ~VtkAnnotation2D() override;

  private:
    /** \brief copy constructor */
    VtkAnnotation2D(const VtkAnnotation2D &);

    /** \brief assignment operator */
    VtkAnnotation2D &operator=(const VtkAnnotation2D &);
  };

} // namespace mitk
#endif // VTKAnnotation2D_H
