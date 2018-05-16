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

#include "mitkSurfaceVtkMapper2D.h"

// MITK includes
#include "mitkVtkPropRenderer.h"
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkLookupTableProperty.h>
#include <mitkProperties.h>
#include <mitkSurface.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkScalarModeProperty.h>

// VTK includes
#include <vtkActor.h>
#include <vtkArrowSource.h>
#include <vtkAssembly.h>
#include <vtkCutter.h>
#include <vtkGlyph3D.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkReverseSense.h>
#include <vtkTransformPolyDataFilter.h>

// constructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Mapper->ScalarVisibilityOff();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_PropAssembly = vtkSmartPointer<vtkAssembly>::New();
  m_PropAssembly->AddPart(m_Actor);
  m_CuttingPlane = vtkSmartPointer<vtkPlane>::New();
  m_Cutter = vtkSmartPointer<vtkCutter>::New();
  m_Cutter->SetCutFunction(m_CuttingPlane);
  m_Mapper->SetInputConnection(m_Cutter->GetOutputPort());

  m_NormalGlyph = vtkSmartPointer<vtkGlyph3D>::New();

  m_InverseNormalGlyph = vtkSmartPointer<vtkGlyph3D>::New();

  // Source for the glyph filter
  m_ArrowSource = vtkSmartPointer<vtkArrowSource>::New();
  // set small default values for fast rendering
  m_ArrowSource->SetTipRadius(0.05);
  m_ArrowSource->SetTipLength(0.20);
  m_ArrowSource->SetTipResolution(5);
  m_ArrowSource->SetShaftResolution(5);
  m_ArrowSource->SetShaftRadius(0.01);

  m_NormalGlyph->SetSourceConnection(m_ArrowSource->GetOutputPort());
  m_NormalGlyph->SetVectorModeToUseNormal();
  m_NormalGlyph->OrientOn();

  m_InverseNormalGlyph->SetSourceConnection(m_ArrowSource->GetOutputPort());
  m_InverseNormalGlyph->SetVectorModeToUseNormal();
  m_InverseNormalGlyph->OrientOn();

  m_NormalMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_NormalMapper->SetInputConnection(m_NormalGlyph->GetOutputPort());
  m_NormalMapper->ScalarVisibilityOff();

  m_InverseNormalMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_InverseNormalMapper->SetInputConnection(m_NormalGlyph->GetOutputPort());
  m_InverseNormalMapper->ScalarVisibilityOff();

  m_NormalActor = vtkSmartPointer<vtkActor>::New();
  m_NormalActor->SetMapper(m_NormalMapper);

  m_InverseNormalActor = vtkSmartPointer<vtkActor>::New();
  m_InverseNormalActor->SetMapper(m_InverseNormalMapper);

  m_ReverseSense = vtkSmartPointer<vtkReverseSense>::New();
}

// destructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::~LocalStorage()
{
}

const mitk::Surface *mitk::SurfaceVtkMapper2D::GetInput() const
{
  return static_cast<const mitk::Surface *>(GetDataNode()->GetData());
}

// constructor PointSetVtkMapper2D
mitk::SurfaceVtkMapper2D::SurfaceVtkMapper2D()
{
}

mitk::SurfaceVtkMapper2D::~SurfaceVtkMapper2D()
{
}

// reset mapper so that nothing is displayed e.g. toggle visiblity of the propassembly
void mitk::SurfaceVtkMapper2D::ResetMapper(BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_PropAssembly->VisibilityOff();
}

vtkProp *mitk::SurfaceVtkMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_PropAssembly;
}

void mitk::SurfaceVtkMapper2D::Update(mitk::BaseRenderer *renderer)
{
  const mitk::DataNode *node = GetDataNode();
  if (node == nullptr)
    return;
  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if (!visible)
    return;

  auto *surface = static_cast<mitk::Surface *>(node->GetData());
  if (surface == nullptr)
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const mitk::TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(this->GetTimestep())))
  {
    return;
  }

  surface->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check if something important has changed and we need to rerender
  if ((localStorage->m_LastUpdateTime < node->GetMTime()) // was the node modified?
      ||
      (localStorage->m_LastUpdateTime < surface->GetPipelineMTime()) // Was the data modified?
      ||
      (localStorage->m_LastUpdateTime <
       renderer->GetCurrentWorldPlaneGeometryUpdateTime()) // was the geometry modified?
      ||
      (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) // was a property modified?
      ||
      (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::SurfaceVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  const DataNode *node = GetDataNode();
  auto *surface = static_cast<Surface *>(node->GetData());
  const TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  ScalarType time = renderer->GetTime();
  int timestep = 0;

  if (time > itk::NumericTraits<ScalarType>::NonpositiveMin())
    timestep = dataTimeGeometry->TimePointToTimeStep(time);

  vtkSmartPointer<vtkPolyData> inputPolyData = surface->GetVtkPolyData(timestep);
  if ((inputPolyData == nullptr) || (inputPolyData->GetNumberOfPoints() < 1))
    return;

  // apply color and opacity read from the PropertyList
  this->ApplyAllProperties(renderer);

  const PlaneGeometry *planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if ((planeGeometry == nullptr) || (!planeGeometry->IsValid()) || (!planeGeometry->HasReferenceGeometry()))
  {
    return;
  }

  if (localStorage->m_Actor->GetMapper() == nullptr)
    localStorage->m_Actor->SetMapper(localStorage->m_Mapper);

  double origin[3];
  origin[0] = planeGeometry->GetOrigin()[0];
  origin[1] = planeGeometry->GetOrigin()[1];
  origin[2] = planeGeometry->GetOrigin()[2];

  double normal[3];
  normal[0] = planeGeometry->GetNormal()[0];
  normal[1] = planeGeometry->GetNormal()[1];
  normal[2] = planeGeometry->GetNormal()[2];

  localStorage->m_CuttingPlane->SetOrigin(origin);
  localStorage->m_CuttingPlane->SetNormal(normal);
  // Transform the data according to its geometry.
  // See UpdateVtkTransform documentation for details.
  vtkSmartPointer<vtkLinearTransform> vtktransform = GetDataNode()->GetVtkTransform(this->GetTimestep());
  vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  filter->SetTransform(vtktransform);
  filter->SetInputData(inputPolyData);
  localStorage->m_Cutter->SetInputConnection(filter->GetOutputPort());
  localStorage->m_Cutter->Update();

  bool generateNormals = false;
  node->GetBoolProperty("draw normals 2D", generateNormals);
  if (generateNormals)
  {
    localStorage->m_NormalGlyph->SetInputConnection(localStorage->m_Cutter->GetOutputPort());
    localStorage->m_NormalGlyph->Update();

    localStorage->m_NormalMapper->SetInputConnection(localStorage->m_NormalGlyph->GetOutputPort());

    localStorage->m_PropAssembly->AddPart(localStorage->m_NormalActor);
  }
  else
  {
    localStorage->m_NormalGlyph->SetInputConnection(nullptr);
    localStorage->m_PropAssembly->RemovePart(localStorage->m_NormalActor);
  }

  bool generateInverseNormals = false;
  node->GetBoolProperty("invert normals", generateInverseNormals);
  if (generateInverseNormals)
  {
    localStorage->m_ReverseSense->SetInputConnection(localStorage->m_Cutter->GetOutputPort());
    localStorage->m_ReverseSense->ReverseCellsOff();
    localStorage->m_ReverseSense->ReverseNormalsOn();

    localStorage->m_InverseNormalGlyph->SetInputConnection(localStorage->m_ReverseSense->GetOutputPort());
    localStorage->m_InverseNormalGlyph->Update();

    localStorage->m_InverseNormalMapper->SetInputConnection(localStorage->m_InverseNormalGlyph->GetOutputPort());

    localStorage->m_PropAssembly->AddPart(localStorage->m_InverseNormalActor);
  }
  else
  {
    localStorage->m_ReverseSense->SetInputConnection(nullptr);
    localStorage->m_PropAssembly->RemovePart(localStorage->m_InverseNormalActor);
  }
}

void mitk::SurfaceVtkMapper2D::FixupLegacyProperties(PropertyList *properties)
{
  // Before bug 18528, "line width" was an IntProperty, now it is a FloatProperty
  float lineWidth = 1.0f;
  if (!properties->GetFloatProperty("line width", lineWidth))
  {
    int legacyLineWidth = lineWidth;
    if (properties->GetIntProperty("line width", legacyLineWidth))
    {
      properties->ReplaceProperty("line width", FloatProperty::New(static_cast<float>(legacyLineWidth)));
    }
  }
}

void mitk::SurfaceVtkMapper2D::ApplyAllProperties(mitk::BaseRenderer *renderer)
{
  const DataNode *node = GetDataNode();

  if (node == nullptr)
  {
    return;
  }

  FixupLegacyProperties(node->GetPropertyList(renderer));
  FixupLegacyProperties(node->GetPropertyList());

  float lineWidth = 1.0f;
  node->GetFloatProperty("line width", lineWidth, renderer);

  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check for color and opacity properties, use it for rendering if they exists
  float color[3] = {1.0f, 1.0f, 1.0f};
  node->GetColor(color, renderer, "color");
  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  // Pass properties to VTK
  localStorage->m_Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  localStorage->m_NormalActor->GetProperty()->SetOpacity(opacity);
  localStorage->m_InverseNormalActor->GetProperty()->SetOpacity(opacity);
  localStorage->m_Actor->GetProperty()->SetLineWidth(lineWidth);
  // By default, the cutter will also copy/compute normals of the cut
  // to the output polydata. The normals will influence the
  // vtkPolyDataMapper lightning. To view a clean cut the lighting has
  // to be disabled.
  localStorage->m_Actor->GetProperty()->SetLighting(false);

  // same block for scalar data rendering as in 3D mapper
  mitk::TransferFunctionProperty::Pointer transferFuncProp;
  this->GetDataNode()->GetProperty(transferFuncProp, "Surface.TransferFunction", renderer);
  if (transferFuncProp.IsNotNull())
  {
    localStorage->m_Mapper->SetLookupTable(transferFuncProp->GetValue()->GetColorTransferFunction());
  }

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if (lookupTableProp.IsNotNull())
  {
    localStorage->m_Mapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  mitk::LevelWindow levelWindow;
  if (this->GetDataNode()->GetLevelWindow(levelWindow, renderer, "levelWindow"))
  {
    localStorage->m_Mapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  }
  else if (this->GetDataNode()->GetLevelWindow(levelWindow, renderer))
  {
    localStorage->m_Mapper->SetScalarRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  }

  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  localStorage->m_Mapper->SetScalarVisibility((scalarVisibility ? 1 : 0));

  if (scalarVisibility)
  {
    mitk::VtkScalarModeProperty *scalarMode;
    if (this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer))
      localStorage->m_Mapper->SetScalarMode(scalarMode->GetVtkScalarMode());
    else
      localStorage->m_Mapper->SetScalarModeToDefault();

    bool colorMode = false;
    this->GetDataNode()->GetBoolProperty("color mode", colorMode);
    localStorage->m_Mapper->SetColorMode((colorMode ? 1 : 0));

    double scalarsMin = 0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

    double scalarsMax = 1.0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

    localStorage->m_Mapper->SetScalarRange(scalarsMin, scalarsMax);
  }

  // color for inverse normals
  float inverseNormalsColor[3] = {1.0f, 0.0f, 0.0f};
  node->GetColor(inverseNormalsColor, renderer, "back color");
  localStorage->m_InverseNormalActor->GetProperty()->SetColor(
    inverseNormalsColor[0], inverseNormalsColor[1], inverseNormalsColor[2]);

  // color for normals
  float normalsColor[3] = {0.0f, 1.0f, 0.0f};
  node->GetColor(normalsColor, renderer, "front color");
  localStorage->m_NormalActor->GetProperty()->SetColor(normalsColor[0], normalsColor[1], normalsColor[2]);

  // normals scaling
  float normalScaleFactor = 10.0f;
  node->GetFloatProperty("front normal lenth (px)", normalScaleFactor, renderer);
  localStorage->m_NormalGlyph->SetScaleFactor(normalScaleFactor);

  // inverse normals scaling
  float inverseNormalScaleFactor = 10.0f;
  node->GetFloatProperty("back normal lenth (px)", inverseNormalScaleFactor, renderer);
  localStorage->m_InverseNormalGlyph->SetScaleFactor(inverseNormalScaleFactor);
}

void mitk::SurfaceVtkMapper2D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  mitk::IPropertyAliases *aliases = mitk::CoreServices::GetPropertyAliases();
  node->AddProperty("line width", FloatProperty::New(2.0f), renderer, overwrite);
  aliases->AddAlias("line width", "Surface.2D.Line Width", "Surface");
  node->AddProperty("scalar mode", VtkScalarModeProperty::New(), renderer, overwrite);
  node->AddProperty("draw normals 2D", BoolProperty::New(false), renderer, overwrite);
  aliases->AddAlias("draw normals 2D", "Surface.2D.Normals.Draw Normals", "Surface");
  node->AddProperty("invert normals", BoolProperty::New(false), renderer, overwrite);
  aliases->AddAlias("invert normals", "Surface.2D.Normals.Draw Inverse Normals", "Surface");
  node->AddProperty("front color", ColorProperty::New(0.0, 1.0, 0.0), renderer, overwrite);
  aliases->AddAlias("front color", "Surface.2D.Normals.Normals Color", "Surface");
  node->AddProperty("back color", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
  aliases->AddAlias("back color", "Surface.2D.Normals.Inverse Normals Color", "Surface");
  node->AddProperty("front normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite);
  aliases->AddAlias("front normal lenth (px)", "Surface.2D.Normals.Normals Scale Factor", "Surface");
  node->AddProperty("back normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite);
  aliases->AddAlias("back normal lenth (px)", "Surface.2D.Normals.Inverse Normals Scale Factor", "Surface");
  node->AddProperty("layer", IntProperty::New(100), renderer, overwrite);
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
