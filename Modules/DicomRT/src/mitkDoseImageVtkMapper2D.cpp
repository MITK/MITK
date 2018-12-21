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

// MITK
#include "mitkImageStatisticsHolder.h"
#include "mitkPlaneClipping.h"
#include "mitkPropertyNameHelper.h"
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkImageSliceSelector.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPixelType.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRTConstants.h>
#include <mitkRenderingModeProperty.h>
#include <mitkResliceMethodProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>

// MITK Rendering
#include "mitkDoseImageVtkMapper2D.h"

#include "vtkMitkLevelWindowFilter.h"
#include "vtkMitkThickSlicesFilter.h"
#include "vtkNeverTranslucentTexture.h"

// VTK
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkGeneralTransform.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageReslice.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>

// ITK
#include <itkRGBAPixel.h>

mitk::DoseImageVtkMapper2D::DoseImageVtkMapper2D()
{
}

mitk::DoseImageVtkMapper2D::~DoseImageVtkMapper2D()
{
  // The 3D RW Mapper (PlaneGeometryDataVtkMapper3D) is listening to this event,
  // in order to delete the images from the 3D RW.
  this->InvokeEvent(itk::DeleteEvent());
}

// set the two points defining the textured plane according to the dimension and spacing
void mitk::DoseImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer *renderer, double planeBounds[6])
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float depth = this->CalculateLayerDepth(renderer);
  // Set the origin to (xMin; yMin; depth) of the plane. This is necessary for obtaining the correct
  // plane size in crosshair rotation and swivel mode.
  localStorage->m_Plane->SetOrigin(planeBounds[0], planeBounds[2], depth);
  // These two points define the axes of the plane in combination with the origin.
  // Point 1 is the x-axis and point 2 the y-axis.
  // Each plane is transformed according to the view (axial, coronal and saggital) afterwards.
  localStorage->m_Plane->SetPoint1(planeBounds[1], planeBounds[2], depth); // P1: (xMax, yMin, depth)
  localStorage->m_Plane->SetPoint2(planeBounds[0], planeBounds[3], depth); // P2: (xMin, yMax, depth)
}

float mitk::DoseImageVtkMapper2D::CalculateLayerDepth(mitk::BaseRenderer *renderer)
{
  // get the clipping range to check how deep into z direction we can render images
  double maxRange = renderer->GetVtkRenderer()->GetActiveCamera()->GetClippingRange()[1];

  // Due to a VTK bug, we cannot use the whole clipping range. /100 is empirically determined
  float depth = -maxRange * 0.01; // divide by 100
  int layer = 0;
  GetDataNode()->GetIntProperty("layer", layer, renderer);
  // add the layer property for each image to render images with a higher layer on top of the others
  depth += layer * 10; //*10: keep some room for each image (e.g. for ODFs in between)
  if (depth > 0.0f)
  {
    depth = 0.0f;
    MITK_WARN << "Layer value exceeds clipping range. Set to minimum instead.";
  }
  return depth;
}

const mitk::Image *mitk::DoseImageVtkMapper2D::GetInput(void)
{
  return static_cast<const mitk::Image *>(GetDataNode()->GetData());
}

vtkProp *mitk::DoseImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  // return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

void mitk::DoseImageVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::Image *input = const_cast<mitk::Image *>(this->GetInput());
  mitk::DataNode *datanode = this->GetDataNode();

  if (input == nullptr || input->IsInitialized() == false)
  {
    return;
  }

  // check if there is a valid worldGeometry
  const PlaneGeometry *worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if ((worldGeometry == nullptr) || (!worldGeometry->IsValid()) || (!worldGeometry->HasReferenceGeometry()))
  {
    return;
  }

  input->Update();

  // early out if there is no intersection of the current rendering geometry
  // and the geometry of the image that is to be rendered.
  if (!RenderingGeometryIntersectsImage(worldGeometry, input->GetSlicedGeometry()))
  {
    // set image to nullptr, to clear the texture in 3D, because
    // the latest image is used there if the plane is out of the geometry
    // see bug-13275
    localStorage->m_ReslicedImage = nullptr;
    localStorage->m_Mapper->SetInputData(localStorage->m_EmptyPolyData);
    return;
  }

  // set main input for ExtractSliceFilter
  localStorage->m_Reslicer->SetInput(input);
  localStorage->m_Reslicer->SetWorldGeometry(worldGeometry);
  localStorage->m_Reslicer->SetTimeStep(this->GetTimestep());

  // set the transformation of the image to adapt reslice axis
  localStorage->m_Reslicer->SetResliceTransformByGeometry(
    input->GetTimeGeometry()->GetGeometryForTimeStep(this->GetTimestep()));

  // is the geometry of the slice based on the input image or the worldgeometry?
  bool inPlaneResampleExtentByGeometry = false;
  datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
  localStorage->m_Reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);

  // Initialize the interpolation mode for resampling; switch to nearest
  // neighbor if the input image is too small.
  if ((input->GetDimension() >= 3) && (input->GetDimension(2) > 1))
  {
    VtkResliceInterpolationProperty *resliceInterpolationProperty;
    datanode->GetProperty(resliceInterpolationProperty, "reslice interpolation");

    int interpolationMode = VTK_RESLICE_NEAREST;
    if (resliceInterpolationProperty != nullptr)
    {
      interpolationMode = resliceInterpolationProperty->GetInterpolation();
    }

    switch (interpolationMode)
    {
      case VTK_RESLICE_NEAREST:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
        break;
      case VTK_RESLICE_LINEAR:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_LINEAR);
        break;
      case VTK_RESLICE_CUBIC:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_CUBIC);
        break;
    }
  }
  else
  {
    localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
  }

  // set the vtk output property to true, makes sure that no unneeded mitk image convertion
  // is done.
  localStorage->m_Reslicer->SetVtkOutputRequest(true);

  // Thickslicing
  int thickSlicesMode = 0;
  int thickSlicesNum = 1;
  // Thick slices parameters
  if (input->GetPixelType().GetNumberOfComponents() == 1) // for now only single component are allowed
  {
    DataNode *dn = renderer->GetCurrentWorldPlaneGeometryNode();
    if (dn)
    {
      ResliceMethodProperty *resliceMethodEnumProperty = nullptr;

      if (dn->GetProperty(resliceMethodEnumProperty, "reslice.thickslices") && resliceMethodEnumProperty)
        thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

      IntProperty *intProperty = nullptr;
      if (dn->GetProperty(intProperty, "reslice.thickslices.num") && intProperty)
      {
        thickSlicesNum = intProperty->GetValue();
        if (thickSlicesNum < 1)
          thickSlicesNum = 1;
        if (thickSlicesNum > 10)
          thickSlicesNum = 10;
      }
    }
    else
    {
      MITK_WARN << "no associated widget plane data tree node found";
    }
  }

  const PlaneGeometry *planeGeometry = dynamic_cast<const PlaneGeometry *>(worldGeometry);

  if (thickSlicesMode > 0)
  {
    double dataZSpacing = 1.0;

    Vector3D normInIndex, normal;

    if (planeGeometry != nullptr)
    {
      normal = planeGeometry->GetNormal();
    }
    else
    {
      const mitk::AbstractTransformGeometry *abstractGeometry =
        dynamic_cast<const AbstractTransformGeometry *>(worldGeometry);
      if (abstractGeometry != nullptr)
        normal = abstractGeometry->GetPlane()->GetNormal();
      else
        return; // no fitting geometry set
    }
    normal.Normalize();

    input->GetTimeGeometry()->GetGeometryForTimeStep(this->GetTimestep())->WorldToIndex(normal, normInIndex);

    dataZSpacing = 1.0 / normInIndex.GetNorm();

    localStorage->m_Reslicer->SetOutputDimensionality(3);
    localStorage->m_Reslicer->SetOutputSpacingZDirection(dataZSpacing);
    localStorage->m_Reslicer->SetOutputExtentZDirection(-thickSlicesNum, 0 + thickSlicesNum);

    // Do the reslicing. Modified() is called to make sure that the reslicer is
    // executed even though the input geometry information did not change; this
    // is necessary when the input /em data, but not the /em geometry changes.
    localStorage->m_TSFilter->SetThickSliceMode(thickSlicesMode - 1);
    localStorage->m_TSFilter->SetInputData(localStorage->m_Reslicer->GetVtkOutput());

    // vtkFilter=>mitkFilter=>vtkFilter update mechanism will fail without calling manually
    localStorage->m_Reslicer->Modified();
    localStorage->m_Reslicer->Update();

    localStorage->m_TSFilter->Modified();
    localStorage->m_TSFilter->Update();
    localStorage->m_ReslicedImage = localStorage->m_TSFilter->GetOutput();
  }
  else
  {
    // this is needed when thick mode was enable bevore. These variable have to be reset to default values
    localStorage->m_Reslicer->SetOutputDimensionality(2);
    localStorage->m_Reslicer->SetOutputSpacingZDirection(1.0);
    localStorage->m_Reslicer->SetOutputExtentZDirection(0, 0);

    localStorage->m_Reslicer->Modified();
    // start the pipeline with updating the largest possible, needed if the geometry of the input has changed
    localStorage->m_Reslicer->UpdateLargestPossibleRegion();
    localStorage->m_ReslicedImage = localStorage->m_Reslicer->GetVtkOutput();
  }

  // Bounds information for reslicing (only reuqired if reference geometry
  // is present)
  // this used for generating a vtkPLaneSource with the right size
  double sliceBounds[6];
  for (int i = 0; i < 6; ++i)
  {
    sliceBounds[i] = 0.0;
  }
  localStorage->m_Reslicer->GetClippedPlaneBounds(sliceBounds);

  // get the spacing of the slice
  localStorage->m_mmPerPixel = localStorage->m_Reslicer->GetOutputSpacing();

  // calculate minimum bounding rect of IMAGE in texture
  {
    double textureClippingBounds[6];
    for (int i = 0; i < 6; ++i)
    {
      textureClippingBounds[i] = 0.0;
    }
    // Calculate the actual bounds of the transformed plane clipped by the
    // dataset bounding box; this is required for drawing the texture at the
    // correct position during 3D mapping.
    mitk::PlaneClipping::CalculateClippedPlaneBounds(input->GetGeometry(), planeGeometry, textureClippingBounds);

    textureClippingBounds[0] = static_cast<int>(textureClippingBounds[0] / localStorage->m_mmPerPixel[0] + 0.5);
    textureClippingBounds[1] = static_cast<int>(textureClippingBounds[1] / localStorage->m_mmPerPixel[0] + 0.5);
    textureClippingBounds[2] = static_cast<int>(textureClippingBounds[2] / localStorage->m_mmPerPixel[1] + 0.5);
    textureClippingBounds[3] = static_cast<int>(textureClippingBounds[3] / localStorage->m_mmPerPixel[1] + 0.5);

    // clipping bounds for cutting the image
    localStorage->m_LevelWindowFilter->SetClippingBounds(textureClippingBounds);
  }

  // get the number of scalar components to distinguish between different image types
  int numberOfComponents = localStorage->m_ReslicedImage->GetNumberOfScalarComponents();
  // get the showIsoLines property
  bool showIsoLines = false;
  datanode->GetBoolProperty("dose.showIsoLines", showIsoLines, renderer);

  if (showIsoLines) // contour rendering
  {
    // generate contours/outlines
    localStorage->m_OutlinePolyData = CreateOutlinePolyData(renderer);

    float binaryOutlineWidth(1.0);
    if (datanode->GetFloatProperty("outline width", binaryOutlineWidth, renderer))
    {
      if (localStorage->m_Actors->GetNumberOfPaths() > 1)
      {
        float binaryOutlineShadowWidth(1.5);
        datanode->GetFloatProperty("outline shadow width", binaryOutlineShadowWidth, renderer);

        dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))
          ->GetProperty()
          ->SetLineWidth(binaryOutlineWidth * binaryOutlineShadowWidth);
      }

      localStorage->m_Actor->GetProperty()->SetLineWidth(binaryOutlineWidth);
    }
  }
  else
  {
    localStorage->m_ReslicedImage = nullptr;
    localStorage->m_Mapper->SetInputData(localStorage->m_EmptyPolyData);
    return;
  }

  this->ApplyOpacity(renderer);
  this->ApplyRenderingMode(renderer);

  // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
  localStorage->m_Texture->SetColorModeToDirectScalars();

  int displayedComponent = 0;

  if (datanode->GetIntProperty("Image.Displayed Component", displayedComponent, renderer) && numberOfComponents > 1)
  {
    localStorage->m_VectorComponentExtractor->SetComponents(displayedComponent);
    localStorage->m_VectorComponentExtractor->SetInputData(localStorage->m_ReslicedImage);

    localStorage->m_LevelWindowFilter->SetInputConnection(localStorage->m_VectorComponentExtractor->GetOutputPort(0));
  }
  else
  {
    // connect the input with the levelwindow filter
    localStorage->m_LevelWindowFilter->SetInputData(localStorage->m_ReslicedImage);
  }

  // check for texture interpolation property
  bool textureInterpolation = false;
  GetDataNode()->GetBoolProperty("texture interpolation", textureInterpolation, renderer);

  // set the interpolation modus according to the property
  localStorage->m_Texture->SetInterpolate(textureInterpolation);

  // connect the texture with the output of the levelwindow filter
  localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowFilter->GetOutputPort());

  this->TransformActor(renderer);

  vtkActor *contourShadowActor = dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0));

  if (showIsoLines) // connect the mapper with the polyData which contains the lines
  {
    // We need the contour for the binary outline property as actor
    localStorage->m_Mapper->SetInputData(localStorage->m_OutlinePolyData);
    localStorage->m_Actor->SetTexture(nullptr); // no texture for contours

    bool binaryOutlineShadow(false);
    datanode->GetBoolProperty("outline binary shadow", binaryOutlineShadow, renderer);

    if (binaryOutlineShadow)
      contourShadowActor->SetVisibility(true);
    else
      contourShadowActor->SetVisibility(false);
  }
  else
  { // Connect the mapper with the input texture. This is the standard case.
    // setup the textured plane
    this->GeneratePlane(renderer, sliceBounds);
    // set the plane as input for the mapper
    localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
    // set the texture for the actor

    localStorage->m_Actor->SetTexture(localStorage->m_Texture);
    contourShadowActor->SetVisibility(false);
  }

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::DoseImageVtkMapper2D::ApplyLevelWindow(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);

  LevelWindow levelWindow;
  this->GetDataNode()->GetLevelWindow(levelWindow, renderer, "levelwindow");
  localStorage->m_LevelWindowFilter->GetLookupTable()->SetRange(levelWindow.GetLowerWindowBound(),
                                                                levelWindow.GetUpperWindowBound());

  mitk::LevelWindow opacLevelWindow;
  if (this->GetDataNode()->GetLevelWindow(opacLevelWindow, renderer, "opaclevelwindow"))
  {
    // pass the opaque level window to the filter
    localStorage->m_LevelWindowFilter->SetMinOpacity(opacLevelWindow.GetLowerWindowBound());
    localStorage->m_LevelWindowFilter->SetMaxOpacity(opacLevelWindow.GetUpperWindowBound());
  }
  else
  {
    // no opaque level window
    localStorage->m_LevelWindowFilter->SetMinOpacity(0.0);
    localStorage->m_LevelWindowFilter->SetMaxOpacity(255.0);
  }
}

void mitk::DoseImageVtkMapper2D::ApplyColor(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);

  float rgb[3] = {1.0f, 1.0f, 1.0f};

  // check for color prop and use it for rendering if it exists
  // binary image hovering & binary image selection
  bool hover = false;
  bool selected = false;
  GetDataNode()->GetBoolProperty("binaryimage.ishovering", hover, renderer);
  GetDataNode()->GetBoolProperty("selected", selected, renderer);
  if (hover && !selected)
  {
    mitk::ColorProperty::Pointer colorprop =
      dynamic_cast<mitk::ColorProperty *>(GetDataNode()->GetProperty("binaryimage.hoveringcolor", renderer));
    if (colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
    }
    else
    {
      GetDataNode()->GetColor(rgb, renderer, "color");
    }
  }
  if (selected)
  {
    mitk::ColorProperty::Pointer colorprop =
      dynamic_cast<mitk::ColorProperty *>(GetDataNode()->GetProperty("binaryimage.selectedcolor", renderer));
    if (colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
    }
    else
    {
      GetDataNode()->GetColor(rgb, renderer, "color");
    }
  }
  if (!hover && !selected)
  {
    GetDataNode()->GetColor(rgb, renderer, "color");
  }

  double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; // conversion to double for VTK
  dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))->GetProperty()->SetColor(rgbConv);
  localStorage->m_Actor->GetProperty()->SetColor(rgbConv);

  if (localStorage->m_Actors->GetParts()->GetNumberOfItems() > 1)
  {
    float rgb[3] = {1.0f, 1.0f, 1.0f};
    mitk::ColorProperty::Pointer colorprop =
      dynamic_cast<mitk::ColorProperty *>(GetDataNode()->GetProperty("outline binary shadow color", renderer));
    if (colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3 * sizeof(float));
    }
    double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; // conversion to double for VTK
    dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))->GetProperty()->SetColor(rgbConv);
  }
}

void mitk::DoseImageVtkMapper2D::ApplyOpacity(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);
  float opacity = 1.0f;
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity(opacity, renderer, "opacity");
  // set the opacity according to the properties
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  if (localStorage->m_Actors->GetParts()->GetNumberOfItems() > 1)
  {
    dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))
      ->GetProperty()
      ->SetOpacity(opacity);
  }
}

void mitk::DoseImageVtkMapper2D::ApplyRenderingMode(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  bool binary = false;
  this->GetDataNode()->GetBoolProperty("binary", binary, renderer);
  if (binary) // is it a binary image?
  {
    // for binary images, we always use our default LuT and map every value to (0,1)
    // the opacity of 0 will always be 0.0. We never a apply a LuT/TfF nor a level window.
    localStorage->m_LevelWindowFilter->SetLookupTable(localStorage->m_BinaryLookupTable);
  }
  else
  {
    // all other image types can make use of the rendering mode
    int renderingMode = mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR;
    mitk::RenderingModeProperty::Pointer mode =
      dynamic_cast<mitk::RenderingModeProperty *>(this->GetDataNode()->GetProperty("Image Rendering.Mode", renderer));
    if (mode.IsNotNull())
    {
      renderingMode = mode->GetRenderingMode();
    }
    switch (renderingMode)
    {
      case mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR:
        MITK_DEBUG << "'Image Rendering.Mode' = LevelWindow_LookupTable_Color";
        this->ApplyLookuptable(renderer);
        this->ApplyLevelWindow(renderer);
        break;
      case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR:
        MITK_DEBUG << "'Image Rendering.Mode' = LevelWindow_ColorTransferFunction_Color";
        this->ApplyColorTransferFunction(renderer);
        this->ApplyLevelWindow(renderer);
        break;
      case mitk::RenderingModeProperty::LOOKUPTABLE_COLOR:
        MITK_DEBUG << "'Image Rendering.Mode' = LookupTable_Color";
        this->ApplyLookuptable(renderer);
        break;
      case mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR:
        MITK_DEBUG << "'Image Rendering.Mode' = ColorTransferFunction_Color";
        this->ApplyColorTransferFunction(renderer);
        break;
      default:
        MITK_ERROR << "No valid 'Image Rendering.Mode' set. Using LOOKUPTABLE_LEVELWINDOW_COLOR instead.";
        this->ApplyLookuptable(renderer);
        this->ApplyLevelWindow(renderer);
        break;
    }
  }
  // we apply color for all images (including binaries).
  this->ApplyColor(renderer);
}

void mitk::DoseImageVtkMapper2D::ApplyLookuptable(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  vtkLookupTable *usedLookupTable = localStorage->m_ColorLookupTable;

  // If lookup table or transferfunction use is requested...
  mitk::LookupTableProperty::Pointer lookupTableProp =
    dynamic_cast<mitk::LookupTableProperty *>(this->GetDataNode()->GetProperty("LookupTable"));

  if (lookupTableProp.IsNotNull()) // is a lookuptable set?
  {
    usedLookupTable = lookupTableProp->GetLookupTable()->GetVtkLookupTable();
  }
  else
  {
    //"Image Rendering.Mode was set to use a lookup table but there is no property 'LookupTable'.
    // A default (rainbow) lookup table will be used.
    // Here have to do nothing. Warning for the user has been removed, due to unwanted console output
    // in every interation of the rendering.
  }
  localStorage->m_LevelWindowFilter->SetLookupTable(usedLookupTable);
}

void mitk::DoseImageVtkMapper2D::ApplyColorTransferFunction(mitk::BaseRenderer *renderer)
{
  mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty *>(
    this->GetDataNode()->GetProperty("Image Rendering.Transfer Function", renderer));

  if (transferFunctionProp.IsNull())
  {
    MITK_ERROR << "'Image Rendering.Mode'' was set to use a color transfer function but there is no property 'Image "
                  "Rendering.Transfer Function'. Nothing will be done.";
    return;
  }
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  // pass the transfer function to our level window filter
  localStorage->m_LevelWindowFilter->SetLookupTable(transferFunctionProp->GetValue()->GetColorTransferFunction());
}

void mitk::DoseImageVtkMapper2D::Update(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
  {
    return;
  }

  mitk::Image *data = const_cast<mitk::Image *>(this->GetInput());
  if (data == nullptr)
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = data->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(this->GetTimestep())))
  {
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check if something important has changed and we need to rerender
  if ((localStorage->m_LastUpdateTime < node->GetMTime()) // was the node modified?
      ||
      (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) // Was the data modified?
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

void mitk::DoseImageVtkMapper2D::SetDefaultProperties(mitk::DataNode *node,
                                                      mitk::BaseRenderer *renderer,
                                                      bool overwrite)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(node->GetData());

  // Properties common for both images and segmentations
  node->AddProperty("depthOffset", mitk::FloatProperty::New(0.0), renderer, overwrite);
  node->AddProperty("outline binary", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("outline width", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty("outline binary shadow", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("outline binary shadow color", ColorProperty::New(0.0, 0.0, 0.0), renderer, overwrite);
  node->AddProperty("outline shadow width", mitk::FloatProperty::New(1.5), renderer, overwrite);
  if (image->IsRotated())
    node->AddProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC));
  else
    node->AddProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New());
  node->AddProperty("texture interpolation",
                    mitk::BoolProperty::New(false)); // default value
  node->AddProperty("in plane resample extent by geometry", mitk::BoolProperty::New(false));
  node->AddProperty("bounding box", mitk::BoolProperty::New(false));

  mitk::RenderingModeProperty::Pointer renderingModeProperty = mitk::RenderingModeProperty::New();
  node->AddProperty("Image Rendering.Mode", renderingModeProperty);

  // Set default grayscale look-up table
  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
  mitkLut->SetType(mitk::LookupTable::GRAYSCALE);
  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLut);
  node->SetProperty("LookupTable", mitkLutProp);

  std::string photometricInterpretation; // DICOM tag telling us how pixel values should be displayed
  if (node->GetStringProperty("dicom.pixel.PhotometricInterpretation", photometricInterpretation))
  {
    // modality provided by DICOM or other reader
    if (photometricInterpretation.find("MONOCHROME1") != std::string::npos) // meaning: display MINIMUM pixels as WHITE
    {
      // Set inverse grayscale look-up table
      mitkLut->SetType(mitk::LookupTable::INVERSE_GRAYSCALE);
      mitkLutProp->SetLookupTable(mitkLut);
      node->SetProperty("LookupTable", mitkLutProp);
    }
    // Otherwise do nothing - the default grayscale look-up table has already been set
  }

  bool isBinaryImage(false);
  if (!node->GetBoolProperty("binary", isBinaryImage))
  {
    // ok, property is not set, use heuristic to determine if this
    // is a binary image
    mitk::Image::Pointer centralSliceImage;
    ScalarType minValue = 0.0;
    ScalarType maxValue = 0.0;
    ScalarType min2ndValue = 0.0;
    ScalarType max2ndValue = 0.0;
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();

    sliceSelector->SetInput(image);
    sliceSelector->SetSliceNr(image->GetDimension(2) / 2);
    sliceSelector->SetTimeNr(image->GetDimension(3) / 2);
    sliceSelector->SetChannelNr(image->GetDimension(4) / 2);
    sliceSelector->Update();
    centralSliceImage = sliceSelector->GetOutput();
    if (centralSliceImage.IsNotNull() && centralSliceImage->IsInitialized())
    {
      minValue = centralSliceImage->GetStatistics()->GetScalarValueMin();
      maxValue = centralSliceImage->GetStatistics()->GetScalarValueMax();
      min2ndValue = centralSliceImage->GetStatistics()->GetScalarValue2ndMin();
      max2ndValue = centralSliceImage->GetStatistics()->GetScalarValue2ndMax();
    }
    if ((maxValue == min2ndValue && minValue == max2ndValue) || minValue == maxValue)
    {
      // centralSlice is strange, lets look at all data
      minValue = image->GetStatistics()->GetScalarValueMin();
      maxValue = image->GetStatistics()->GetScalarValueMaxNoRecompute();
      min2ndValue = image->GetStatistics()->GetScalarValue2ndMinNoRecompute();
      max2ndValue = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute();
    }
    isBinaryImage = (maxValue == min2ndValue && minValue == max2ndValue);
  }

  // some more properties specific for a binary...
  if (isBinaryImage)
  {
    node->AddProperty("opacity", mitk::FloatProperty::New(0.3f), renderer, overwrite);
    node->AddProperty("color", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
    node->AddProperty("binaryimage.selectedcolor", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
    node->AddProperty("binaryimage.selectedannotationcolor", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
    node->AddProperty("binaryimage.hoveringcolor", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
    node->AddProperty("binaryimage.hoveringannotationcolor", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite);
    node->AddProperty("binary", mitk::BoolProperty::New(true), renderer, overwrite);
    node->AddProperty("layer", mitk::IntProperty::New(10), renderer, overwrite);
  }
  else //...or image type object
  {
    node->AddProperty("opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite);
    node->AddProperty("color", ColorProperty::New(1.0, 1.0, 1.0), renderer, overwrite);
    node->AddProperty("binary", mitk::BoolProperty::New(false), renderer, overwrite);
    node->AddProperty("layer", mitk::IntProperty::New(0), renderer, overwrite);

    std::string className = image->GetNameOfClass();

    if (className != "TensorImage" && className != "OdfImage" && className != "ShImage")
    {
      PixelType pixelType = image->GetPixelType();
      size_t numComponents = pixelType.GetNumberOfComponents();

      if ((pixelType.GetPixelTypeAsString() == "vector" && numComponents > 1) || numComponents == 2 ||
          numComponents > 4)
        node->AddProperty("Image.Displayed Component", mitk::IntProperty::New(0), renderer, overwrite);
    }
  }

  if (image.IsNotNull() && image->IsInitialized())
  {
    if ((overwrite) || (node->GetProperty("levelwindow", renderer) == nullptr))
    {
      /* initialize level/window from DICOM tags */
      std::string sLevel;
      std::string sWindow;

      if (GetBackwardsCompatibleDICOMProperty(
            0x0028, 0x1050, "dicom.voilut.WindowCenter", image->GetPropertyList(), sLevel) &&
          GetBackwardsCompatibleDICOMProperty(
            0x0028, 0x1051, "dicom.voilut.WindowWidth", image->GetPropertyList(), sWindow))
      {
        float level = atof(sLevel.c_str());
        float window = atof(sWindow.c_str());

        mitk::LevelWindow contrast;
        std::string sSmallestPixelValueInSeries;
        std::string sLargestPixelValueInSeries;

        if (GetBackwardsCompatibleDICOMProperty(0x0028,
                                                0x0108,
                                                "dicom.series.SmallestPixelValueInSeries",
                                                image->GetPropertyList(),
                                                sSmallestPixelValueInSeries) &&
            GetBackwardsCompatibleDICOMProperty(0x0028,
                                                0x0109,
                                                "dicom.series.LargestPixelValueInSeries",
                                                image->GetPropertyList(),
                                                sLargestPixelValueInSeries))
        {
          float smallestPixelValueInSeries = atof(sSmallestPixelValueInSeries.c_str());
          float largestPixelValueInSeries = atof(sLargestPixelValueInSeries.c_str());
          contrast.SetRangeMinMax(smallestPixelValueInSeries - 1,
                                  largestPixelValueInSeries + 1); // why not a little buffer?
          // might remedy some l/w widget challenges
        }
        else
        {
          contrast.SetAuto(static_cast<mitk::Image *>(node->GetData()), false, true); // we need this as a fallback
        }

        contrast.SetLevelWindow(level, window, true);
        node->SetProperty("levelwindow", LevelWindowProperty::New(contrast), renderer);
      }
    }
    if (((overwrite) || (node->GetProperty("opaclevelwindow", renderer) == nullptr)) &&
        (image->GetPixelType().GetPixelType() == itk::ImageIOBase::RGBA) &&
        (image->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR))
    {
      mitk::LevelWindow opaclevwin;
      opaclevwin.SetRangeMinMax(0, 255);
      opaclevwin.SetWindowBounds(0, 255);
      mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(opaclevwin);
      node->SetProperty("opaclevelwindow", prop, renderer);
    }
  }
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::DoseImageVtkMapper2D::LocalStorage *mitk::DoseImageVtkMapper2D::GetLocalStorage(mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

vtkSmartPointer<vtkPolyData> mitk::DoseImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer *renderer)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();      // the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); // the lines to connect the points
  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetNumberOfComponents(3);
  colors->SetName("Colors");

  float pref;
  this->GetDataNode()->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), pref);

  mitk::IsoDoseLevelSetProperty::Pointer propIsoSet = dynamic_cast<mitk::IsoDoseLevelSetProperty *>(
    GetDataNode()->GetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str()));
  mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = propIsoSet->GetValue();

  for (mitk::IsoDoseLevelSet::ConstIterator doseIT = isoDoseLevelSet->Begin(); doseIT != isoDoseLevelSet->End();
       ++doseIT)
  {
    if (doseIT->GetVisibleIsoLine())
    {
      this->CreateLevelOutline(renderer, &(doseIT.Value()), pref, points, lines, colors);
    } // end of if visible dose value
  }   // end of loop over all does values

  mitk::IsoDoseLevelVectorProperty::Pointer propfreeIsoVec = dynamic_cast<mitk::IsoDoseLevelVectorProperty *>(
    GetDataNode()->GetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str()));
  mitk::IsoDoseLevelVector::Pointer frereIsoDoseLevelVec = propfreeIsoVec->GetValue();

  for (mitk::IsoDoseLevelVector::ConstIterator freeDoseIT = frereIsoDoseLevelVec->Begin();
       freeDoseIT != frereIsoDoseLevelVec->End();
       ++freeDoseIT)
  {
    if (freeDoseIT->Value()->GetVisibleIsoLine())
    {
      this->CreateLevelOutline(renderer, freeDoseIT->Value(), pref, points, lines, colors);
    } // end of if visible dose value
  }   // end of loop over all does values

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  polyData->GetCellData()->SetScalars(colors);
  return polyData;
}

void mitk::DoseImageVtkMapper2D::CreateLevelOutline(mitk::BaseRenderer *renderer,
                                                    const mitk::IsoDoseLevel *level,
                                                    float pref,
                                                    vtkSmartPointer<vtkPoints> points,
                                                    vtkSmartPointer<vtkCellArray> lines,
                                                    vtkSmartPointer<vtkUnsignedCharArray> colors)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);

  // get the min and max index values of each direction
  int *extent = localStorage->m_ReslicedImage->GetExtent();
  int xMin = extent[0];
  int xMax = extent[1];
  int yMin = extent[2];
  int yMax = extent[3];

  int *dims = localStorage->m_ReslicedImage->GetDimensions(); // dimensions of the image
  int line = dims[0];                                         // how many pixels per line?
  // get the depth for each contour
  float depth = CalculateLayerDepth(renderer);

  double doseValue = level->GetDoseValue() * pref;
  mitk::IsoDoseLevel::ColorType isoColor = level->GetColor();
  unsigned char colorLine[3] = {static_cast<unsigned char>(isoColor.GetRed() * 255),
                                static_cast<unsigned char>(isoColor.GetGreen() * 255),
                                static_cast<unsigned char>(isoColor.GetBlue() * 255)};

  int x = xMin; // pixel index x
  int y = yMin; // pixel index y
  float *currentPixel;

  // We take the pointer to the first pixel of the image
  currentPixel = static_cast<float *>(localStorage->m_ReslicedImage->GetScalarPointer());

  if (!currentPixel){
    mitkThrow() << "currentPixel invalid";
  }

  while (y <= yMax)
  {
    // if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel >= doseValue))
    {
      // check in which direction a line is necessary
      // a line is added if the neighbor of the current pixel has the value 0
      // and if the pixel is located at the edge of the image

      // if   vvvvv  not the first line vvvvv
      if (y > yMin && *(currentPixel - line) < doseValue)
      { // x direction - bottom edge of the pixel
        // add the 2 points
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 =
          points->InsertNextPoint((x + 1) * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        // add the line between both points
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  not the last line vvvvv
      if (y < yMax && *(currentPixel + line) < doseValue)
      { // x direction - top edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  not the first pixel vvvvv
      if ((x > xMin || y > yMin) && *(currentPixel - 1) < doseValue)
      { // y direction - left edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  not the last pixel vvvvv
      if ((y < yMax || (x < xMax)) && *(currentPixel + 1) < doseValue)
      { // y direction - right edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint((x + 1) * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      /*  now consider pixels at the edge of the image  */

      // if   vvvvv  left edge of image vvvvv
      if (x == xMin)
      { // draw left edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  right edge of image vvvvv
      if (x == xMax)
      { // draw right edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint((x + 1) * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  bottom edge of image vvvvv
      if (y == yMin)
      { // draw bottom edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 =
          points->InsertNextPoint((x + 1) * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }

      // if   vvvvv  top edge of image vvvvv
      if (y == yMax)
      { // draw top edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
        colors->InsertNextTypedTuple(colorLine);
      }
    } // end if currentpixel is set

    x++;

    if (x > xMax)
    { // reached end of line
      x = xMin;
      y++;
    }

    // Increase the pointer-position to the next pixel.
    // This is safe, as the while-loop and the x-reset logic above makes
    // sure we do not exceed the bounds of the image
    currentPixel++;
  } // end of while
}

void mitk::DoseImageVtkMapper2D::TransformActor(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  // get the transformation matrix of the reslicer in order to render the slice as axial, coronal or saggital
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_Reslicer->GetResliceAxes();
  trans->SetMatrix(matrix);
  // transform the plane/contour (the actual actor) to the corresponding view (axial, coronal or saggital)
  localStorage->m_Actor->SetUserTransform(trans);
  // transform the origin to center based coordinates, because MITK is center based.
  localStorage->m_Actor->SetPosition(-0.5 * localStorage->m_mmPerPixel[0], -0.5 * localStorage->m_mmPerPixel[1], 0.0);

  if (localStorage->m_Actors->GetNumberOfPaths() > 1)
  {
    vtkActor *secondaryActor = dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0));
    secondaryActor->SetUserTransform(trans);
    secondaryActor->SetPosition(-0.5 * localStorage->m_mmPerPixel[0], -0.5 * localStorage->m_mmPerPixel[1], 0.0);
  }
}

bool mitk::DoseImageVtkMapper2D::RenderingGeometryIntersectsImage(const PlaneGeometry *renderingGeometry,
                                                                  SlicedGeometry3D *imageGeometry)
{
  // if either one of the two geometries is nullptr we return true
  // for safety reasons
  if (renderingGeometry == nullptr || imageGeometry == nullptr)
    return true;

  // get the distance for the first cornerpoint
  ScalarType initialDistance = renderingGeometry->SignedDistance(imageGeometry->GetCornerPoint(0));
  for (int i = 1; i < 8; i++)
  {
    mitk::Point3D cornerPoint = imageGeometry->GetCornerPoint(i);

    // get the distance to the other cornerpoints
    ScalarType distance = renderingGeometry->SignedDistance(cornerPoint);

    // if it has not the same signing as the distance of the first point
    if (initialDistance * distance < 0)
    {
      // we have an intersection and return true
      return true;
    }
  }

  // all distances have the same sign, no intersection and we return false
  return false;
}

mitk::DoseImageVtkMapper2D::LocalStorage::~LocalStorage()
{
}

mitk::DoseImageVtkMapper2D::LocalStorage::LocalStorage()
  : m_VectorComponentExtractor(vtkSmartPointer<vtkImageExtractComponents>::New())
{
  m_LevelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();

  // Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  m_Texture = vtkSmartPointer<vtkNeverTranslucentTexture>::New().GetPointer();
  m_DefaultLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_BinaryLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_ColorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_Reslicer = mitk::ExtractSliceFilter::New();
  m_TSFilter = vtkSmartPointer<vtkMitkThickSlicesFilter>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_ReslicedImage = vtkSmartPointer<vtkImageData>::New();
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  // the following actions are always the same and thus can be performed
  // in the constructor for each image (i.e. the image-corresponding local storage)
  m_TSFilter->ReleaseDataFlagOn();

  mitk::LookupTable::Pointer mitkLUT = mitk::LookupTable::New();
  // built a default lookuptable
  mitkLUT->SetType(mitk::LookupTable::GRAYSCALE);
  m_DefaultLookupTable = mitkLUT->GetVtkLookupTable();

  mitkLUT->SetType(mitk::LookupTable::LEGACY_BINARY);
  m_BinaryLookupTable = mitkLUT->GetVtkLookupTable();

  mitkLUT->SetType(mitk::LookupTable::LEGACY_RAINBOW_COLOR);
  m_ColorLookupTable = mitkLUT->GetVtkLookupTable();

  // do not repeat the texture (the image)
  m_Texture->RepeatOff();

  // set the mapper for the actor
  m_Actor->SetMapper(m_Mapper);

  vtkSmartPointer<vtkActor> outlineShadowActor = vtkSmartPointer<vtkActor>::New();
  outlineShadowActor->SetMapper(m_Mapper);

  m_Actors->AddPart(outlineShadowActor);
  m_Actors->AddPart(m_Actor);
}
