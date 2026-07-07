/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkMapper3D_h
#define mitkSurfaceVtkMapper3D_h

#include <mitkBaseRenderer.h>
#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>
#include <MitkCoreExports.h>
#include <mitkSurface.h>

#include <vtkActor.h>
#include <vtkDepthSortPolyData.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
  * @brief Vtk-based mapper for Surfaces.
  *
  * The mapper renders a surface in 3D. The actor is adapted according to the geometry in
  * the base class in mitk::VtkMapper::UpdateVtkTransform().
  *

  * Properties that can be set for surfaces and influence the surfaceVTKMapper3D are:
  *
  *   - \b "Backface Culling": True enables backface culling, which means only front-facing polygons will be visualized.
  False/disabled by default.
  *   - \b "color": (ColorProperty) Diffuse color of the surface object (this property will be read when
  material.diffuseColor is not defined)
  *   - \b "Opacity": (FloatProperty) Opacity of the surface object
  *   - \b "material.ambientColor": (ColorProperty) Ambient color  of the surface object
  *   - \b "material.ambientCoefficient": (  FloatProperty) Ambient coefficient of the surface object
  *   - \b "material.diffuseColor": ( ColorProperty) Diffuse color of the surface object
  *   - \b "material.diffuseCoefficient": (FloatProperty) Diffuse coefficient of the surface object
  *   - \b "material.specularColor": (ColorProperty) Specular Color of the surface object
  *   - \b "material.specularCoefficient": (FloatProperty) Specular coefficient of the surface object
  *   - \b "material.specularPower": (FloatProperty) Specular power of the surface object
  *   - \b "material.interpolation": (VtkInterpolationProperty) Interpolation
  *   - \b "material.representation": (VtkRepresentationProperty*) Representation
  *   - \b "material.wireframeLineWidth": (FloatProperty) Width in pixels of the lines drawn.
  *   - \b "material.pointSize": (FloatProperty) Size in pixels of the points drawn.
  *   - \b "scalar visibility": (BoolProperty) If the scarlars of the surface are visible
  *   - \b "Surface.TransferFunction (TransferFunctionProperty) Set a transferfunction for coloring the surface
  *   - \b "LookupTable (LookupTableProperty) LookupTable

  * Properties to look for are:
  *
  *   - \b "scalar visibility": if set to on, scalars assigned to the data are shown
  *        Turn this on if using a lookup table.
  *   - \b "ScalarsRangeMinimum": Optional. Can be used to store the scalar min, e.g.
  *         for the level window settings.
  *   - \b "ScalarsRangeMaximum": Optional. See above.
  *
  * There might be still some other, deprecated properties. These will not be documented anymore.
  * Please check the source if you really need them.
  *
  * @ingroup Mapper
  */

  class MITKCORE_EXPORT SurfaceVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(SurfaceVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Enable or disable automatic surface normal generation.
     * \param[in] _arg True to generate normals, false otherwise.
     */
    itkSetMacro(GenerateNormals, bool);

    /** \brief Get whether automatic surface normal generation is enabled.
     * \return True if normal generation is enabled.
     */
    itkGetMacro(GenerateNormals, bool);

    /**
     * \brief Get the input Surface from the associated DataNode.
     * \return Const pointer to the input mitk::Surface.
     */
    virtual const mitk::Surface *GetInput();

    /**
     * \brief Get the VTK prop (actor) for the given renderer.
     * \param[in] renderer The renderer for which the prop is requested.
     * \return Pointer to the vtkProp (vtkActor) used for 3D rendering.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /**
     * \brief Apply all material, color, opacity, and scalar visibility properties to the given actor.
     * \param[in] renderer The renderer whose property list is queried.
     * \param[in] actor The vtkActor to apply properties to.
     */
    virtual void ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor *actor);

    /**
     * \brief Set default properties for 3D surface rendering on the given DataNode.
     *
     * Initializes material properties (ambient, diffuse, specular), color, opacity,
     * scalar visibility, and related rendering options.
     *
     * \param[in] node The DataNode on which to set the properties.
     * \param[in] renderer The renderer context (nullptr for default property list).
     * \param[in] overwrite If true, overwrite existing properties.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

  protected:
    SurfaceVtkMapper3D();

    ~SurfaceVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void ResetMapper(mitk::BaseRenderer *renderer) override;

    /** Checks whether the specified property is a ClippingProperty and if yes,
     * adds it to m_ClippingPlaneCollection (internal method). */
    virtual void CheckForClippingProperty(mitk::BaseRenderer *renderer, mitk::BaseProperty *property);

    bool m_GenerateNormals;

  public:
    /**
     * \brief Per-renderer storage holding VTK objects for 3D surface rendering.
     */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief The VTK actor representing the surface in 3D. */
      vtkSmartPointer<vtkActor> m_Actor;
      /** \brief The VTK poly data mapper. */
      vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;
      /** \brief Filter for computing surface normals. */
      vtkSmartPointer<vtkPolyDataNormals> m_VtkPolyDataNormals;
      /** \brief Collection of clipping planes applied to the surface. */
      vtkSmartPointer<vtkPlaneCollection> m_ClippingPlaneCollection;
      /** \brief Filter for depth-sorting translucent polygons. */
      vtkSmartPointer<vtkDepthSortPolyData> m_DepthSort;
      /** \brief Timestamp tracking the last shader update. */
      itk::TimeStamp m_ShaderTimestampUpdate;

      LocalStorage()
      {
        m_VtkPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_VtkPolyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
        m_Actor = vtkSmartPointer<vtkActor>::New();
        m_ClippingPlaneCollection = vtkSmartPointer<vtkPlaneCollection>::New();

        m_Actor->SetMapper(m_VtkPolyDataMapper);

        m_DepthSort = vtkSmartPointer<vtkDepthSortPolyData>::New();
      }

      ~LocalStorage() override {}
    };

    /** \brief Handler managing per-renderer LocalStorage instances. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;

    /**
     * \brief Apply MITK material properties from a DataNode to a vtkProperty.
     *
     * Reads color, ambient, diffuse, specular, interpolation, representation,
     * and other properties from the DataNode and applies them to the given vtkProperty.
     *
     * \param[in] node The DataNode providing the properties.
     * \param[in] property The vtkProperty to configure.
     * \param[in] renderer The renderer context for renderer-specific properties.
     */
    static void ApplyMitkPropertiesToVtkProperty(mitk::DataNode *node,
                                                 vtkProperty *property,
                                                 mitk::BaseRenderer *renderer);

    /**
     * \brief Set default material properties for vtkProperty-based rendering.
     * \param[in] node The DataNode on which to set the properties.
     * \param[in] renderer The renderer context.
     * \param[in] overwrite If true, overwrite existing properties.
     */
    static void SetDefaultPropertiesForVtkProperty(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite);
  };
} // namespace mitk

#endif
