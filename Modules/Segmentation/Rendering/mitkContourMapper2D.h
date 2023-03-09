/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourMapper2D_h
#define mitkContourMapper2D_h

#include "mitkCommon.h"
#include "mitkMapper.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class BaseRenderer;
  class Contour;

  /**
   * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window
   *
   *
   * @ingroup Mapper
   */
  class MITKSEGMENTATION_EXPORT ContourMapper2D : public Mapper
  {
  public:
    mitkClassMacro(ContourMapper2D, Mapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * reimplemented from Baseclass
       */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor=nullptr) override;

    /**
     * return a refernce of the rendered data object
     */
    const Contour *GetInput(void);

  protected:
    ContourMapper2D();

    ~ContourMapper2D() override;
  };

} // namespace mitk

#endif
