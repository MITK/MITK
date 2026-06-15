/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkAnnotation2D_h
#define mitkVtkAnnotation2D_h

#include <mitkVtkAnnotation.h>
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkProperty2D;

namespace mitk
{
  /**
   * \brief Base class for VTK-based 2D annotations rendered as vtkActor2D elements.
   *
   * Provides position and offset management in 2D display coordinates.
   * Subclasses must implement UpdateVtkAnnotation2D() and GetVtkActor2D().
   *
   * \sa VtkAnnotation, VtkAnnotation3D, TextAnnotation2D
   */
  class MITKANNOTATION_EXPORT VtkAnnotation2D : public VtkAnnotation
  {
  public:
    mitkClassMacro(VtkAnnotation2D, VtkAnnotation);

    /**
     * \brief Get the bounding rectangle of this annotation on the display.
     * \param[in] renderer The renderer to query.
     * \return The bounds in display coordinates.
     */
    Annotation::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const override;

    /**
     * \brief Set the bounding rectangle of this annotation on the display.
     * \param[in] renderer The renderer context.
     * \param[in] bounds The new bounds in display coordinates.
     */
    void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds &bounds) override;

    /**
     * \brief Set the 2D position in display coordinates.
     * \param[in] position2D The position.
     */
    void SetPosition2D(const Point2D &position2D);

    /**
     * \brief Get the 2D position in display coordinates.
     * \return The current position.
     */
    Point2D GetPosition2D() const;

    /**
     * \brief Set an offset vector added to the position.
     * \param[in] OffsetVector The offset.
     */
    void SetOffsetVector(const Point2D &OffsetVector);

    /**
     * \brief Get the offset vector.
     * \return The current offset.
     */
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
#endif
