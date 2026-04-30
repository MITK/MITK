/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkAnnotation_h
#define mitkVtkAnnotation_h

#include <mitkAnnotation.h>
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

class vtkProp;

namespace mitk
{
  /**
   * \brief Base class for all annotations that use the VTK framework for rendering.
   *
   * Subclasses must implement GetVtkProp() to provide a vtkProp element and
   * UpdateVtkAnnotation() to synchronize properties before rendering.
   *
   * \sa Annotation, VtkAnnotation2D, VtkAnnotation3D
   * \ingroup Annotation
   */
  class MITKANNOTATION_EXPORT VtkAnnotation : public Annotation
  {
  public:
    mitkClassMacro(VtkAnnotation, Annotation);

    /**
     * \brief Update the annotation for the given renderer.
     * \param[in] renderer The renderer to update for.
     */
    void Update(BaseRenderer *renderer) override;

    /**
     * \brief Add this annotation's vtkProp to the given BaseRenderer.
     * \param[in] renderer The renderer to add to.
     */
    void AddToBaseRenderer(BaseRenderer *renderer) override;

    /**
     * \brief Add this annotation's vtkProp to a specific vtkRenderer.
     * \param[in] renderer The MITK renderer.
     * \param[in] vtkrenderer The VTK renderer.
     */
    void AddToRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) override;

    /**
     * \brief Remove this annotation's vtkProp from a specific vtkRenderer.
     * \param[in] renderer The MITK renderer.
     * \param[in] vtkrenderer The VTK renderer.
     */
    void RemoveFromRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) override;

    /**
     * \brief Remove this annotation's vtkProp from the given BaseRenderer.
     * \param[in] renderer The renderer to remove from.
     */
    void RemoveFromBaseRenderer(BaseRenderer *renderer) override;

    /**
     * \brief Force an immediate paint of this annotation.
     *
     * \warning Should only be used as alternative to the AnnotationManager
     * mechanism in GL-Mappers.
     *
     * \param[in] renderer The renderer to paint into.
     */
    void Paint(BaseRenderer *renderer);

  protected:
    /**
     * \brief Create the VTK prop representing this annotation.
     *
     * Must be implemented by subclasses.
     *
     * \return The vtkProp element created by the subclass.
     */
    virtual vtkProp *GetVtkProp(BaseRenderer *renderer) const = 0;
    virtual void UpdateVtkAnnotation(BaseRenderer *renderer) = 0;

    /** \brief explicit constructor which disallows implicit conversions */
    explicit VtkAnnotation();

    /** \brief virtual destructor in order to derive from this class */
    ~VtkAnnotation() override;

  private:
    /** \brief copy constructor */
    VtkAnnotation(const VtkAnnotation &);

    /** \brief assignment operator */
    VtkAnnotation &operator=(const VtkAnnotation &);
  };

} // namespace mitk
#endif
