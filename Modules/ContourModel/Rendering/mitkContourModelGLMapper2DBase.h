/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelGLMapper2DBase_h
#define mitkContourModelGLMapper2DBase_h

#include "mitkCommon.h"
#include "mitkMapper.h"
#include "mitkTextAnnotation2D.h"
#include <MitkContourModelExports.h>
#include <vtkSmartPointer.h>

class vtkContext2D;
class vtkOpenGLContextDevice2D;
class vtkPen;

namespace mitk
{
  class BaseRenderer;
  class ContourModel;

  /**
  * @brief Base class for OpenGL based 2D mappers.
  * Provides functionality to draw a contour.
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelGLMapper2DBase : public Mapper
  {
  public:
    mitkClassMacro(ContourModelGLMapper2DBase, Mapper);

    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor = nullptr) override;

  protected:
    typedef TextAnnotation2D::Pointer TextAnnotationPointerType;

    ContourModelGLMapper2DBase();

    ~ContourModelGLMapper2DBase() override;

    void DrawContour(mitk::ContourModel *contour, mitk::BaseRenderer *renderer);

    void WriteTextWithAnnotation(
      TextAnnotationPointerType textAnnotation, const char *text, float rgb[3], Point2D pt2d, mitk::BaseRenderer *);

    virtual void InternalDrawContour(mitk::ContourModel *renderingContour, mitk::BaseRenderer *renderer);

    TextAnnotationPointerType m_PointNumbersAnnotation;
    TextAnnotationPointerType m_ControlPointNumbersAnnotation;

    typedef std::vector<BaseRenderer *> RendererListType;
    RendererListType m_RendererList;

  private:
    struct LocalStorage
    {
      vtkSmartPointer<vtkOpenGLContextDevice2D> Device;
      vtkSmartPointer<vtkContext2D> Context;
    };

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };

} // namespace mitk

#endif
