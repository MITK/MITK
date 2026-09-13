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
    ContourModelMapper2DBase();

    ~ContourModelMapper2DBase() override;

    /** \brief Prepare the drawing context of a renderer for one frame.
     *
     * Brackets any number of DrawContour() calls together with EndDrawing().
     * The context and its device are kept per renderer and reused across
     * frames, so only the painting itself is repeated.
     */
    void BeginDrawing(mitk::BaseRenderer *renderer);

    /** \brief Release the drawing context of a renderer again. */
    void EndDrawing(mitk::BaseRenderer *renderer);

    /** \brief Draw one contour. Only valid between BeginDrawing() and EndDrawing(). */
    void DrawContour(mitk::ContourModel *contour, mitk::BaseRenderer *renderer);

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
