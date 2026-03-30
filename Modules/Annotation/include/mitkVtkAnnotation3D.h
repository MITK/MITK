/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkAnnotation3D_h
#define mitkVtkAnnotation3D_h

#include <mitkVtkAnnotation.h>
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \brief Base class for VTK-based 3D annotations rendered as vtkProp elements.
   *
   * Provides position and offset management in 3D world coordinates.
   * Subclasses must implement UpdateVtkAnnotation().
   *
   * \sa VtkAnnotation, VtkAnnotation2D, TextAnnotation3D, LabelAnnotation3D
   * \ingroup Annotation
   */
  class MITKANNOTATION_EXPORT VtkAnnotation3D : public VtkAnnotation
  {
  public:
    /**
     * \brief Set the 3D position in world coordinates.
     * \param[in] position3D The position.
     */
    void SetPosition3D(const Point3D &position3D);

    /**
     * \brief Get the 3D position in world coordinates.
     * \return The current position.
     */
    Point3D GetPosition3D() const;

    /**
     * \brief Set an offset vector added to the 3D position.
     * \param[in] OffsetVector The offset in world coordinates.
     */
    void SetOffsetVector(const Point3D &OffsetVector);

    /**
     * \brief Get the 3D offset vector.
     * \return The current offset.
     */
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
#endif
