/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceVtkMapper3D.h>
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkExtractSliceFilter.h>
#include <mitkFloatPropertyExtension.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyExtensions.h>
#include <mitkIntPropertyExtension.h>
#include <mitkImageSliceSelector.h>
#include <mitkLookupTableProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSmartPointerProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkScalarModeProperty.h>

// VTK
#include <vtkActor.h>
#include <vtkPlaneCollection.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkShaderProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkUniforms.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <optional>
#include <string>

namespace
{
  constexpr const char* PULSE_PROPERTY = "animated.pulse";
  constexpr const char* PULSE_FREQUENCY_PROPERTY = "animated.pulse.frequency";
  constexpr const char* COLOR_PROPERTY = "animated.color";
  constexpr const char* COLOR_FREQUENCY_PROPERTY = "animated.color.frequency";
  constexpr const char* SPIN_PROPERTY = "animated.spin";
  constexpr const char* SPIN_FREQUENCY_PROPERTY = "animated.spin.frequency";
  constexpr const char* SPIN_AXIS_PROPERTY = "animated.spin.axis";
  constexpr const char* BOUNCE_PROPERTY = "animated.bounce";
  constexpr const char* BOUNCE_FREQUENCY_PROPERTY = "animated.bounce.frequency";
  constexpr const char* BOUNCE_HEIGHT_PROPERTY = "animated.bounce.height";
  constexpr const char* BOUNCE_AXIS_PROPERTY = "animated.bounce.axis";

  constexpr float DEFAULT_PULSE_FREQUENCY = 1.5f;
  constexpr float DEFAULT_COLOR_FREQUENCY = 2.0f;
  constexpr float DEFAULT_SPIN_FREQUENCY = 0.25f;
  constexpr float DEFAULT_BOUNCE_FREQUENCY = 1.0f;
  constexpr float DEFAULT_BOUNCE_HEIGHT = 0.5f;
  constexpr int DEFAULT_AXIS = 2;

  constexpr const char* PULSE_UNIFORM = "mitkPulse";
  constexpr const char* TINT_UNIFORM = "mitkTint";

  // A cosine between 0.5 and 1.0, the factor on the lit color of a pulsing surface.
  float GetPulse(double time, double frequency)
  {
    return static_cast<float>(0.75 + 0.25 * std::cos(2.0 * std::numbers::pi * frequency * time));
  }

  // The tint on the lit color of a color-cycling surface: a point that runs around a hue
  // wheel of constant luminance. Pure hues would not do, since blue is only a tenth as
  // bright as yellow and turns the surface nearly black. Offsets along the Cb and Cr axes
  // of BT.709 YCbCr carry no luminance, so every tint on the wheel darkens the surface by
  // the same amount, and each is as colorful as the gamut allows.
  std::array<float, 3> GetCycledTint(double time, double frequency)
  {
    constexpr double luminance = 0.75;

    const double angle = 2.0 * std::numbers::pi * frequency * time;
    const double cr = std::cos(angle);
    const double cb = std::sin(angle);

    // The RGB direction of the offset, from the YCbCr to RGB conversion without its Y term.
    const double direction[3] = { 1.5748 * cr, -0.1873 * cb - 0.4681 * cr, 1.8556 * cb };

    // The largest step along it that keeps every channel within [0, 1].
    double amplitude = 1.0;

    for (const double d : direction)
    {
      if (d > 0.0)
        amplitude = std::min(amplitude, (1.0 - luminance) / d);
      else if (d < 0.0)
        amplitude = std::min(amplitude, luminance / -d);
    }

    std::array<float, 3> tint;

    for (int i = 0; i < 3; ++i)
      tint[i] = static_cast<float>(luminance + amplitude * direction[i]);

    return tint;
  }

  // The fraction of the current cycle, in [0, 1) for any sign of time and frequency.
  double GetCyclePhase(double time, double frequency)
  {
    const double cycles = frequency * time;
    return cycles - std::floor(cycles);
  }

  // The parabolic arc of a ball thrown up from its rest position, which it returns to at the end of each cycle.
  double GetBounceOffset(double time, double frequency, double height)
  {
    const double phase = GetCyclePhase(time, frequency);
    return 4.0 * height * phase * (1.0 - phase);
  }

  // The geometry axis an IntProperty selects: 0 for x, 1 for y, 2 for z. None for any other value.
  std::optional<unsigned int> GetAxis(const mitk::DataNode *node, const char *propertyKey, const mitk::BaseRenderer *renderer)
  {
    int axis = DEFAULT_AXIS;
    node->GetIntProperty(propertyKey, axis, renderer);

    if (axis < 0 || 2 < axis)
      return std::nullopt;

    return static_cast<unsigned int>(axis);
  }

  // The world direction of a geometry axis.
  std::array<double, 3> GetAxisDirection(const mitk::BaseGeometry *geometry, unsigned int axis)
  {
    // The column is as long as the spacing along the axis.
    auto direction = geometry->GetMatrixColumn(axis);
    direction.normalize();

    return { direction[0], direction[1], direction[2] };
  }
}

const mitk::Surface *mitk::SurfaceVtkMapper3D::GetInput()
{
  return static_cast<const mitk::Surface *>(GetDataNode()->GetData());
}

mitk::SurfaceVtkMapper3D::SurfaceVtkMapper3D()
{
  m_GenerateNormals = false;
}

mitk::SurfaceVtkMapper3D::~SurfaceVtkMapper3D()
{
}

void mitk::SurfaceVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
  {
    ls->m_Actor->VisibilityOff();
    return;
  }

  //
  // set the input-object at time t for the mapper
  //
  mitk::Surface::ConstPointer input = this->GetInput();

  const auto* worldGeometry = renderer->GetWorldTimeGeometry();
  const auto timeBounds = worldGeometry->GetTimeBounds(renderer->GetTimeStep());

  if (!input->GetTimeGeometry()->IsValidTimePoint(timeBounds[0]))
  {
    ls->m_Actor->VisibilityOff();
    return;
  }

  vtkSmartPointer<vtkPolyData> polydata = input->GetVtkPolyData(this->GetTimestep());
  if (polydata == nullptr)
  {
    ls->m_Actor->VisibilityOff();
    return;
  }
  if (m_GenerateNormals)
  {
    ls->m_VtkPolyDataNormals->SetInputData(polydata);
    ls->m_VtkPolyDataMapper->SetInputConnection(ls->m_VtkPolyDataNormals->GetOutputPort());
  }
  else
  {
    bool depthsorting = false;
    GetDataNode()->GetBoolProperty("Depth Sorting", depthsorting);

    if (depthsorting)
    {
      ls->m_DepthSort->SetInputData(polydata);
      ls->m_DepthSort->SetCamera(renderer->GetVtkRenderer()->GetActiveCamera());
      ls->m_DepthSort->SetDirectionToBackToFront();
      ls->m_DepthSort->Update();
      ls->m_VtkPolyDataMapper->SetInputConnection(ls->m_DepthSort->GetOutputPort());
    }
    else
    {
      ls->m_VtkPolyDataMapper->SetInputData(polydata);
    }
  }

  //
  // apply properties read from the PropertyList
  //
  ApplyAllProperties(renderer, ls->m_Actor);

  if (visible)
    ls->m_Actor->VisibilityOn();
}

void mitk::SurfaceVtkMapper3D::Update(mitk::BaseRenderer *renderer)
{
  Superclass::Update(renderer);

  const auto *node = this->GetDataNode();

  bool pulse = false;
  bool colorCycle = false;
  bool spin = false;
  bool bounce = false;
  node->GetBoolProperty(PULSE_PROPERTY, pulse, renderer);
  node->GetBoolProperty(COLOR_PROPERTY, colorCycle, renderer);
  node->GetBoolProperty(SPIN_PROPERTY, spin, renderer);
  node->GetBoolProperty(BOUNCE_PROPERTY, bounce, renderer);

  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  auto *renderingManager = RenderingManager::GetInstance();

  // The actor is invisible whenever there is nothing to draw, see GenerateDataForRenderer().
  if ((pulse || colorCycle || spin || bounce) && ls->m_Actor->GetVisibility())
    renderingManager->RequestAnimationFrame(renderer->GetRenderWindow());

  // A surface that never pulsed or cycled its color keeps the standard shader.
  if (!pulse && !colorCycle && !ls->m_HasAnimationShader)
    return;

  // Nothing on the vtkProperty may animate: vtkOpenGLPolyDataMapper rebuilds its buffers
  // whenever the property changes. A custom uniform is declared once and afterwards only
  // changes its value, which rebuilds neither the buffers nor the shader.
  auto *shaderProperty = ls->m_Actor->GetShaderProperty();

  if (!ls->m_HasAnimationShader)
  {
    // After the standard replacements, which write the lit color before this tag.
    shaderProperty->AddFragmentShaderReplacement("//VTK::Light::Impl", false,
      std::string("//VTK::Light::Impl\n  gl_FragData[0].rgb *= ") + PULSE_UNIFORM + " * " + TINT_UNIFORM + ";\n", false);

    ls->m_HasAnimationShader = true;
  }

  const double time = renderingManager->GetAnimationTime();

  float pulseFrequency = DEFAULT_PULSE_FREQUENCY;
  float colorFrequency = DEFAULT_COLOR_FREQUENCY;
  node->GetFloatProperty(PULSE_FREQUENCY_PROPERTY, pulseFrequency, renderer);
  node->GetFloatProperty(COLOR_FREQUENCY_PROPERTY, colorFrequency, renderer);

  auto *uniforms = shaderProperty->GetFragmentCustomUniforms();
  uniforms->SetUniformf(PULSE_UNIFORM, pulse ? GetPulse(time, pulseFrequency) : 1.0f);

  const auto tint = colorCycle ? GetCycledTint(time, colorFrequency) : std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
  uniforms->SetUniform3f(TINT_UNIFORM, tint.data());
}

void mitk::SurfaceVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  const auto *node = this->GetDataNode();

  bool spin = false;
  bool bounce = false;
  node->GetBoolProperty(SPIN_PROPERTY, spin, renderer);
  node->GetBoolProperty(BOUNCE_PROPERTY, bounce, renderer);

  // Nothing moves along an axis out of range.
  const auto spinAxis = GetAxis(node, SPIN_AXIS_PROPERTY, renderer);
  const auto bounceAxis = GetAxis(node, BOUNCE_AXIS_PROPERTY, renderer);
  spin = spin && spinAxis.has_value();
  bounce = bounce && bounceAxis.has_value();

  // None at a time step the surface does not have.
  const auto *geometry = this->GetInput()->GetGeometry(this->GetTimestep());

  if ((!spin && !bounce) || nullptr == geometry)
  {
    Superclass::UpdateVtkTransform(renderer);
    return;
  }

  float spinFrequency = DEFAULT_SPIN_FREQUENCY;
  float bounceFrequency = DEFAULT_BOUNCE_FREQUENCY;
  float bounceHeight = DEFAULT_BOUNCE_HEIGHT;
  node->GetFloatProperty(SPIN_FREQUENCY_PROPERTY, spinFrequency, renderer);
  node->GetFloatProperty(BOUNCE_FREQUENCY_PROPERTY, bounceFrequency, renderer);
  node->GetFloatProperty(BOUNCE_HEIGHT_PROPERTY, bounceHeight, renderer);

  const double time = RenderingManager::GetInstance()->GetAnimationTime();

  // Both happen along the axes of the geometry, but in world coordinates, after the geometry placed
  // the surface. The spin turns around an axis through the center at rest, the bounce starts there.
  const auto center = geometry->GetCenter();

  // A matrix of its own: the geometry's transform is shared with every other user of the data.
  auto *transform = m_LSH.GetLocalStorage(renderer)->m_AnimationTransform.GetPointer();
  transform->Identity();
  transform->PostMultiply();
  transform->Concatenate(geometry->GetVtkTransform()->GetMatrix());

  if (spin)
  {
    const auto direction = GetAxisDirection(geometry, *spinAxis);

    transform->Translate(-center[0], -center[1], -center[2]);
    transform->RotateWXYZ(360.0 * GetCyclePhase(time, spinFrequency), direction.data());
    transform->Translate(center[0], center[1], center[2]);
  }

  if (bounce)
  {
    const auto direction = GetAxisDirection(geometry, *bounceAxis);

    // The height is relative to the extent of the bounding box along the axis.
    const double offset = GetBounceOffset(time, bounceFrequency, bounceHeight * geometry->GetExtentInMM(*bounceAxis));

    transform->Translate(offset * direction[0], offset * direction[1], offset * direction[2]);
  }

  m_LSH.GetLocalStorage(renderer)->m_Actor->SetUserTransform(transform);
}

void mitk::SurfaceVtkMapper3D::ResetMapper(BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_Actor->VisibilityOff();
}

void mitk::SurfaceVtkMapper3D::ApplyMitkPropertiesToVtkProperty(mitk::DataNode *node,
                                                                vtkProperty *property,
                                                                mitk::BaseRenderer *renderer)
{
  // Backface culling
  {
    mitk::BoolProperty::Pointer p;
    node->GetProperty(p, "Backface Culling", renderer);
    bool useCulling = false;
    if (p.IsNotNull())
      useCulling = p->GetValue();
    property->SetBackfaceCulling(useCulling);
  }

  // Colors
  {
    double ambient[3] = {0.5, 0.5, 0.0};
    double diffuse[3] = {0.5, 0.5, 0.0};
    double specular[3] = {1.0, 1.0, 1.0};

    float coeff_ambient = 0.5f;
    float coeff_diffuse = 0.5f;
    float coeff_specular = 0.5f;
    float power_specular = 10.0f;

    // Color
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "color", renderer);
      if (p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        ambient[0] = c.GetRed();
        ambient[1] = c.GetGreen();
        ambient[2] = c.GetBlue();
        diffuse[0] = c.GetRed();
        diffuse[1] = c.GetGreen();
        diffuse[2] = c.GetBlue();
        // Setting specular color to the same, make physically no real sense, however vtk rendering slows down, if these
        // colors are different.
        specular[0] = c.GetRed();
        specular[1] = c.GetGreen();
        specular[2] = c.GetBlue();
      }
    }

    // Ambient
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.ambientColor", renderer);
      if (p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        ambient[0] = c.GetRed();
        ambient[1] = c.GetGreen();
        ambient[2] = c.GetBlue();
      }
    }

    // Diffuse
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.diffuseColor", renderer);
      if (p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        diffuse[0] = c.GetRed();
        diffuse[1] = c.GetGreen();
        diffuse[2] = c.GetBlue();
      }
    }

    // Specular
    {
      mitk::ColorProperty::Pointer p;
      node->GetProperty(p, "material.specularColor", renderer);
      if (p.IsNotNull())
      {
        mitk::Color c = p->GetColor();
        specular[0] = c.GetRed();
        specular[1] = c.GetGreen();
        specular[2] = c.GetBlue();
      }
    }

    // Ambient coeff
    {
      node->GetFloatProperty("material.ambientCoefficient", coeff_ambient, renderer);
    }

    // Diffuse coeff
    {
      node->GetFloatProperty("material.diffuseCoefficient", coeff_diffuse, renderer);
    }

    // Specular coeff
    {
      node->GetFloatProperty("material.specularCoefficient", coeff_specular, renderer);
    }

    // Specular power
    {
      node->GetFloatProperty("material.specularPower", power_specular, renderer);
    }

    property->SetAmbient(coeff_ambient);
    property->SetDiffuse(coeff_diffuse);
    property->SetSpecular(coeff_specular);
    property->SetSpecularPower(power_specular);

    property->SetAmbientColor(ambient);
    property->SetDiffuseColor(diffuse);
    property->SetSpecularColor(specular);
  }

  // Render mode
  {
    // Opacity
    {
      float opacity = 1.0f;
      if (node->GetOpacity(opacity, renderer))
        property->SetOpacity(opacity);
    }

    // Wireframe line width
    {
      float lineWidth = 1;
      node->GetFloatProperty("material.wireframeLineWidth", lineWidth, renderer);
      property->SetLineWidth(lineWidth);
    }

    // Point size
    {
      float pointSize = 1.0f;
      node->GetFloatProperty("material.pointSize", pointSize, renderer);
      property->SetPointSize(pointSize);
    }

    // Representation
    {
      mitk::VtkRepresentationProperty::Pointer p;
      node->GetProperty(p, "material.representation", renderer);
      if (p.IsNotNull())
        property->SetRepresentation(p->GetVtkRepresentation());
    }

    // Interpolation
    {
      mitk::VtkInterpolationProperty::Pointer p;
      node->GetProperty(p, "material.interpolation", renderer);
      if (p.IsNotNull())
        property->SetInterpolation(p->GetVtkInterpolation());
    }
  }
}

void mitk::SurfaceVtkMapper3D::ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor * /*actor*/)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  Superclass::ApplyColorAndOpacityProperties(renderer, ls->m_Actor);
  // VTK Properties
  ApplyMitkPropertiesToVtkProperty(this->GetDataNode(), ls->m_Actor->GetProperty(), renderer);

  mitk::TransferFunctionProperty::Pointer transferFuncProp;
  this->GetDataNode()->GetProperty(transferFuncProp, "Surface.TransferFunction", renderer);
  if (transferFuncProp.IsNotNull())
  {
    ls->m_VtkPolyDataMapper->SetLookupTable(transferFuncProp->GetValue()->GetColorTransferFunction());
  }

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if (lookupTableProp.IsNotNull())
  {
    ls->m_VtkPolyDataMapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  mitk::LevelWindow levelWindow;
  if (this->GetDataNode()->GetLevelWindow(levelWindow, renderer, "levelWindow"))
  {
    ls->m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  }
  else if (this->GetDataNode()->GetLevelWindow(levelWindow, renderer))
  {
    ls->m_VtkPolyDataMapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  }

  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  ls->m_VtkPolyDataMapper->SetScalarVisibility((scalarVisibility ? 1 : 0));

  if (scalarVisibility)
  {
    mitk::VtkScalarModeProperty *scalarMode;
    if (this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer))
      ls->m_VtkPolyDataMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
    else
      ls->m_VtkPolyDataMapper->SetScalarModeToDefault();

    bool colorMode = false;
    this->GetDataNode()->GetBoolProperty("color mode", colorMode);
    ls->m_VtkPolyDataMapper->SetColorMode((colorMode ? 1 : 0));

    double scalarsMin = 0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

    double scalarsMax = 1.0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

    ls->m_VtkPolyDataMapper->SetScalarRange(scalarsMin, scalarsMax);
  }

  mitk::SmartPointerProperty::Pointer imagetextureProp =
    dynamic_cast<mitk::SmartPointerProperty *>(GetDataNode()->GetProperty("Surface.Texture", renderer));

  if (imagetextureProp.IsNotNull())
  {
    mitk::Image *miktTexture = dynamic_cast<mitk::Image *>(imagetextureProp->GetSmartPointer().GetPointer());
    vtkSmartPointer<vtkTexture> vtkTxture = vtkSmartPointer<vtkTexture>::New();
    // Either select the first slice of a volume
    if (miktTexture->GetDimension(2) > 1)
    {
      MITK_WARN << "3D Textures are not supported by VTK and MITK. The first slice of the volume will be used instead!";
      mitk::ImageSliceSelector::Pointer sliceselector = mitk::ImageSliceSelector::New();
      sliceselector->SetSliceNr(0);
      sliceselector->SetChannelNr(0);
      sliceselector->SetTimeNr(0);
      sliceselector->SetInput(miktTexture);
      sliceselector->Update();
      vtkTxture->SetInputData(sliceselector->GetOutput()->GetVtkImageData());
    }
    else // or just use the 2D image
    {
      vtkTxture->SetInputData(miktTexture->GetVtkImageData());
    }
    // pass the texture to the actor
    ls->m_Actor->SetTexture(vtkTxture);
    if (ls->m_VtkPolyDataMapper->GetInput()->GetPointData()->GetTCoords() == nullptr)
    {
      MITK_ERROR << "Surface.Texture property was set, but there are no texture coordinates. Please provide texture "
                    "coordinates for the vtkPolyData via vtkPolyData->GetPointData()->SetTCoords().";
    }
    // if no texture is set, this will also remove a previously used texture
    // and reset the actor to it's default behaviour
  }
  else
  {
    ls->m_Actor->SetTexture(nullptr);
  }

  // deprecated settings
  bool deprecatedUseCellData = false;
  this->GetDataNode()->GetBoolProperty("deprecated useCellDataForColouring", deprecatedUseCellData);

  bool deprecatedUsePointData = false;
  this->GetDataNode()->GetBoolProperty("deprecated usePointDataForColouring", deprecatedUsePointData);

  if (deprecatedUseCellData)
  {
    ls->m_VtkPolyDataMapper->SetColorModeToDefault();
    ls->m_VtkPolyDataMapper->SetScalarRange(0, 255);
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_VtkPolyDataMapper->SetScalarModeToUseCellData();
    ls->m_Actor->GetProperty()->SetSpecular(1);
    ls->m_Actor->GetProperty()->SetSpecularPower(50);
    ls->m_Actor->GetProperty()->SetInterpolationToPhong();
  }
  else if (deprecatedUsePointData)
  {
    float scalarsMin = 0;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum")) != nullptr)
      scalarsMin =
        dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMinimum"))->GetValue();

    float scalarsMax = 0.1;
    if (dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum")) != nullptr)
      scalarsMax =
        dynamic_cast<mitk::FloatProperty *>(this->GetDataNode()->GetProperty("ScalarsRangeMaximum"))->GetValue();

    ls->m_VtkPolyDataMapper->SetScalarRange(scalarsMin, scalarsMax);
    ls->m_VtkPolyDataMapper->SetColorModeToMapScalars();
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_Actor->GetProperty()->SetSpecular(1);
    ls->m_Actor->GetProperty()->SetSpecularPower(50);
    ls->m_Actor->GetProperty()->SetInterpolationToPhong();
  }

  int deprecatedScalarMode = VTK_COLOR_MODE_DEFAULT;
  if (this->GetDataNode()->GetIntProperty("deprecated scalar mode", deprecatedScalarMode, renderer))
  {
    ls->m_VtkPolyDataMapper->SetScalarMode(deprecatedScalarMode);
    ls->m_VtkPolyDataMapper->ScalarVisibilityOn();
    ls->m_Actor->GetProperty()->SetSpecular(1);
    ls->m_Actor->GetProperty()->SetSpecularPower(50);
  }

  // Check whether one or more ClippingProperty objects have been defined for
  // this node. Check both renderer specific and global property lists, since
  // properties in both should be considered.
  const PropertyList::PropertyMap *rendererProperties = this->GetDataNode()->GetPropertyList(renderer)->GetMap();
  const PropertyList::PropertyMap *globalProperties = this->GetDataNode()->GetPropertyList(nullptr)->GetMap();

  // Add clipping planes (if any)
  ls->m_ClippingPlaneCollection->RemoveAllItems();

  PropertyList::PropertyMap::const_iterator it;
  for (it = rendererProperties->begin(); it != rendererProperties->end(); ++it)
  {
    this->CheckForClippingProperty(renderer, (*it).second.GetPointer());
  }

  for (it = globalProperties->begin(); it != globalProperties->end(); ++it)
  {
    this->CheckForClippingProperty(renderer, (*it).second.GetPointer());
  }

  if (ls->m_ClippingPlaneCollection->GetNumberOfItems() > 0)
  {
    ls->m_VtkPolyDataMapper->SetClippingPlanes(ls->m_ClippingPlaneCollection);
  }
  else
  {
    ls->m_VtkPolyDataMapper->RemoveAllClippingPlanes();
  }
}

vtkProp *mitk::SurfaceVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_Actor;
}

void mitk::SurfaceVtkMapper3D::CheckForClippingProperty(mitk::BaseRenderer *renderer, mitk::BaseProperty *property)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  auto *clippingProperty = dynamic_cast<ClippingProperty *>(property);

  if ((clippingProperty != nullptr) && (clippingProperty->GetClippingEnabled()))
  {
    const Point3D &origin = clippingProperty->GetOrigin();
    const Vector3D &normal = clippingProperty->GetNormal();

    vtkSmartPointer<vtkPlane> clippingPlane = vtkSmartPointer<vtkPlane>::New();
    clippingPlane->SetOrigin(origin[0], origin[1], origin[2]);
    clippingPlane->SetNormal(normal[0], normal[1], normal[2]);

    ls->m_ClippingPlaneCollection->AddItem(clippingPlane);
  }
}

void mitk::SurfaceVtkMapper3D::SetDefaultPropertiesForVtkProperty(mitk::DataNode *node,
                                                                  mitk::BaseRenderer *renderer,
                                                                  bool overwrite)
{
  // Shading
  {
    node->AddProperty("material.wireframeLineWidth", mitk::FloatProperty::New(1.0f), renderer, overwrite);
    node->AddProperty("material.pointSize", mitk::FloatProperty::New(1.0f), renderer, overwrite);

    node->AddProperty("material.ambientCoefficient", mitk::FloatProperty::New(0.05f), renderer, overwrite);
    node->AddProperty("material.diffuseCoefficient", mitk::FloatProperty::New(0.9f), renderer, overwrite);
    node->AddProperty("material.specularCoefficient", mitk::FloatProperty::New(1.0f), renderer, overwrite);
    node->AddProperty("material.specularPower", mitk::FloatProperty::New(16.0f), renderer, overwrite);

    node->AddProperty("material.representation", mitk::VtkRepresentationProperty::New(), renderer, overwrite);
    node->AddProperty("material.interpolation", mitk::VtkInterpolationProperty::New(), renderer, overwrite);
  }
}

void mitk::SurfaceVtkMapper3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  node->AddProperty("color", mitk::ColorProperty::New(1.0f, 1.0f, 1.0f), renderer, overwrite);
  node->AddProperty("opacity", mitk::FloatProperty::New(1.0), renderer, overwrite);

  mitk::SurfaceVtkMapper3D::SetDefaultPropertiesForVtkProperty(node, renderer, overwrite); // Shading

  node->AddProperty("scalar visibility", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("color mode", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("scalar mode", mitk::VtkScalarModeProperty::New(), renderer, overwrite);
  mitk::Surface::Pointer surface = dynamic_cast<Surface *>(node->GetData());
  if (surface.IsNotNull())
  {
    if ((surface->GetVtkPolyData() != nullptr) && (surface->GetVtkPolyData()->GetPointData() != nullptr) &&
        (surface->GetVtkPolyData()->GetPointData()->GetScalars() != nullptr))
    {
      node->AddProperty("scalar visibility", mitk::BoolProperty::New(true), renderer, overwrite);
      node->AddProperty("color mode", mitk::BoolProperty::New(true), renderer, overwrite);
    }
  }

  // Backface culling
  node->AddProperty("Backface Culling", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("Depth Sorting", mitk::BoolProperty::New(false), renderer, overwrite);
  mitk::CoreServicePointer<mitk::IPropertyDescriptions> propDescService(mitk::CoreServices::GetPropertyDescriptions());
  propDescService->AddDescription(
    "Depth Sorting",
    "Enables correct rendering for transparent objects by ordering polygons according to the distance "
    "to the camera. It is not recommended to enable this property for large surfaces (rendering might "
    "be slow).");

  node->AddProperty(PULSE_PROPERTY, mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(PULSE_FREQUENCY_PROPERTY, mitk::FloatProperty::New(DEFAULT_PULSE_FREQUENCY), renderer, overwrite);
  node->AddProperty(COLOR_PROPERTY, mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(COLOR_FREQUENCY_PROPERTY, mitk::FloatProperty::New(DEFAULT_COLOR_FREQUENCY), renderer, overwrite);
  node->AddProperty(SPIN_PROPERTY, mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(SPIN_FREQUENCY_PROPERTY, mitk::FloatProperty::New(DEFAULT_SPIN_FREQUENCY), renderer, overwrite);
  node->AddProperty(SPIN_AXIS_PROPERTY, mitk::IntProperty::New(DEFAULT_AXIS), renderer, overwrite);
  node->AddProperty(BOUNCE_PROPERTY, mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(BOUNCE_FREQUENCY_PROPERTY, mitk::FloatProperty::New(DEFAULT_BOUNCE_FREQUENCY), renderer, overwrite);
  node->AddProperty(BOUNCE_HEIGHT_PROPERTY, mitk::FloatProperty::New(DEFAULT_BOUNCE_HEIGHT), renderer, overwrite);
  node->AddProperty(BOUNCE_AXIS_PROPERTY, mitk::IntProperty::New(DEFAULT_AXIS), renderer, overwrite);

  // The editors of the Properties view, which otherwise start at 0 and end at 100.
  mitk::CoreServicePointer<mitk::IPropertyExtensions> propExtService(mitk::CoreServices::GetPropertyExtensions());
  propExtService->AddExtension(SPIN_FREQUENCY_PROPERTY, mitk::FloatPropertyExtension::New(-10.0f, 10.0f, 0.05f));
  propExtService->AddExtension(SPIN_AXIS_PROPERTY, mitk::IntPropertyExtension::New(0, 2));
  propExtService->AddExtension(BOUNCE_HEIGHT_PROPERTY, mitk::FloatPropertyExtension::New(-10.0f, 10.0f, 0.05f));
  propExtService->AddExtension(BOUNCE_AXIS_PROPERTY, mitk::IntPropertyExtension::New(0, 2));

  propDescService->AddDescription(PULSE_PROPERTY, "Lets the surface pulse in 3D.");
  propDescService->AddDescription(PULSE_FREQUENCY_PROPERTY, "Pulses per second.");
  propDescService->AddDescription(COLOR_PROPERTY, "Tints the surface in 3D with a color that runs around a hue wheel.");
  propDescService->AddDescription(COLOR_FREQUENCY_PROPERTY, "Turns around the hue wheel per second.");
  propDescService->AddDescription(SPIN_PROPERTY, "Spins the surface in 3D around an axis through its center.");
  propDescService->AddDescription(SPIN_FREQUENCY_PROPERTY, "Turns per second. Negative values spin the other way.");
  propDescService->AddDescription(SPIN_AXIS_PROPERTY, "The axis of the surface's geometry it spins around: 0 for x, 1 for y, 2 for z.");
  propDescService->AddDescription(BOUNCE_PROPERTY, "Lets the surface bounce in 3D like a ball, from where it is.");
  propDescService->AddDescription(BOUNCE_FREQUENCY_PROPERTY, "Bounces per second.");
  propDescService->AddDescription(BOUNCE_HEIGHT_PROPERTY,
    "How far the surface bounces, relative to its extent along the axis. Negative values bounce the other way.");
  propDescService->AddDescription(BOUNCE_AXIS_PROPERTY, "The axis of the surface's geometry it bounces along: 0 for x, 1 for y, 2 for z.");

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
