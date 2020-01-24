/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_ContourModelSetGLMapper2D_H_
#define MITK_ContourModelSetGLMapper2D_H_

#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkContourModelGLMapper2DBase.h"
#include "mitkContourModelSet.h"
#include <MitkContourModelExports.h>

namespace mitk
{
  class BaseRenderer;
  class ContourModel;

  /**
  * @brief OpenGL-based mapper to display a mitk::ContourModelSet object containing several contours in a 2D render
  * window
  *
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelSetGLMapper2D : public ContourModelGLMapper2DBase
  {
  public:
    mitkClassMacro(ContourModelSetGLMapper2D, ContourModelGLMapper2DBase);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
      * reimplemented from Baseclass
      */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    ContourModelSetGLMapper2D();

    ~ContourModelSetGLMapper2D() override;

  private:
    /**
    * return a reference of the rendered data object
    */
    ContourModelSet *GetInput(void);
  };

} // namespace mitk

#endif
