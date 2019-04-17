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

#include "mitkShowSegmentationAsSmoothedSurface.h"
#include "itkIntelligentBinaryClosingFilter.h"
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include <itkAddImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMultiplyImageFilter.h>
#include <itkRegionOfInterestImageFilter.h>
#include <mitkGeometry3D.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkUIDGenerator.h>
#include <mitkVtkRepresentationProperty.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>

using namespace mitk;
using namespace std;

ShowSegmentationAsSmoothedSurface::ShowSegmentationAsSmoothedSurface()
{
}

ShowSegmentationAsSmoothedSurface::~ShowSegmentationAsSmoothedSurface()
{
}

void ShowSegmentationAsSmoothedSurface::Initialize(const NonBlockingAlgorithm *other)
{
  Superclass::Initialize(other);

  bool syncVisibility = false;

  if (other != nullptr)
    other->GetParameter("Sync visibility", syncVisibility);

  SetParameter("Sync visibility", syncVisibility);
  SetParameter("Wireframe", false);

  // The Smoothing value is used as variance for a Gauss filter.
  // A reasonable default value equals the image spacing in mm.
  SetParameter("Smoothing", 1.0);

  // Valid range for decimation value is [0, 1). High values
  // increase decimation, especially when very close to 1.
  // A value of 0 disables decimation.
  SetParameter("Decimation", 0.5);

  // Valid range for closing value is [0, 1]. Higher values
  // increase closing. A value of 0 disables closing.
  SetParameter("Closing", 0.0);
}

bool ShowSegmentationAsSmoothedSurface::ReadyToRun()
{
  try
  {
    mitk::Image::Pointer image;
    GetPointerParameter("Input", image);

    return image.IsNotNull() && GetGroupNode();
  }
  catch (const invalid_argument &)
  {
    return false;
  }
}

bool ShowSegmentationAsSmoothedSurface::ThreadedUpdateFunction()
{
  Image::Pointer image;
  GetPointerParameter("Input", image);

  double smoothing;
  GetParameter("Smoothing", smoothing);

  double decimation;
  GetParameter("Decimation", decimation);

  double closing;
  GetParameter("Closing", closing);

  int timeNr = 0;
  GetParameter("TimeNr", timeNr);

  if (image->GetDimension() == 4)
    MITK_INFO << "CREATING SMOOTHED POLYGON MODEL (t = " << timeNr << ')';
  else
    MITK_INFO << "CREATING SMOOTHED POLYGON MODEL";

  MITK_INFO << "  Smoothing  = " << smoothing;
  MITK_INFO << "  Decimation = " << decimation;
  MITK_INFO << "  Closing    = " << closing;

  Geometry3D::Pointer geometry = dynamic_cast<Geometry3D *>(image->GetGeometry()->Clone().GetPointer());

  // Make ITK image out of MITK image

  typedef itk::Image<unsigned char, 3> CharImageType;
  typedef itk::Image<unsigned short, 3> ShortImageType;
  typedef itk::Image<float, 3> FloatImageType;

  if (image->GetDimension() == 4)
  {
    ImageTimeSelector::Pointer imageTimeSelector = ImageTimeSelector::New();
    imageTimeSelector->SetInput(image);
    imageTimeSelector->SetTimeNr(timeNr);
    imageTimeSelector->UpdateLargestPossibleRegion();
    image = imageTimeSelector->GetOutput(0);
  }

  ImageToItk<CharImageType>::Pointer imageToItkFilter = ImageToItk<CharImageType>::New();

  try
  {
    imageToItkFilter->SetInput(image);
  }
  catch (const itk::ExceptionObject &e)
  {
    // Most probably the input image type is wrong. Binary images are expected to be
    // >unsigned< char images.
    MITK_ERROR << e.GetDescription() << endl;
    return false;
  }

  imageToItkFilter->Update();

  CharImageType::Pointer itkImage = imageToItkFilter->GetOutput();

  // Get bounding box and relabel

  MITK_INFO << "Extracting VOI...";

  int imageLabel = 1;
  bool roiFound = false;

  CharImageType::IndexType minIndex;
  minIndex.Fill(numeric_limits<CharImageType::IndexValueType>::max());

  CharImageType::IndexType maxIndex;
  maxIndex.Fill(numeric_limits<CharImageType::IndexValueType>::min());

  itk::ImageRegionIteratorWithIndex<CharImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());

  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    if (iter.Get() == imageLabel)
    {
      roiFound = true;
      iter.Set(1);

      CharImageType::IndexType currentIndex = iter.GetIndex();

      for (unsigned int dim = 0; dim < 3; ++dim)
      {
        minIndex[dim] = min(currentIndex[dim], minIndex[dim]);
        maxIndex[dim] = max(currentIndex[dim], maxIndex[dim]);
      }
    }
    else
    {
      iter.Set(0);
    }
  }

  if (!roiFound)
  {
    ProgressBar::GetInstance()->Progress(8);
    MITK_ERROR << "Didn't found segmentation labeled with " << imageLabel << "!" << endl;
    return false;
  }

  ProgressBar::GetInstance()->Progress(1);

  // Extract and pad bounding box

  typedef itk::RegionOfInterestImageFilter<CharImageType, CharImageType> ROIFilterType;

  ROIFilterType::Pointer roiFilter = ROIFilterType::New();
  CharImageType::RegionType region;
  CharImageType::SizeType size;

  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    size[dim] = maxIndex[dim] - minIndex[dim] + 1;
  }

  region.SetIndex(minIndex);
  region.SetSize(size);

  roiFilter->SetInput(itkImage);
  roiFilter->SetRegionOfInterest(region);
  roiFilter->ReleaseDataFlagOn();
  roiFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::ConstantPadImageFilter<CharImageType, CharImageType> PadFilterType;

  PadFilterType::Pointer padFilter = PadFilterType::New();
  const PadFilterType::SizeValueType pad[3] = {10, 10, 10};

  padFilter->SetInput(roiFilter->GetOutput());
  padFilter->SetConstant(0);
  padFilter->SetPadLowerBound(pad);
  padFilter->SetPadUpperBound(pad);
  padFilter->ReleaseDataFlagOn();
  padFilter->ReleaseDataBeforeUpdateFlagOn();
  padFilter->Update();

  CharImageType::Pointer roiImage = padFilter->GetOutput();

  roiImage->DisconnectPipeline();
  roiFilter = nullptr;
  padFilter = nullptr;

  // Correct origin of real geometry (changed by cropping and padding)

  typedef Geometry3D::TransformType TransformType;

  TransformType::Pointer transform = TransformType::New();
  TransformType::OutputVectorType translation;

  for (unsigned int dim = 0; dim < 3; ++dim)
    translation[dim] = (int)minIndex[dim] - (int)pad[dim];

  transform->SetIdentity();
  transform->Translate(translation);
  geometry->Compose(transform, true);

  ProgressBar::GetInstance()->Progress(1);

  // Median

  MITK_INFO << "Median...";

  typedef itk::BinaryMedianImageFilter<CharImageType, CharImageType> MedianFilterType;

  MedianFilterType::Pointer medianFilter = MedianFilterType::New();
  CharImageType::SizeType radius = {{0}};

  medianFilter->SetRadius(radius);
  medianFilter->SetBackgroundValue(0);
  medianFilter->SetForegroundValue(1);
  medianFilter->SetInput(roiImage);
  medianFilter->ReleaseDataFlagOn();
  medianFilter->ReleaseDataBeforeUpdateFlagOn();
  medianFilter->Update();

  ProgressBar::GetInstance()->Progress(1);

  // Intelligent closing

  MITK_INFO << "Intelligent closing...";

  auto surfaceRatio = (unsigned int)((1.0f - closing) * 100.0f);

  typedef itk::IntelligentBinaryClosingFilter<CharImageType, ShortImageType> ClosingFilterType;

  ClosingFilterType::Pointer closingFilter = ClosingFilterType::New();

  closingFilter->SetInput(medianFilter->GetOutput());
  closingFilter->ReleaseDataFlagOn();
  closingFilter->ReleaseDataBeforeUpdateFlagOn();
  closingFilter->SetSurfaceRatio(surfaceRatio);
  closingFilter->Update();

  ShortImageType::Pointer closedImage = closingFilter->GetOutput();

  closedImage->DisconnectPipeline();
  roiImage = nullptr;
  medianFilter = nullptr;
  closingFilter = nullptr;

  ProgressBar::GetInstance()->Progress(1);

  // Gaussian blur

  MITK_INFO << "Gauss...";

  typedef itk::BinaryThresholdImageFilter<ShortImageType, FloatImageType> BinaryThresholdToFloatFilterType;

  BinaryThresholdToFloatFilterType::Pointer binThresToFloatFilter = BinaryThresholdToFloatFilterType::New();

  binThresToFloatFilter->SetInput(closedImage);
  binThresToFloatFilter->SetLowerThreshold(1);
  binThresToFloatFilter->SetUpperThreshold(1);
  binThresToFloatFilter->SetInsideValue(100);
  binThresToFloatFilter->SetOutsideValue(0);
  binThresToFloatFilter->ReleaseDataFlagOn();
  binThresToFloatFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::DiscreteGaussianImageFilter<FloatImageType, FloatImageType> GaussianFilterType;

  // From the following line on, IntelliSense (VS 2008) is broken. Any idea how to fix it?
  GaussianFilterType::Pointer gaussFilter = GaussianFilterType::New();

  gaussFilter->SetInput(binThresToFloatFilter->GetOutput());
  gaussFilter->SetUseImageSpacing(true);
  gaussFilter->SetVariance(smoothing);
  gaussFilter->ReleaseDataFlagOn();
  gaussFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::BinaryThresholdImageFilter<FloatImageType, CharImageType> BinaryThresholdFromFloatFilterType;

  BinaryThresholdFromFloatFilterType::Pointer binThresFromFloatFilter = BinaryThresholdFromFloatFilterType::New();

  binThresFromFloatFilter->SetInput(gaussFilter->GetOutput());
  binThresFromFloatFilter->SetLowerThreshold(50);
  binThresFromFloatFilter->SetUpperThreshold(255);
  binThresFromFloatFilter->SetInsideValue(1);
  binThresFromFloatFilter->SetOutsideValue(0);
  binThresFromFloatFilter->ReleaseDataFlagOn();
  binThresFromFloatFilter->ReleaseDataBeforeUpdateFlagOn();
  binThresFromFloatFilter->Update();

  CharImageType::Pointer blurredImage = binThresFromFloatFilter->GetOutput();

  blurredImage->DisconnectPipeline();
  closedImage = nullptr;
  binThresToFloatFilter = nullptr;
  gaussFilter = nullptr;

  ProgressBar::GetInstance()->Progress(1);

  // Fill holes

  MITK_INFO << "Filling cavities...";

  typedef itk::ConnectedThresholdImageFilter<CharImageType, CharImageType> ConnectedThresholdFilterType;

  ConnectedThresholdFilterType::Pointer connectedThresFilter = ConnectedThresholdFilterType::New();

  CharImageType::IndexType corner;

  corner[0] = 0;
  corner[1] = 0;
  corner[2] = 0;

  connectedThresFilter->SetInput(blurredImage);
  connectedThresFilter->SetSeed(corner);
  connectedThresFilter->SetLower(0);
  connectedThresFilter->SetUpper(0);
  connectedThresFilter->SetReplaceValue(2);
  connectedThresFilter->ReleaseDataFlagOn();
  connectedThresFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::BinaryThresholdImageFilter<CharImageType, CharImageType> BinaryThresholdFilterType;

  BinaryThresholdFilterType::Pointer binThresFilter = BinaryThresholdFilterType::New();

  binThresFilter->SetInput(connectedThresFilter->GetOutput());
  binThresFilter->SetLowerThreshold(0);
  binThresFilter->SetUpperThreshold(0);
  binThresFilter->SetInsideValue(50);
  binThresFilter->SetOutsideValue(0);
  binThresFilter->ReleaseDataFlagOn();
  binThresFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::AddImageFilter<CharImageType, CharImageType, CharImageType> AddFilterType;

  AddFilterType::Pointer addFilter = AddFilterType::New();

  addFilter->SetInput1(blurredImage);
  addFilter->SetInput2(binThresFilter->GetOutput());
  addFilter->ReleaseDataFlagOn();
  addFilter->ReleaseDataBeforeUpdateFlagOn();
  addFilter->Update();

  ProgressBar::GetInstance()->Progress(1);

  // Surface extraction

  MITK_INFO << "Surface extraction...";

  Image::Pointer filteredImage = Image::New();
  CastToMitkImage(addFilter->GetOutput(), filteredImage);

  filteredImage->SetGeometry(geometry);

  ImageToSurfaceFilter::Pointer imageToSurfaceFilter = ImageToSurfaceFilter::New();

  imageToSurfaceFilter->SetInput(filteredImage);
  imageToSurfaceFilter->SetThreshold(50);
  imageToSurfaceFilter->SmoothOn();
  imageToSurfaceFilter->SetDecimate(ImageToSurfaceFilter::NoDecimation);

  m_Surface = imageToSurfaceFilter->GetOutput(0);

  ProgressBar::GetInstance()->Progress(1);

  // Mesh decimation

  if (decimation > 0.0f && decimation < 1.0f)
  {
    MITK_INFO << "Quadric mesh decimation...";

    vtkQuadricDecimation *quadricDecimation = vtkQuadricDecimation::New();
    quadricDecimation->SetInputData(m_Surface->GetVtkPolyData());
    quadricDecimation->SetTargetReduction(decimation);
    quadricDecimation->AttributeErrorMetricOn();
    quadricDecimation->GlobalWarningDisplayOff();
    quadricDecimation->Update();

    vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
    cleaner->SetInputConnection(quadricDecimation->GetOutputPort());
    cleaner->PieceInvariantOn();
    cleaner->ConvertLinesToPointsOn();
    cleaner->ConvertStripsToPolysOn();
    cleaner->PointMergingOn();
    cleaner->Update();

    m_Surface->SetVtkPolyData(cleaner->GetOutput());
  }

  ProgressBar::GetInstance()->Progress(1);

  // Compute Normals

  vtkPolyDataNormals *computeNormals = vtkPolyDataNormals::New();
  computeNormals->SetInputData(m_Surface->GetVtkPolyData());
  computeNormals->SetFeatureAngle(360.0f);
  computeNormals->AutoOrientNormalsOn();
  computeNormals->FlipNormalsOff();
  computeNormals->Update();

  m_Surface->SetVtkPolyData(computeNormals->GetOutput());

  return true;
}

void ShowSegmentationAsSmoothedSurface::ThreadedUpdateSuccessful()
{
  DataNode::Pointer node = DataNode::New();

  bool wireframe = false;
  GetParameter("Wireframe", wireframe);

  if (wireframe)
  {
    auto *representation =
      dynamic_cast<VtkRepresentationProperty *>(node->GetProperty("material.representation"));

    if (representation != nullptr)
      representation->SetRepresentationToWireframe();
  }

  node->SetProperty("opacity", FloatProperty::New(1.0));
  node->SetProperty("line width", IntProperty::New(1));
  node->SetProperty("scalar visibility", BoolProperty::New(false));

  std::string groupNodeName = "surface";
  DataNode *groupNode = GetGroupNode();

  if (groupNode != nullptr)
    groupNode->GetName(groupNodeName);

  node->SetProperty("name", StringProperty::New(groupNodeName));
  node->SetData(m_Surface);

  BaseProperty *colorProperty = groupNode->GetProperty("color");

  if (colorProperty != nullptr)
    node->ReplaceProperty("color", colorProperty->Clone());
  else
    node->SetProperty("color", ColorProperty::New(1.0f, 0.0f, 0.0f));

  bool showResult = true;
  GetParameter("Show result", showResult);

  bool syncVisibility = false;
  GetParameter("Sync visibility", syncVisibility);

  Image::Pointer image;
  GetPointerParameter("Input", image);

  BaseProperty *organTypeProperty = image->GetProperty("organ type");

  if (organTypeProperty != nullptr)
    m_Surface->SetProperty("organ type", organTypeProperty);

  BaseProperty *visibleProperty = groupNode->GetProperty("visible");

  if (visibleProperty != nullptr && syncVisibility)
    node->ReplaceProperty("visible", visibleProperty->Clone());
  else
    node->SetProperty("visible", BoolProperty::New(showResult));

  InsertBelowGroupNode(node);

  Superclass::ThreadedUpdateSuccessful();
}
