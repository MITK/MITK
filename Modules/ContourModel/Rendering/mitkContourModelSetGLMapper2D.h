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


#ifndef MITK_ContourModelSetGLMapper2D_H_
#define MITK_ContourModelSetGLMapper2D_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include "mitkContourModelGLMapper2DBase.h"
#include "mitkContourModelSet.h"
#include "mitkBaseRenderer.h"


namespace mitk {

  class BaseRenderer;
  class ContourModel;

  /**
  * @brief OpenGL-based mapper to display a mitk::ContourModelSet object containing several contours in a 2D render window
  *
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelSetGLMapper2D : public ContourModelGLMapper2DBase
  {
  public:

    mitkClassMacro(ContourModelSetGLMapper2D, ContourModelGLMapper2DBase);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * reimplemented from Baseclass
    */
    virtual void Paint(BaseRenderer * renderer) override;

    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    ContourModelSetGLMapper2D();

    virtual ~ContourModelSetGLMapper2D();

    void InternalDrawContour(mitk::ContourModel* contour, mitk::BaseRenderer* renderer) override;

  private:

    /**
    * return a refernce of the rendered data object
    */
    ContourModelSet* GetInput(void);
  };

} // namespace mitk



#endif
