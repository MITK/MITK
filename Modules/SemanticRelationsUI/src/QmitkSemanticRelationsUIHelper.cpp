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

// semantic relations ui module
#include <QmitkSemanticRelationsUIHelper.h>

// mitk core
#include <mitkExtractSliceFilter.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkPlanarFigure.h>
#include <mitkPropertyNameHelper.h>

// vtk
#include <vtkLookupTable.h>

QPixmap QmitkSemanticRelationsUIHelper::GetPixmapFromImageNode(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    return QPixmap();
  }

  const mitk::Image* image = static_cast<const mitk::Image*>(dataNode->GetData());
  if (nullptr == image || !image->IsInitialized())
  {
    return QPixmap();
  }

  if (image->GetPixelType().GetNumberOfComponents() != 1) // for now only single component are allowed
  {
    return QPixmap();
  }

  mitk::PlaneGeometry::Pointer sagittalPlaneGeometry = mitk::PlaneGeometry::New();
  int sliceNumber = image->GetDimension(1) / 2;
  sagittalPlaneGeometry->InitializeStandardPlane(image->GetGeometry(), mitk::PlaneGeometry::Sagittal, sliceNumber);

  mitk::ExtractSliceFilter::Pointer extractSliceFilter = mitk::ExtractSliceFilter::New();
  extractSliceFilter->SetInput(image);
  extractSliceFilter->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_CUBIC);
  extractSliceFilter->SetResliceTransformByGeometry(image->GetGeometry());
  extractSliceFilter->SetWorldGeometry(sagittalPlaneGeometry);
  extractSliceFilter->SetOutputDimensionality(2);
  extractSliceFilter->SetVtkOutputRequest(true);
  extractSliceFilter->Update();

  /*
  mitk::Vector3D spacing;
  mitk::FillVector3D(spacing,1.0,1.0,1.0);
  slice->GetGeometry()->SetSpacing(spacing);
  // save image slice
  mitk::IOUtil::SaveImage( slice, d->m_Path + fileName + ".png" );
  */

  vtkImageData* imageData = extractSliceFilter->GetVtkOutput();

  mitk::LevelWindow levelWindow;
  dataNode->GetLevelWindow(levelWindow);
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  lookupTable->SetSaturationRange(0.0, 0.0);
  lookupTable->SetValueRange(0.0, 1.0);
  lookupTable->SetHueRange(0.0, 0.0);
  lookupTable->SetRampToLinear();

  vtkSmartPointer<vtkMitkLevelWindowFilter> levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
  levelWindowFilter->SetLookupTable(lookupTable);
  levelWindowFilter->SetInputData(imageData);
  levelWindowFilter->SetMinOpacity(0.0);
  levelWindowFilter->SetMaxOpacity(1.0);
  int dims[3];
  imageData->GetDimensions(dims);
  double clippingBounds[] = { 0.0, static_cast<double>(dims[0]), 0.0, static_cast<double>(dims[1]) };
  levelWindowFilter->SetClippingBounds(clippingBounds);
  levelWindowFilter->Update();
  imageData = levelWindowFilter->GetOutput();

  QImage thumbnailImage(reinterpret_cast<const unsigned char*>(imageData->GetScalarPointer()), dims[0], dims[1], QImage::Format_ARGB32);

  thumbnailImage = thumbnailImage.rgbSwapped().mirrored(false, true);
  return QPixmap::fromImage(thumbnailImage);
}
