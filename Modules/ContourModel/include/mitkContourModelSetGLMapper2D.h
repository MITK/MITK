/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetGLMapper2D_h
#define mitkContourModelSetGLMapper2D_h

#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkContourModelGLMapper2DBase.h>
#include <mitkContourModelSet.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  class BaseRenderer;
  class ContourModel;

  /** \brief OpenGL-based mapper to display a mitk::ContourModelSet in a 2D render window.
   *
   * Iterates over all ContourModel instances in the ContourModelSet and draws each
   * one onto the current 2D slice using OpenGL primitives.
   *
   * \sa ContourModelSet, ContourModelGLMapper2DBase, ContourModelGLMapper2D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSetGLMapper2D : public ContourModelGLMapper2DBase
  {
  public:
    mitkClassMacro(ContourModelSetGLMapper2D, ContourModelGLMapper2DBase);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Render all contour models in the set onto the 2D slice.
       * \param[in] renderer The renderer for the current render window.
       * \param[in] type The render type (opaque, transparent, overlay).
       */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    /** \brief Set default rendering properties for ContourModelSet nodes.
     * \param[in] node The data node to configure.
     * \param[in] renderer The renderer context. If nullptr, properties are set globally.
     * \param[in] overwrite If true, existing properties are overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Local storage handler for per-renderer data. */
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
