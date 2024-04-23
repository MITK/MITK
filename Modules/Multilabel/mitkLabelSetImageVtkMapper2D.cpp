/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSetImageVtkMapper2D.h"

// MITK
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkImageSliceSelector.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkVectorProperty.h>

// MITK Rendering
#include "vtkNeverTranslucentTexture.h"

// VTK
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkLookupTable.h>
#include <vtkPlaneSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageMapToColors.h>

namespace
{
  itk::ModifiedTimeType PropertyTimeStampIsNewer(const mitk::IPropertyProvider* provider, mitk::BaseRenderer* renderer, const std::string& propName, itk::ModifiedTimeType refMT)
  {
    const std::string context = renderer != nullptr ? renderer->GetName() : "";
    auto prop = provider->GetConstProperty(propName, context);
    if (prop != nullptr)
    {
      return prop->GetTimeStamp() > refMT;
    }
    return false;
  }
}

mitk::LabelSetImageVtkMapper2D::LabelSetImageVtkMapper2D()
{
}

mitk::LabelSetImageVtkMapper2D::~LabelSetImageVtkMapper2D()
{
}

vtkProp *mitk::LabelSetImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  // return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

mitk::LabelSetImageVtkMapper2D::LocalStorage *mitk::LabelSetImageVtkMapper2D::GetLocalStorage(
  mitk::BaseRenderer *renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

void mitk::LabelSetImageVtkMapper2D::GenerateLookupTable(mitk::BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image && image->IsInitialized());

  localStorage->m_LabelLookupTable = image->GetLookupTable()->Clone();
  const auto labelValues = image->GetAllLabelValues();

  std::string propertyName = "org.mitk.multilabel.labels.highlighted";

  mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(node->GetNonConstProperty(propertyName));
  if (nullptr != prop)
  {
    const auto highlightedLabelValues = prop->GetValue();

    if (!highlightedLabelValues.empty())
    {
      auto lookUpTable  = localStorage->m_LabelLookupTable->GetVtkLookupTable();
      auto highlightEnd = highlightedLabelValues.cend();

      double rgba[4];
      for (const auto& value : labelValues)
      {
        lookUpTable->GetTableValue(value, rgba);
        if (highlightEnd == std::find(highlightedLabelValues.begin(), highlightedLabelValues.end(), value))
        { //make all none highlighted values more transparent
          rgba[3] *= 0.3;
        }
        else
        {
          if (rgba[3] != 0)
          { //if highlighted values are visible set them to opaque to pop out
            rgba[3] = 1.;
          }
          else
          { //if highlighted values are invisible the opacity is increased a bit
            //to give a visual hint that the are highlighted but also invisible.
            //e.g. needed to see a difference if you change the visibility of
            //a highlighted label in the MultiLabelInspector
            rgba[3] = 0.4;
          }
        }
        lookUpTable->SetTableValue(value, rgba);
      }
      localStorage->m_LabelLookupTable->Modified();
    }
  }
}

namespace
{
  std::vector<mitk::LabelSetImage::GroupIndexType> GetOutdatedGroups(const mitk::LabelSetImageVtkMapper2D::LocalStorage* ls, const mitk::LabelSetImage* seg)
  {
    const auto nrOfGroups = seg->GetNumberOfLayers();
    std::vector<mitk::LabelSetImage::GroupIndexType> result;

    for (mitk::LabelSetImage::GroupIndexType groupID = 0; groupID < nrOfGroups; ++groupID)
    {
      const auto groupImage = seg->GetGroupImage(groupID);
      if (groupImage->GetMTime() > ls->m_LastDataUpdateTime
        || groupImage->GetPipelineMTime() > ls->m_LastDataUpdateTime
        || ls->m_GroupImageIDs.size() <= groupID
        || groupImage != ls->m_GroupImageIDs[groupID])
      {
        result.push_back(groupID);
      }
    }
    return result;
  }
}

void mitk::LabelSetImageVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode *node = this->GetDataNode();
  auto *image = dynamic_cast<mitk::LabelSetImage *>(node->GetData());
  assert(image && image->IsInitialized());

  bool isLookupModified = localStorage->m_LabelLookupTable.IsNull() ||
    (localStorage->m_LabelLookupTable->GetMTime() < image->GetLookupTable()->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "org.mitk.multilabel.labels.highlighted", localStorage->m_LabelLookupTable->GetMTime()) ||
    PropertyTimeStampIsNewer(node, renderer, "opacity", localStorage->m_LabelLookupTable->GetMTime());

  if (isLookupModified)
  {
    this->GenerateLookupTable(renderer);
  }

  auto outdatedGroups = GetOutdatedGroups(localStorage, image);

  bool isGeometryModified = (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
    (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime());

  // check if there is a valid worldGeometry
  if (isGeometryModified)
  {
    const PlaneGeometry* worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
    isGeometryModified = !((worldGeometry == nullptr)
      || (!worldGeometry->IsValid())
      || (!worldGeometry->HasReferenceGeometry())
      || (localStorage->m_WorldPlane.IsNotNull() && Equal(*worldGeometry, *(localStorage->m_WorldPlane.GetPointer()))));

    localStorage->m_WorldPlane = worldGeometry->Clone();
  }

  if (isGeometryModified)
  {
    //if geometry is outdated all groups need regeneration
    outdatedGroups.resize(image->GetNumberOfLayers());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  if (!outdatedGroups.empty())
  {
    auto hasValidContent = this->GenerateImageSlice(renderer, outdatedGroups);
    if (!hasValidContent) return;
  }

  auto activeLayer = image->GetActiveLayer();
  bool activeGroupIsOutdated = std::find(outdatedGroups.begin(), outdatedGroups.end(), activeLayer) != outdatedGroups.end();

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  if (isLookupModified)
  {
    //if lookup table is modified all groups need a new color mapping
    outdatedGroups.resize(image->GetNumberOfLayers());
    std::iota(outdatedGroups.begin(), outdatedGroups.end(), 0);
  }

  for (const auto groupID: outdatedGroups)
  {
    localStorage->m_LayerImageMapToColors[groupID]->SetLookupTable(localStorage->m_LabelLookupTable->GetVtkLookupTable());
    localStorage->m_LayerImageMapToColors[groupID]->SetInputData(localStorage->m_ReslicedImageVector[groupID]);
    localStorage->m_LayerImageMapToColors[groupID]->Update();

    // check for texture interpolation property
    bool textureInterpolation = false;
    node->GetBoolProperty("texture interpolation", textureInterpolation, renderer);

    // set the interpolation modus according to the property
    localStorage->m_LayerTextureVector[groupID]->SetInterpolate(textureInterpolation);

    localStorage->m_LayerTextureVector[groupID]->SetInputConnection(
      localStorage->m_LayerImageMapToColors[groupID]->GetOutputPort());
    this->TransformActor(renderer);

    // set the plane as input for the mapper
    localStorage->m_LayerMapperVector[groupID]->SetInputConnection(localStorage->m_Plane->GetOutputPort());

    // set the texture for the actor
    localStorage->m_LayerActorVector[groupID]->SetTexture(localStorage->m_LayerTextureVector[groupID]);
    localStorage->m_LayerActorVector[groupID]->GetProperty()->SetOpacity(opacity);
  }

  if (activeGroupIsOutdated
      || PropertyTimeStampIsNewer(node, renderer, "opacity", localStorage->m_LastActiveLabelUpdateTime.GetMTime())
      || PropertyTimeStampIsNewer(node, renderer, "labelset.contour.active", localStorage->m_LastActiveLabelUpdateTime.GetMTime())
      || PropertyTimeStampIsNewer(node, renderer, "labelset.contour.width", localStorage->m_LastActiveLabelUpdateTime.GetMTime())
    )
  {
    this->GenerateActiveLabelOutline(renderer);
  }
}

bool mitk::LabelSetImageVtkMapper2D::GenerateImageSlice(mitk::BaseRenderer* renderer, const std::vector<mitk::LabelSetImage::GroupIndexType>& outdatedGroupIDs)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  assert(image && image->IsInitialized());

  // check if there is a valid worldGeometry
  const PlaneGeometry* worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if ((worldGeometry == nullptr)
    || (!worldGeometry->IsValid())
    || (!worldGeometry->HasReferenceGeometry()))
    return false;

  localStorage->m_WorldPlane = worldGeometry->Clone();
  image->Update();

  const auto numberOfLayers = image->GetNumberOfLayers();

  if (numberOfLayers != localStorage->m_NumberOfLayers)
  {
    if (numberOfLayers > localStorage->m_NumberOfLayers)
    {
      for (unsigned int lidx = localStorage->m_NumberOfLayers; lidx < numberOfLayers; ++lidx)
      {
        localStorage->m_GroupImageIDs.push_back(nullptr);
        localStorage->m_ReslicedImageVector.push_back(vtkSmartPointer<vtkImageData>::New());
        localStorage->m_ReslicerVector.push_back(mitk::ExtractSliceFilter::New());
        localStorage->m_LayerTextureVector.push_back(vtkSmartPointer<vtkNeverTranslucentTexture>::New());
        localStorage->m_LayerMapperVector.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
        localStorage->m_LayerActorVector.push_back(vtkSmartPointer<vtkActor>::New());
        localStorage->m_LayerImageMapToColors.push_back(vtkSmartPointer<vtkImageMapToColors>::New());

        // do not repeat the texture (the image)
        localStorage->m_LayerTextureVector[lidx]->RepeatOff();
        // set corresponding mappers for the actors
        localStorage->m_LayerActorVector[lidx]->SetMapper(localStorage->m_LayerMapperVector[lidx]);
      }
    }
    else
    {
      localStorage->m_GroupImageIDs.resize(numberOfLayers);
      localStorage->m_ReslicedImageVector.resize(numberOfLayers);
      localStorage->m_ReslicerVector.resize(numberOfLayers);
      localStorage->m_LayerTextureVector.resize(numberOfLayers);
      localStorage->m_LayerMapperVector.resize(numberOfLayers);
      localStorage->m_LayerActorVector.resize(numberOfLayers);
      localStorage->m_LayerImageMapToColors.resize(numberOfLayers);
    }
    localStorage->m_NumberOfLayers = numberOfLayers;

    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

    for (unsigned int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      localStorage->m_Actors->AddPart(localStorage->m_LayerActorVector[lidx]);
    }
    localStorage->m_Actors->AddPart(localStorage->m_OutlineShadowActor);
    localStorage->m_Actors->AddPart(localStorage->m_OutlineActor);
  }

  // early out if there is no intersection of the current rendering geometry
  // and the geometry of the image that is to be rendered.
  if (!RenderingGeometryIntersectsImage(worldGeometry, image->GetSlicedGeometry()))
  {
    // set image to nullptr, to clear the texture in 3D, because
    // the latest image is used there if the plane is out of the geometry
    // see bug-13275
    for (unsigned int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      localStorage->m_ReslicedImageVector[lidx] = nullptr;
      localStorage->m_LayerMapperVector[lidx]->SetInputData(localStorage->m_EmptyPolyData);
      localStorage->m_OutlineActor->SetVisibility(false);
      localStorage->m_OutlineShadowActor->SetVisibility(false);
    }
    localStorage->m_LastDataUpdateTime.Modified();
    return false;
  }

  for (const auto groupID : outdatedGroupIDs)
  {
    const auto groupImage = image->GetGroupImage(groupID);
    localStorage->m_GroupImageIDs[groupID] = groupImage;

    localStorage->m_ReslicerVector[groupID]->SetInput(groupImage);
    localStorage->m_ReslicerVector[groupID]->SetWorldGeometry(worldGeometry);
    localStorage->m_ReslicerVector[groupID]->SetTimeStep(this->GetTimestep());

    // set the transformation of the image to adapt reslice axis
    localStorage->m_ReslicerVector[groupID]->SetResliceTransformByGeometry(
      groupImage->GetTimeGeometry()->GetGeometryForTimeStep(this->GetTimestep()));

    // is the geometry of the slice based on the image image or the worldgeometry?
    bool inPlaneResampleExtentByGeometry = false;
    node->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
    localStorage->m_ReslicerVector[groupID]->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);
    localStorage->m_ReslicerVector[groupID]->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
    localStorage->m_ReslicerVector[groupID]->SetVtkOutputRequest(true);

    // this is needed when thick mode was enabled before. These variables have to be reset to default values
    localStorage->m_ReslicerVector[groupID]->SetOutputDimensionality(2);
    localStorage->m_ReslicerVector[groupID]->SetOutputSpacingZDirection(1.0);
    localStorage->m_ReslicerVector[groupID]->SetOutputExtentZDirection(0, 0);

    // Bounds information for reslicing (only required if reference geometry is present)
    // this used for generating a vtkPLaneSource with the right size
    double sliceBounds[6];
    sliceBounds[0] = 0.0;
    sliceBounds[1] = 0.0;
    sliceBounds[2] = 0.0;
    sliceBounds[3] = 0.0;
    sliceBounds[4] = 0.0;
    sliceBounds[5] = 0.0;

    localStorage->m_ReslicerVector[groupID]->GetClippedPlaneBounds(sliceBounds);

    // setup the textured plane
    this->GeneratePlane(renderer, sliceBounds);

    // get the spacing of the slice
    localStorage->m_mmPerPixel = localStorage->m_ReslicerVector[groupID]->GetOutputSpacing();
    localStorage->m_ReslicerVector[groupID]->Modified();
    // start the pipeline with updating the largest possible, needed if the geometry of the image has changed
    localStorage->m_ReslicerVector[groupID]->UpdateLargestPossibleRegion();
    localStorage->m_ReslicedImageVector[groupID] = localStorage->m_ReslicerVector[groupID]->GetVtkOutput();
  }
  localStorage->m_LastDataUpdateTime.Modified();
  return true;
}

void mitk::LabelSetImageVtkMapper2D::GenerateActiveLabelOutline(mitk::BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode* node = this->GetDataNode();
  auto* image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

  int activeLayer = image->GetActiveLayer();

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  mitk::Label* activeLabel = image->GetActiveLabel();
  bool contourActive = false;
  node->GetBoolProperty("labelset.contour.active", contourActive, renderer);
  if (nullptr != activeLabel && contourActive && activeLabel->GetVisible())
  {
    //generate contours/outlines
    localStorage->m_OutlinePolyData =
      this->CreateOutlinePolyData(renderer, localStorage->m_ReslicedImageVector[activeLayer], activeLabel->GetValue());
    localStorage->m_OutlineActor->SetVisibility(true);
    localStorage->m_OutlineShadowActor->SetVisibility(true);
    const mitk::Color& color = activeLabel->GetColor();
    localStorage->m_OutlineActor->GetProperty()->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
    localStorage->m_OutlineShadowActor->GetProperty()->SetColor(0, 0, 0);

    float contourWidth(2.0);
    node->GetFloatProperty("labelset.contour.width", contourWidth, renderer);
    localStorage->m_OutlineActor->GetProperty()->SetLineWidth(contourWidth);
    localStorage->m_OutlineShadowActor->GetProperty()->SetLineWidth(contourWidth * 1.5);

    localStorage->m_OutlineActor->GetProperty()->SetOpacity(opacity);
    localStorage->m_OutlineShadowActor->GetProperty()->SetOpacity(opacity);

    localStorage->m_OutlineMapper->SetInputData(localStorage->m_OutlinePolyData);
  }
  else
  {
    localStorage->m_OutlineActor->SetVisibility(false);
    localStorage->m_OutlineShadowActor->SetVisibility(false);
  }
  localStorage->m_LastActiveLabelUpdateTime.Modified();
}


bool mitk::LabelSetImageVtkMapper2D::RenderingGeometryIntersectsImage(const PlaneGeometry *renderingGeometry,
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

vtkSmartPointer<vtkPolyData> mitk::LabelSetImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer *renderer,
                                                                                   vtkImageData *image,
                                                                                   int pixelValue)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);

  // get the min and max index values of each direction
  int *extent = image->GetExtent();
  int xMin = extent[0];
  int xMax = extent[1];
  int yMin = extent[2];
  int yMax = extent[3];

  int *dims = image->GetDimensions(); // dimensions of the image
  int line = dims[0];                 // how many pixels per line?
  int x = xMin;                       // pixel index x
  int y = yMin;                       // pixel index y

  // get the depth for each contour
  float depth = this->CalculateLayerDepth(renderer);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();      // the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); // the lines to connect the points

  // We take the pointer to the first pixel of the image
  auto *currentPixel = static_cast<mitk::Label::PixelType *>(image->GetScalarPointer());

  while (y <= yMax)
  {
    // if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel == pixelValue))
    {
      // check in which direction a line is necessary
      // a line is added if the neighbor of the current pixel has the value 0
      // and if the pixel is located at the edge of the image

      // if   vvvvv  not the first line vvvvv
      if (y > yMin && *(currentPixel - line) != pixelValue)
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
      }

      // if   vvvvv  not the last line vvvvv
      if (y < yMax && *(currentPixel + line) != pixelValue)
      { // x direction - top edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      // if   vvvvv  not the first pixel vvvvv
      if ((x > xMin || y > yMin) && *(currentPixel - 1) != pixelValue)
      { // y direction - left edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 =
          points->InsertNextPoint(x * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      // if   vvvvv  not the last pixel vvvvv
      if ((y < yMax || (x < xMax)) && *(currentPixel + 1) != pixelValue)
      { // y direction - right edge of the pixel
        vtkIdType p1 =
          points->InsertNextPoint((x + 1) * localStorage->m_mmPerPixel[0], y * localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(
          (x + 1) * localStorage->m_mmPerPixel[0], (y + 1) * localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
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

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  return polyData;
}

void mitk::LabelSetImageVtkMapper2D::Update(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  const DataNode *node = this->GetDataNode();
  node->GetVisibility(visible, renderer, "visible");

  if (!visible)
    return;

  auto *image = dynamic_cast<mitk::LabelSetImage *>(node->GetData());

  if (image == nullptr || image->IsInitialized() == false)
    return;

  // Calculate time step of the image data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = image->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(this->GetTimestep())))
  {
    return;
  }

  image->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  // check if something important has changed and we need to re-render

  if (localStorage->m_LabelLookupTable.IsNull() ||
      (localStorage->m_LabelLookupTable->GetMTime() < image->GetLookupTable()->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < image->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < image->GetPipelineMTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) ||
      (localStorage->m_LastPropertyUpdateTime < image->GetPropertyList()->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
  else if ((localStorage->m_LastPropertyUpdateTime < node->GetPropertyList()->GetMTime()) ||
           (localStorage->m_LastPropertyUpdateTime < node->GetPropertyList(renderer)->GetMTime()) ||
           (localStorage->m_LastPropertyUpdateTime < image->GetPropertyList()->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastPropertyUpdateTime.Modified();
  }
}

// set the two points defining the textured plane according to the dimension and spacing
void mitk::LabelSetImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer *renderer, double planeBounds[6])
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float depth = this->CalculateLayerDepth(renderer);
  // Set the origin to (xMin; yMin; depth) of the plane. This is necessary for obtaining the correct
  // plane size in crosshair rotation and swivel mode.
  localStorage->m_Plane->SetOrigin(planeBounds[0], planeBounds[2], depth);
  // These two points define the axes of the plane in combination with the origin.
  // Point 1 is the x-axis and point 2 the y-axis.
  // Each plane is transformed according to the view (axial, coronal and sagittal) afterwards.
  localStorage->m_Plane->SetPoint1(planeBounds[1], planeBounds[2], depth); // P1: (xMax, yMin, depth)
  localStorage->m_Plane->SetPoint2(planeBounds[0], planeBounds[3], depth); // P2: (xMin, yMax, depth)
}

float mitk::LabelSetImageVtkMapper2D::CalculateLayerDepth(mitk::BaseRenderer *renderer)
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

void mitk::LabelSetImageVtkMapper2D::TransformActor(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  // get the transformation matrix of the reslicer in order to render the slice as axial, coronal or sagittal
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_ReslicerVector[0]->GetResliceAxes(); // same for all layers
  trans->SetMatrix(matrix);

  for (unsigned int lidx = 0; lidx < localStorage->m_NumberOfLayers; ++lidx)
  {
    // transform the plane/contour (the actual actor) to the corresponding view (axial, coronal or sagittal)
    localStorage->m_LayerActorVector[lidx]->SetUserTransform(trans);
    // transform the origin to center based coordinates, because MITK is center based.
    localStorage->m_LayerActorVector[lidx]->SetPosition(
      -0.5 * localStorage->m_mmPerPixel[0], -0.5 * localStorage->m_mmPerPixel[1], 0.0);
  }
  // same for outline actor
  localStorage->m_OutlineActor->SetUserTransform(trans);
  localStorage->m_OutlineActor->SetPosition(
    -0.5 * localStorage->m_mmPerPixel[0], -0.5 * localStorage->m_mmPerPixel[1], 0.0);
  // same for outline shadow actor
  localStorage->m_OutlineShadowActor->SetUserTransform(trans);
  localStorage->m_OutlineShadowActor->SetPosition(
    -0.5 * localStorage->m_mmPerPixel[0], -0.5 * localStorage->m_mmPerPixel[1], 0.0);
}

void mitk::LabelSetImageVtkMapper2D::SetDefaultProperties(mitk::DataNode *node,
                                                          mitk::BaseRenderer *renderer,
                                                          bool overwrite)
{
  // add/replace the following properties
  node->SetProperty("opacity", FloatProperty::New(1.0f), renderer);
  node->SetProperty("binary", BoolProperty::New(false), renderer);

  node->SetProperty("labelset.contour.active", BoolProperty::New(true), renderer);
  node->SetProperty("labelset.contour.width", FloatProperty::New(2.0), renderer);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::LabelSetImageVtkMapper2D::LocalStorage::~LocalStorage()
{
}

mitk::LabelSetImageVtkMapper2D::LocalStorage::LocalStorage()
{
  // Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_OutlineActor = vtkSmartPointer<vtkActor>::New();
  m_OutlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_OutlineShadowActor = vtkSmartPointer<vtkActor>::New();

  m_NumberOfLayers = 0;
  m_mmPerPixel = nullptr;

  m_OutlineActor->SetMapper(m_OutlineMapper);
  m_OutlineShadowActor->SetMapper(m_OutlineMapper);

  m_OutlineActor->SetVisibility(false);
  m_OutlineShadowActor->SetVisibility(false);
}
