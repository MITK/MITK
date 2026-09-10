/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkMapper_h
#define mitkVtkMapper_h

#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkLocalStorageHandler.h>
#include <mitkMapper.h>
#include <mitkVtkPropRenderer.h>
#include <MitkCoreExports.h>
#include <mitkCoreServices.h>

#include <vtkActor.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3D.h>
#include <vtkProp3DCollection.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>

class vtkProp;
class vtkProp3D;
class vtkActor;

namespace mitk
{
  /**
   * \brief Base class of all VTK-based Mappers for displaying primitives using VTK functionality.
   *
   * Rendering of opaque, translucent or volumetric geometry and overlays
   * is done in consecutive render passes. Subclasses must implement GetVtkProp()
   * to provide the VTK prop (actor, assembly, etc.) for each BaseRenderer.
   *
   * \sa Mapper
   * \sa ImageVtkMapper2D
   * \sa SurfaceVtkMapper2D
   * \sa SurfaceVtkMapper3D
   * \ingroup Mapper
   */
  class MITKCORE_EXPORT VtkMapper : public Mapper
  {
  public:
    mitkClassMacro(VtkMapper, Mapper);

    /**
     * \brief Return the VTK prop (actor, assembly, etc.) for the given renderer.
     *
     * Each subclass must implement this to provide its rendering representation.
     *
     * \param[in] renderer The renderer for which the prop is requested.
     * \return Pointer to the vtkProp used for rendering.
     */
    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) = 0;

    /**
     * \brief Determine the render pass type and call the appropriate render method.
     *
     * Dispatches to MitkRenderOpaqueGeometry(), MitkRenderTranslucentGeometry(),
     * MitkRenderOverlay(), or MitkRenderVolumetricGeometry() based on the type.
     * Called by mitk::VtkPropRenderer::Render.
     *
     * \param[in] renderer The renderer to render into.
     * \param[in] type The render pass type (Opaque, Translucent, Overlay, Volumetric).
     */
    void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    /**
     * \brief Check visibility and render the overlay pass.
     * \param[in] renderer The renderer to render into.
     */
    virtual void MitkRenderOverlay(BaseRenderer *renderer);

    /**
     * \brief Check visibility and render opaque (untransparent) geometry.
     * \param[in] renderer The renderer to render into.
     */
    virtual void MitkRenderOpaqueGeometry(BaseRenderer *renderer);

    /**
     * \brief Check visibility and render translucent (transparent) geometry.
     * \param[in] renderer The renderer to render into.
     */
    virtual void MitkRenderTranslucentGeometry(BaseRenderer *renderer);

    /**
     * \brief Check visibility and render volumetric geometry.
     * \param[in] renderer The renderer to render into.
     */
    virtual void MitkRenderVolumetricGeometry(BaseRenderer *renderer);

    /**
     * \brief Check whether this mapper owns the specified vtkProp for the given renderer.
     *
     * Default implementation compares the given prop pointer with the result of GetVtkProp().
     *
     * \param[in] prop The vtkProp to check ownership of.
     * \param[in] renderer The renderer context.
     * \return true if this mapper owns the given prop, false otherwise.
     */
    virtual bool HasVtkProp(const vtkProp *prop, BaseRenderer *renderer);

    /** \brief Set the vtkTransform of the m_Prop3D for
    * the current time step of \a renderer
    *
    * Called by mitk::VtkPropRenderer::Update before rendering. This
    * method will transform all actors (e.g. of an vtkAssembly) according
    * the geometry.
    *
    * \warning This method transforms only props which derive
    * from vtkProp3D. Make sure to use vtkAssembly, if you have
    * multiple props. vtkPropAssembly does not work, since it derives
    * from vtkProp.
    */
    virtual void UpdateVtkTransform(mitk::BaseRenderer *renderer);

    /**
     * \brief Apply color and opacity properties read from the PropertyList to the given vtkActor.
     *
     * Reads the "color" and "opacity" properties from the DataNode and applies them
     * to the actor's vtkProperty.
     *
     * \param[in] renderer The renderer whose property list is queried.
     * \param[in] actor The vtkActor to apply color and opacity to.
     */
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor) override;

    /**
     * \brief Release VTK-based graphics resources consumed by this mapper.
     *
     * Called by mitk::VtkPropRenderer. Subclasses should override this to release
     * renderer-specific VTK resources (textures, framebuffers, etc.).
     *
     */
    virtual void ReleaseGraphicsResources(mitk::BaseRenderer * /*renderer*/) {}

    /** \brief Empty LocalStorage subclass for VtkMapper. */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    };

  protected:
    /** constructor */
    VtkMapper();

    /** virtual destructor in order to derive from this class */
    ~VtkMapper() override;

    /**
     * \brief Applies the opacity and pins the VTK render pass to it.
     *
     * Textured image actors are rendered opaque unless their opacity is
     * below 1, regardless of texture alpha. Pinning the pass also keeps
     * VTK from scanning every texel of the texture for translucency on
     * each update.
     */
    static void SetOpacityAndRenderPass(vtkActor *actor, double opacity);

  private:
    /** copy constructor */
    VtkMapper(const VtkMapper &);

    /** assignment operator */
    VtkMapper &operator=(const VtkMapper &);
  };
} // namespace mitk
#endif
