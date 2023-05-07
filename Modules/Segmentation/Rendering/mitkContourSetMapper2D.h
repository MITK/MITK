/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourSetMapper2D_h
#define mitkContourSetMapper2D_h

#include "mitkCommon.h"
#include "mitkGLMapper.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class BaseRenderer;
  class ContourSet;

  /**
   * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window
   *
   *
   * @ingroup Mapper
   */
  class MITKSEGMENTATION_EXPORT ContourSetMapper2D : public Mapper
  {
  public:
    mitkClassMacro(ContourSetMapper2D, Mapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
      * reimplemented from Baseclass
      */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor = nullptr) override;

    /**
     * return a refernce of the rendered data object
     */
    const mitk::ContourSet *GetInput(void);

  protected:
    ContourSetMapper2D();

    ~ContourSetMapper2D() override;
  };

} // namespace mitk

#endif
