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
#include "SegmentationContourExports.h"
#include "mitkGLMapper.h"
#include "mitkContourModelSet.h"


namespace mitk {

  class BaseRenderer;
  class ContourModel;

  /**
  * @brief OpenGL-based mapper to display a mitk::ContourModelSet object containing several contours in a 2D render window
  *
  *
  * @ingroup Mapper
  */
  class SegmentationContour_EXPORT ContourModelSetGLMapper2D : public GLMapper
  {
  public:

    mitkClassMacro(ContourModelSetGLMapper2D, GLMapper);

    itkNewMacro(Self);

    /**
    * reimplemented from Baseclass
    */
    virtual void Paint(BaseRenderer * renderer);

    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    ContourModelSetGLMapper2D();

    virtual ~ContourModelSetGLMapper2D();

    void DrawContour(mitk::ContourModel& contour);

  private:

    /**
    * return a refernce of the rendered data object
    */
    const ContourModelSet* GetInput(void);
  };

} // namespace mitk



#endif
