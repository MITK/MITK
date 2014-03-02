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


#ifndef MITK_ContourModelGLMapper2DBase_H_
#define MITK_ContourModelGLMapper2DBase_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include "mitkGLMapper.h"
#include "mitkContourModel.h"
#include "mitkBaseRenderer.h"


namespace mitk {

  class BaseRenderer;
  class ContourModel;

  /**
  * @brief Base class for OpenGL based 2D mappers.
  * Provides functionality to draw a contour.
  *
  * @ingroup Mapper
  */
  class MitkContourModel_EXPORT ContourModelGLMapper2DBase : public GLMapper
  {
  public:

    mitkClassMacro(ContourModelGLMapper2DBase, GLMapper);


  protected:
    ContourModelGLMapper2DBase();

    virtual ~ContourModelGLMapper2DBase();

    void DrawContour(mitk::ContourModel* contour, mitk::BaseRenderer* renderer);
  };

} // namespace mitk



#endif
