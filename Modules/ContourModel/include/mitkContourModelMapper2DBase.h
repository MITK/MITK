/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelMapper2DBase_h
#define mitkContourModelMapper2DBase_h

#include <mitkCommon.h>
#include <mitkMapper.h>
#include <mitkTextAnnotation2D.h>
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
   * \brief Abstract base class for OpenGL-based 2D contour mappers.
   *
   * Provides shared functionality for drawing contour models in 2D render windows
   * using OpenGL. Subclasses such as ContourModelMapper2D and ContourModelSetMapper2D
   * implement the MitkRender method to handle specific data types.
   *
   * \sa ContourModelMapper2D, ContourModelSetMapper2D, ContourModel
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelMapper2DBase : public Mapper
  {
  public:
    mitkClassMacro(ContourModelMapper2DBase, Mapper);

    /** \brief Apply color and opacity properties from the data node to the rendering context.
     * \param[in] renderer The base renderer providing the rendering context.
     * \param[in] actor Optional VTK actor (unused in OpenGL rendering). Default is nullptr.
     */
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor * actor = nullptr) override;

  protected:
    typedef TextAnnotation2D::Pointer TextAnnotationPointerType;

    ContourModelMapper2DBase();

    ~ContourModelMapper2DBase() override;

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
