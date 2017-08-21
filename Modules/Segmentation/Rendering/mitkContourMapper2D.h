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

#ifndef MITK_CONTOUR_MAPPER_2D_H_
#define MITK_CONTOUR_MAPPER_2D_H_

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

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /**
       * reimplemented from Baseclass
       */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type);

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor=nullptr);

    /**
     * return a refernce of the rendered data object
     */
    const Contour *GetInput(void);

  protected:
    ContourMapper2D();

    virtual ~ContourMapper2D();
  };

} // namespace mitk

#endif /* MITKContourMapper2D_H_HEADER_INCLUDED */
