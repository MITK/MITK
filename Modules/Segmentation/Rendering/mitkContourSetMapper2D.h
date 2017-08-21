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

#ifndef MITK_CONTOUR_SET_MAPPER_2D_H_
#define MITK_CONTOUR_SET_MAPPER_2D_H_

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

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /**
      * reimplemented from Baseclass
      */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type);

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor = nullptr);

    /**
     * return a refernce of the rendered data object
     */
    const mitk::ContourSet *GetInput(void);

  protected:
    ContourSetMapper2D();

    virtual ~ContourSetMapper2D();
  };

} // namespace mitk

#endif /* MITK_CONTOUR_SET_MAPPER_2D_H_ */
