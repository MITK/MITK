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

#include "mitkSurfaceVtkMapper3D.h"
#include <mitkClippingProperty.h>
#include <mitkColorProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkExtractSliceFilter.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkImageSliceSelector.h>
#include <mitkLookupTableProperty.h>
#include <mitkProperties.h>
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
#include <vtkSmartPointer.h>

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
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(
    "Depth Sorting",
    "Enables correct rendering for transparent objects by ordering polygons according to the distance "
    "to the camera. It is not recommended to enable this property for large surfaces (rendering might "
    "be slow).");
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
