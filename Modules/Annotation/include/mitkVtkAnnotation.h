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

#ifndef VTKAnnotation_H
#define VTKAnnotation_H

#include "mitkAnnotation.h"
#include <MitkAnnotationExports.h>
#include <vtkSmartPointer.h>

class vtkProp;

namespace mitk
{
  /**
   * @brief The VtkAnnotation class is the base for all Annotation which are using the VTK framework to render
   *the elements.
   */
  class MITKANNOTATION_EXPORT VtkAnnotation : public Annotation
  {
  public:
    mitkClassMacro(VtkAnnotation, Annotation);
    void Update(BaseRenderer *renderer) override;
    void AddToBaseRenderer(BaseRenderer *renderer) override;
    void AddToRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) override;
    void RemoveFromRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) override;
    void RemoveFromBaseRenderer(BaseRenderer *renderer) override;

    /**
    * \brief Paints the Annotation.
    *
    * This method forces a paint of the Annotation as it is configured at the moment.
    * \warn Should only be used as alternative to the AnnotationManager mechanism
    * in GL-Mappers.
    */
    void Paint(BaseRenderer *renderer);

  protected:
    /**
     * @brief This method is implemented by the specific VTKAnnotation in order to create the element as a vtkProp
     * @param renderer
     * @return The element that was created by the subclasses as a vtkProp.
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
#endif // Annotation_H
