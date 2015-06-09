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


#ifndef MITK_ContourModelGLMapper2D_H_
#define MITK_ContourModelGLMapper2D_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include "mitkContourModelGLMapper2DBase.h"
#include "mitkContourModel.h"


namespace mitk {

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

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * reimplemented from Baseclass
    */
    virtual void Paint(BaseRenderer * renderer) override;

    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;


  protected:
    ContourModelGLMapper2D();

    virtual ~ContourModelGLMapper2D();

    mitk::ContourModel::Pointer m_SubdivisionContour;
    bool m_InitSubdivisionCurve;


  private:

    /**
    * return a refernce of the rendered data object
    */
    ContourModel* GetInput(void);
  };

} // namespace mitk



#endif
