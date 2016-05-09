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

#ifndef mitkSurfaceVtkMapper3D_h
#define mitkSurfaceVtkMapper3D_h

#include <MitkCoreExports.h>
#include <mitkSurface.h>
#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkLocalStorageHandler.h"

#include <vtkActor.h>
#include <vtkDepthSortPolyData.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>

namespace mitk {
  /**
  * @brief Vtk-based mapper for Surfaces.
  *
  * The mapper renders a surface in 3D. The actor is adapted according to the geometry in
  * the base class in mitk::VtkMapper::UpdateVtkTransform().
  *

  * Properties that can be set for surfaces and influence the surfaceVTKMapper3D are:
  *
  *   - \b "Backface Culling": True enables backface culling, which means only front-facing polygons will be visualized. False/disabled by default.
  *   - \b "color": (ColorProperty) Diffuse color of the surface object (this property will be read when material.diffuseColor is not defined)
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

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkSetMacro(GenerateNormals, bool);

  itkGetMacro(GenerateNormals, bool);

  virtual const mitk::Surface* GetInput();

  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  virtual void ApplyAllProperties(mitk::BaseRenderer* renderer, vtkActor* actor);

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:
  SurfaceVtkMapper3D();

  virtual ~SurfaceVtkMapper3D();

  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

  virtual void ResetMapper( mitk::BaseRenderer* renderer ) override;

  /** Checks whether the specified property is a ClippingProperty and if yes,
   * adds it to m_ClippingPlaneCollection (internal method). */
  virtual void CheckForClippingProperty( mitk::BaseRenderer* renderer, mitk::BaseProperty *property );

  bool m_GenerateNormals;

public:

  class LocalStorage : public mitk::Mapper::BaseLocalStorage
  {
    public:

      vtkSmartPointer<vtkActor> m_Actor;
      vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;
      vtkSmartPointer<vtkPolyDataNormals> m_VtkPolyDataNormals;
      vtkSmartPointer<vtkPlaneCollection> m_ClippingPlaneCollection;
      vtkSmartPointer<vtkDepthSortPolyData> m_DepthSort;
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

      ~LocalStorage()
      {
      }
  };

  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  static void ApplyMitkPropertiesToVtkProperty(mitk::DataNode *node, vtkProperty* property, mitk::BaseRenderer* renderer);
  static void SetDefaultPropertiesForVtkProperty(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite);
};
} // namespace mitk

#endif /* mitkSurfaceVtkMapper3D_h */
