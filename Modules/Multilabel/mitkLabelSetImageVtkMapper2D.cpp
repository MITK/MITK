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

#include "mitkLabelSetImageVtkMapper2D.h"

// MITK
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkImageSliceSelector.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPixelType.h>
#include <mitkPlaneClipping.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkResliceMethodProperty.h>
#include <mitkTimeSlicedGeometry.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>

// MITK Rendering
#include "vtkMitkLevelWindowFilter.h"
#include "vtkMitkThickSlicesFilter.h"
#include "vtkNeverTranslucentTexture.h"

// VTK
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
//#include <vtkOpenGLTexture.h>

// ITK
#include <itkRGBAPixel.h>
#include <mitkRenderingModeProperty.h>

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

void mitk::LabelSetImageVtkMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  mitk::DataNode *node = this->GetDataNode();
  auto *image = dynamic_cast<mitk::LabelSetImage *>(node->GetData());
  assert(image && image->IsInitialized());

  // check if there is a valid worldGeometry
  const PlaneGeometry *worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if ((worldGeometry == nullptr) || (!worldGeometry->IsValid()) || (!worldGeometry->HasReferenceGeometry()))
    return;

  image->Update();

  int numberOfLayers = image->GetNumberOfLayers();
  int activeLayer = image->GetActiveLayer();

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  if (numberOfLayers != localStorage->m_NumberOfLayers)
  {
    localStorage->m_NumberOfLayers = numberOfLayers;
    localStorage->m_ReslicedImageVector.clear();
    localStorage->m_ReslicerVector.clear();
    localStorage->m_LayerTextureVector.clear();
    localStorage->m_LevelWindowFilterVector.clear();
    localStorage->m_LayerMapperVector.clear();
    localStorage->m_LayerActorVector.clear();

    localStorage->m_Actors = vtkSmartPointer<vtkPropAssembly>::New();

    for (int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      localStorage->m_ReslicedImageVector.push_back(vtkSmartPointer<vtkImageData>::New());
      localStorage->m_ReslicerVector.push_back(mitk::ExtractSliceFilter::New());
      localStorage->m_LayerTextureVector.push_back(vtkSmartPointer<vtkNeverTranslucentTexture>::New());
      localStorage->m_LevelWindowFilterVector.push_back(vtkSmartPointer<vtkMitkLevelWindowFilter>::New());
      localStorage->m_LayerMapperVector.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
      localStorage->m_LayerActorVector.push_back(vtkSmartPointer<vtkActor>::New());

      // do not repeat the texture (the image)
      localStorage->m_LayerTextureVector[lidx]->RepeatOff();

      // set corresponding mappers for the actors
      localStorage->m_LayerActorVector[lidx]->SetMapper(localStorage->m_LayerMapperVector[lidx]);

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
    for (int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      localStorage->m_ReslicedImageVector[lidx] = nullptr;
      localStorage->m_LayerMapperVector[lidx]->SetInputData(localStorage->m_EmptyPolyData);
      localStorage->m_OutlineActor->SetVisibility(false);
      localStorage->m_OutlineShadowActor->SetVisibility(false);
    }
    return;
  }

  for (int lidx = 0; lidx < numberOfLayers; ++lidx)
  {
    mitk::Image *layerImage = nullptr;

    // set main input for ExtractSliceFilter
    if (lidx == activeLayer)
      layerImage = image;
    else
      layerImage = image->GetLayerImage(lidx);

    localStorage->m_ReslicerVector[lidx]->SetInput(layerImage);
    localStorage->m_ReslicerVector[lidx]->SetWorldGeometry(worldGeometry);
    localStorage->m_ReslicerVector[lidx]->SetTimeStep(this->GetTimestep());

    // set the transformation of the image to adapt reslice axis
    localStorage->m_ReslicerVector[lidx]->SetResliceTransformByGeometry(
      layerImage->GetTimeGeometry()->GetGeometryForTimeStep(this->GetTimestep()));

    // is the geometry of the slice based on the image image or the worldgeometry?
    bool inPlaneResampleExtentByGeometry = false;
    node->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
    localStorage->m_ReslicerVector[lidx]->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);
    localStorage->m_ReslicerVector[lidx]->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
    localStorage->m_ReslicerVector[lidx]->SetVtkOutputRequest(true);

    // this is needed when thick mode was enabled before. These variables have to be reset to default values
    localStorage->m_ReslicerVector[lidx]->SetOutputDimensionality(2);
    localStorage->m_ReslicerVector[lidx]->SetOutputSpacingZDirection(1.0);
    localStorage->m_ReslicerVector[lidx]->SetOutputExtentZDirection(0, 0);

    // Bounds information for reslicing (only required if reference geometry is present)
    // this used for generating a vtkPLaneSource with the right size
    double sliceBounds[6];
    sliceBounds[0] = 0.0;
    sliceBounds[1] = 0.0;
    sliceBounds[2] = 0.0;
    sliceBounds[3] = 0.0;
    sliceBounds[4] = 0.0;
    sliceBounds[5] = 0.0;

    localStorage->m_ReslicerVector[lidx]->GetClippedPlaneBounds(sliceBounds);

    // setup the textured plane
    this->GeneratePlane(renderer, sliceBounds);

    // get the spacing of the slice
    localStorage->m_mmPerPixel = localStorage->m_ReslicerVector[lidx]->GetOutputSpacing();
    localStorage->m_ReslicerVector[lidx]->Modified();
    // start the pipeline with updating the largest possible, needed if the geometry of the image has changed
    localStorage->m_ReslicerVector[lidx]->UpdateLargestPossibleRegion();
    localStorage->m_ReslicedImageVector[lidx] = localStorage->m_ReslicerVector[lidx]->GetVtkOutput();

    const auto *planeGeometry = dynamic_cast<const PlaneGeometry *>(worldGeometry);

    double textureClippingBounds[6];
    for (auto &textureClippingBound : textureClippingBounds)
    {
      textureClippingBound = 0.0;
    }

    // Calculate the actual bounds of the transformed plane clipped by the
    // dataset bounding box; this is required for drawing the texture at the
    // correct position during 3D mapping.
    mitk::PlaneClipping::CalculateClippedPlaneBounds(layerImage->GetGeometry(), planeGeometry, textureClippingBounds);

    textureClippingBounds[0] = static_cast<int>(textureClippingBounds[0] / localStorage->m_mmPerPixel[0] + 0.5);
    textureClippingBounds[1] = static_cast<int>(textureClippingBounds[1] / localStorage->m_mmPerPixel[0] + 0.5);
    textureClippingBounds[2] = static_cast<int>(textureClippingBounds[2] / localStorage->m_mmPerPixel[1] + 0.5);
    textureClippingBounds[3] = static_cast<int>(textureClippingBounds[3] / localStorage->m_mmPerPixel[1] + 0.5);

    // clipping bounds for cutting the imageLayer
    localStorage->m_LevelWindowFilterVector[lidx]->SetClippingBounds(textureClippingBounds);

    localStorage->m_LevelWindowFilterVector[lidx]->SetLookupTable(
      image->GetLabelSet(lidx)->GetLookupTable()->GetVtkLookupTable());

    // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
    localStorage->m_LayerTextureVector[lidx]->SetColorModeToDirectScalars();

    // connect the imageLayer with the levelwindow filter
    localStorage->m_LevelWindowFilterVector[lidx]->SetInputData(localStorage->m_ReslicedImageVector[lidx]);
    // connect the texture with the output of the levelwindow filter

    // check for texture interpolation property
    bool textureInterpolation = false;
    node->GetBoolProperty("texture interpolation", textureInterpolation, renderer);

    // set the interpolation modus according to the property
    localStorage->m_LayerTextureVector[lidx]->SetInterpolate(textureInterpolation);

    localStorage->m_LayerTextureVector[lidx]->SetInputConnection(
      localStorage->m_LevelWindowFilterVector[lidx]->GetOutputPort());

    this->TransformActor(renderer);

    // set the plane as input for the mapper
    localStorage->m_LayerMapperVector[lidx]->SetInputConnection(localStorage->m_Plane->GetOutputPort());

    // set the texture for the actor
    localStorage->m_LayerActorVector[lidx]->SetTexture(localStorage->m_LayerTextureVector[lidx]);
    localStorage->m_LayerActorVector[lidx]->GetProperty()->SetOpacity(opacity);
  }

  mitk::Label* activeLabel = image->GetActiveLabel(activeLayer);
  if (nullptr != activeLabel)
  {
    bool contourActive = false;
    node->GetBoolProperty("labelset.contour.active", contourActive, renderer);
    if (contourActive && activeLabel->GetVisible()) //contour rendering
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
      return;
    }
  }
  localStorage->m_OutlineActor->SetVisibility(false);
  localStorage->m_OutlineShadowActor->SetVisibility(false);
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

void mitk::LabelSetImageVtkMapper2D::ApplyColor(mitk::BaseRenderer *renderer, const mitk::Color &color)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);
  localStorage->m_OutlineActor->GetProperty()->SetColor(color.GetRed(), color.GetGreen(), color.GetBlue());
  localStorage->m_OutlineShadowActor->GetProperty()->SetColor(0, 0, 0);
}

void mitk::LabelSetImageVtkMapper2D::ApplyOpacity(mitk::BaseRenderer *renderer, int layer)
{
  LocalStorage *localStorage = this->GetLocalStorage(renderer);
  float opacity = 1.0f;
  this->GetDataNode()->GetOpacity(opacity, renderer, "opacity");
  localStorage->m_LayerActorVector[layer]->GetProperty()->SetOpacity(opacity);
  localStorage->m_OutlineActor->GetProperty()->SetOpacity(opacity);
  localStorage->m_OutlineShadowActor->GetProperty()->SetOpacity(opacity);
}

void mitk::LabelSetImageVtkMapper2D::ApplyLookuptable(mitk::BaseRenderer *renderer, int layer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  auto *input = dynamic_cast<mitk::LabelSetImage *>(this->GetDataNode()->GetData());
  localStorage->m_LevelWindowFilterVector[layer]->SetLookupTable(
    input->GetLabelSet(layer)->GetLookupTable()->GetVtkLookupTable());
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

  if ((localStorage->m_LastDataUpdateTime < image->GetMTime()) ||
      (localStorage->m_LastDataUpdateTime < image->GetPipelineMTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) ||
      (localStorage->m_LastDataUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime()))
  {
    this->GenerateDataForRenderer(renderer);
    localStorage->m_LastDataUpdateTime.Modified();
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
  // Each plane is transformed according to the view (axial, coronal and saggital) afterwards.
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
  // get the transformation matrix of the reslicer in order to render the slice as axial, coronal or saggital
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_ReslicerVector[0]->GetResliceAxes(); // same for all layers
  trans->SetMatrix(matrix);

  for (int lidx = 0; lidx < localStorage->m_NumberOfLayers; ++lidx)
  {
    // transform the plane/contour (the actual actor) to the corresponding view (axial, coronal or saggital)
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

  mitk::RenderingModeProperty::Pointer renderingModeProperty =
    mitk::RenderingModeProperty::New(RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
  node->SetProperty("Image Rendering.Mode", renderingModeProperty, renderer);

  mitk::LevelWindow levelwindow(32767.5, 65535);
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New(levelwindow);

  levWinProp->SetLevelWindow(levelwindow);
  node->SetProperty("levelwindow", levWinProp, renderer);

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
