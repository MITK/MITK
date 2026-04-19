/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkGLMapperWrapper_h
#define mitkVtkGLMapperWrapper_h

#include <mitkBaseRenderer.h>
#include <mitkGLMapper.h>
#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>
#include <MitkLegacyGLExports.h>
#include <vtkSmartPointer.h>

class vtkGLMapperProp;

namespace mitk
{
  /** \brief Adapter that wraps a legacy GLMapper as a VtkMapper.
   *
   * Allows legacy OpenGL-based mappers (GLMapper subclasses) to be used within
   * the modern VTK-based rendering pipeline. Internally, the GLMapper's Paint()
   * method is called through a vtkGLMapperProp.
   *
   * \sa GLMapper, vtkGLMapperProp, VtkMapper
   */
  class MITKLEGACYGL_EXPORT VtkGLMapperWrapper : public VtkMapper
  {
  public:
    mitkClassMacro(VtkGLMapperWrapper, VtkMapper);

    /** \brief Create a wrapper for the given legacy GL mapper. */
    mitkNewMacro1Param(Self, GLMapper::Pointer);

    itkCloneMacro(Self);

    /** \brief Get the VTK prop wrapping the GL mapper for the given renderer.
     *
     * \param[in] renderer The renderer context.
     * \return A vtkGLMapperProp that delegates rendering to the wrapped GLMapper.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Generate rendering data for the given renderer.
     * \param[in] renderer The renderer to generate data for.
     */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /** \brief Per-renderer local storage holding the vtkGLMapperProp. */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;
      vtkSmartPointer<vtkGLMapperProp> m_GLMapperProp; /**< \brief The VTK prop wrapping the GL mapper. */
    };

    /** \copydoc VtkMapper::ApplyColorAndOpacityProperties */
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor) override;

    /** \brief Perform rendering by delegating to the wrapped GLMapper.
     *
     * \param[in] renderer The renderer context.
     * \param[in] type     The render pass type.
     */
    void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    /** \brief Update the mapper state for the given renderer.
     * \param[in] renderer The renderer to update for.
     */
    void Update(BaseRenderer *renderer) override;

    /** \brief Set the data node on both this wrapper and the wrapped GLMapper.
     * \param[in] node The data node to set.
     */
    void SetDataNode(DataNode *node) override;

    /** \brief Get the data node from the wrapped GLMapper.
     * \return The data node.
     */
    DataNode *GetDataNode() const override;

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

  protected:
    GLMapper::Pointer m_MitkGLMapper;
    /* constructor */
    VtkGLMapperWrapper(GLMapper::Pointer mitkGLMapper);

    /* destructor */
    ~VtkGLMapperWrapper() override;
    void Enable2DOpenGL(mitk::BaseRenderer *);
    void Disable2DOpenGL();
  };

} // namespace mitk

#endif
