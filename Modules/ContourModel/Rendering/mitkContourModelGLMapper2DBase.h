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
#include "mitkMapper.h"
#include "mitkTextAnnotation2D.h"
#include <MitkContourModelExports.h>
#include "vtkNew.h"

class vtkContext2D;
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

    virtual ~ContourModelGLMapper2DBase();

    void DrawContour(mitk::ContourModel *contour, mitk::BaseRenderer *renderer);

    void WriteTextWithAnnotation(
      TextAnnotationPointerType textAnnotation, const char *text, float rgb[3], Point2D pt2d, mitk::BaseRenderer *);

    virtual void InternalDrawContour(mitk::ContourModel *renderingContour, mitk::BaseRenderer *renderer);

    void Initialize(mitk::BaseRenderer *renderer);

    TextAnnotationPointerType m_PointNumbersAnnotation;
    TextAnnotationPointerType m_ControlPointNumbersAnnotation;

    typedef std::vector<BaseRenderer *> RendererListType;
    RendererListType m_RendererList;

    bool m_Initialized;

    vtkNew<vtkContext2D> m_Context;
  };

} // namespace mitk

#endif
