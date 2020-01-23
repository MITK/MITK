/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_ContourModelGLMapper2D_H_
#define MITK_ContourModelGLMapper2D_H_

#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkContourModelGLMapper2DBase.h"
#include <MitkContourModelExports.h>

namespace mitk
{
  class BaseRenderer;
  class ContourModel;

  /**
  * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window
  *
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelGLMapper2D : public ContourModelGLMapper2DBase
  {
  public:
    mitkClassMacro(ContourModelGLMapper2D, ContourModelGLMapper2DBase);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
      * reimplemented from Baseclass
      */
      void MitkRender(BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    ContourModelGLMapper2D();

    ~ContourModelGLMapper2D() override;

    mitk::ContourModel::Pointer m_SubdivisionContour;
    bool m_InitSubdivisionCurve;

  private:
    /**
    * return a refernce of the rendered data object
    */
    ContourModel *GetInput(void);
  };

} // namespace mitk

#endif
