/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkShowSegmentationAsSmoothedSurface.h"
#include "mitkImageToItk.h"
#include "itkIntelligentBinaryClosingFilter.h"
#include <mitkUIDGenerator.h>
#include <mitkGeometry3D.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkVtkRepresentationProperty.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkAddImageFilter.h>
#include <vtkQuadricDecimation.h>
#include <vtkCleanPolyData.h>

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

  if (other != NULL)
    other->GetParameter("Sync visibility", syncVisibility);

  SetParameter("Sync visibility", syncVisibility);
  SetParameter("Wireframe", false);

  // The Smoothing value is used as variance for a Gauﬂ filter.
  // A reasonable default value equals the image spacing in mm.
  SetParameter("Smoothing", 1.0f);

  // Valid range for dacimation value is ]0, 1[. High values
  // increase decimation, especially when very close to 1.
  SetParameter("Decimation", 0.5f);
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

  float smoothing;
  GetParameter("Smoothing", smoothing);

  float decimation;
  GetParameter("Decimation", decimation);

  MITK_INFO << "CREATING SMOOTHED POLYGON MODEL" << endl;
  MITK_INFO << "  Smoothing = " << smoothing << endl;
  MITK_INFO << "  Decimation = " << decimation << endl;
 
  Geometry3D::Pointer geometry = image->GetGeometry();
  
  // Make ITK image out of MITK image
  
  typedef itk::Image<unsigned char, 3> CharImageType;
  typedef itk::Image<unsigned short, 3> ShortImageType;
  typedef itk::Image<float, 3> FloatImageType;

  ImageToItk<CharImageType>::Pointer imageToItkFilter = ImageToItk<CharImageType>::New();
  imageToItkFilter->SetInput(image);
  imageToItkFilter->Update();

  CharImageType::Pointer itkImage = imageToItkFilter->GetOutput();

  // Get bounding box and relabel

  MITK_INFO << "Extracting VOI..." << endl;

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
    ProgressBar::GetInstance()->Progress(10);
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
  const PadFilterType::SizeValueType pad[3] = { 10, 10, 10 };

  padFilter->SetInput(roiFilter->GetOutput());
  padFilter->SetConstant(0);
  padFilter->SetPadLowerBound(pad);
  padFilter->SetPadUpperBound(pad);
  padFilter->ReleaseDataFlagOn();
  padFilter->ReleaseDataBeforeUpdateFlagOn();
  padFilter->Update();

  CharImageType::Pointer roiImage = padFilter->GetOutput();
  
  roiImage->DisconnectPipeline();
  roiFilter = 0;
  padFilter = 0;

  // Correct origin of real geometry (changed by cropping and padding)

  typedef AffineGeometryFrame3D::TransformType TransformType;

  TransformType::Pointer transform = TransformType::New();
  TransformType::OutputVectorType translation;

  for (unsigned int dim = 0; dim < 3; ++dim)
    translation[dim] = (int)minIndex[dim] - (int)pad[dim];

  transform->SetIdentity();
  transform->Translate(translation);
  geometry->Compose(transform, true);

  ProgressBar::GetInstance()->Progress(1);

  // Median

  MITK_INFO << "Median..." << endl;

  typedef itk::BinaryMedianImageFilter<CharImageType, CharImageType> MedianFilterType;

  MedianFilterType::Pointer medianFilter = MedianFilterType::New();
  CharImageType::SizeType radius = { 1, 1, 1 };

  medianFilter->SetRadius(radius);
  medianFilter->SetBackgroundValue(0);
  medianFilter->SetForegroundValue(1);
  medianFilter->SetInput(roiImage);
  medianFilter->ReleaseDataFlagOn();
  medianFilter->ReleaseDataBeforeUpdateFlagOn();
  medianFilter->Update();

  ProgressBar::GetInstance()->Progress(1);

  // Intelligent closing

  MITK_INFO << "Intelligent closing..." << endl;

  unsigned int surfaceRatio = 70;

  typedef itk::IntelligentBinaryClosingFilter<CharImageType, ShortImageType> ClosingFilterType;

  ClosingFilterType::Pointer closingFilter = ClosingFilterType::New();

  closingFilter->SetInput(medianFilter->GetOutput());
  closingFilter->ReleaseDataFlagOn();
  closingFilter->ReleaseDataBeforeUpdateFlagOn();
  closingFilter->SetSurfaceRatio(surfaceRatio);
  closingFilter->Update();
  
  ShortImageType::Pointer closedImage = closingFilter->GetOutput();

  closedImage->DisconnectPipeline();
  roiImage = 0;
  medianFilter = 0;
  closingFilter = 0;

  ProgressBar::GetInstance()->Progress(1);

  // Gaussian blur

  MITK_INFO << "Gauss..." << endl;

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
  gaussFilter->Update();

  FloatImageType::Pointer blurredImage = gaussFilter->GetOutput();
  
  blurredImage->DisconnectPipeline();
  closedImage = 0;
  binThresToFloatFilter = 0;
  gaussFilter = 0;

  ProgressBar::GetInstance()->Progress(1);

  // Isolate largest component

  MITK_INFO << "Isolate largest component..." << endl;

  typedef itk::BinaryThresholdImageFilter<FloatImageType, CharImageType> BinaryThresholdFromFloatFilterType;

  BinaryThresholdFromFloatFilterType::Pointer binThresFromFloatFilter = BinaryThresholdFromFloatFilterType::New();

  binThresFromFloatFilter->SetInput(blurredImage);
  binThresFromFloatFilter->SetLowerThreshold(50);
  binThresFromFloatFilter->SetUpperThreshold(255);
  binThresFromFloatFilter->SetInsideValue(1);
  binThresFromFloatFilter->SetOutsideValue(0);
  binThresFromFloatFilter->ReleaseDataFlagOn();
  binThresFromFloatFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::ConnectedComponentImageFilter<CharImageType, ShortImageType> ConnectedComponentFilterType;

  ConnectedComponentFilterType::Pointer connectedComponentFilter = ConnectedComponentFilterType::New();

  connectedComponentFilter->SetInput(binThresFromFloatFilter->GetOutput());

  typedef itk::RelabelComponentImageFilter<ShortImageType, CharImageType> RelabelComponentFilterType;

  RelabelComponentFilterType::Pointer relabelComponentFilter = RelabelComponentFilterType::New();

  relabelComponentFilter->SetInput(connectedComponentFilter->GetOutput());
  relabelComponentFilter->ReleaseDataFlagOn();
  relabelComponentFilter->ReleaseDataBeforeUpdateFlagOn();
  relabelComponentFilter->Update();

  MITK_INFO << "  " << relabelComponentFilter->GetNumberOfObjects() << " components found" << endl;

  typedef itk::BinaryThresholdImageFilter<CharImageType, CharImageType> BinaryThresholdFilterType;

  BinaryThresholdFilterType::Pointer binThresFilter = BinaryThresholdFilterType::New();

  binThresFilter->SetInput(relabelComponentFilter->GetOutput());
  binThresFilter->SetLowerThreshold(2);
  binThresFilter->SetUpperThreshold(255);
  binThresFilter->SetInsideValue(0);
  binThresFilter->SetOutsideValue(1);
  binThresFilter->ReleaseDataFlagOn();
  binThresFilter->ReleaseDataBeforeUpdateFlagOn();
  binThresFilter->Update();

  typedef itk::MultiplyImageFilter<FloatImageType, CharImageType, CharImageType> MultiplyFilterType;

  MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();

  multiplyFilter->SetInput1(blurredImage);
  multiplyFilter->SetInput2(binThresFilter->GetOutput());
  multiplyFilter->ReleaseDataFlagOn();
  multiplyFilter->ReleaseDataBeforeUpdateFlagOn();
  multiplyFilter->Update();

  CharImageType::Pointer singleComponentImage = multiplyFilter->GetOutput();
  
  singleComponentImage->DisconnectPipeline();
  blurredImage = 0;
  binThresFromFloatFilter = 0;
  connectedComponentFilter = 0;
  relabelComponentFilter = 0;
  binThresFilter = 0;
  multiplyFilter = 0;

  // Fill holes

  ProgressBar::GetInstance()->Progress(1);
  
  MITK_INFO << "Filling cavities..." << endl;

  binThresFilter = BinaryThresholdFilterType::New();

  binThresFilter->SetInput(singleComponentImage);
  binThresFilter->SetLowerThreshold(50);
  binThresFilter->SetUpperThreshold(255);
  binThresFilter->SetInsideValue(1);
  binThresFilter->SetOutsideValue(0);
  binThresFilter->ReleaseDataFlagOn();
  binThresFilter->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::ConnectedThresholdImageFilter<CharImageType, CharImageType> ConnectedThresholdFilterType;

  ConnectedThresholdFilterType::Pointer connectedThresFilter = ConnectedThresholdFilterType::New();

  CharImageType::IndexType corner;

  corner[0] = 0;
  corner[1] = 0;
  corner[2] = 0;

  connectedThresFilter->SetInput(binThresFilter->GetOutput());
  connectedThresFilter->SetSeed(corner);
  connectedThresFilter->SetLower(0);
  connectedThresFilter->SetUpper(0);
  connectedThresFilter->SetReplaceValue(2);
  connectedThresFilter->ReleaseDataFlagOn();
  connectedThresFilter->ReleaseDataBeforeUpdateFlagOn();

  BinaryThresholdFilterType::Pointer binThresFilter2 = BinaryThresholdFilterType::New();

  binThresFilter2->SetInput(connectedThresFilter->GetOutput());
  binThresFilter2->SetLowerThreshold(0);
  binThresFilter2->SetUpperThreshold(0);
  binThresFilter2->SetInsideValue(50);
  binThresFilter2->SetOutsideValue(0);
  binThresFilter2->ReleaseDataFlagOn();
  binThresFilter2->ReleaseDataBeforeUpdateFlagOn();

  typedef itk::AddImageFilter<CharImageType, CharImageType, CharImageType> AddFilterType;

  AddFilterType::Pointer addFilter = AddFilterType::New();

  addFilter->SetInput1(singleComponentImage);
  addFilter->SetInput2(binThresFilter2->GetOutput());
  addFilter->ReleaseDataFlagOn();
  addFilter->ReleaseDataBeforeUpdateFlagOn();
  addFilter->Update();

  ProgressBar::GetInstance()->Progress(1);

  // Surface extraction

  MITK_INFO << "Surface extraction..." << endl;

  Image::Pointer filteredImage = Image::New();
  CastToMitkImage(addFilter->GetOutput(), filteredImage);

  filteredImage->SetGeometry(geometry);

  ImageToSurfaceFilter::Pointer imageToSurfaceFilter = ImageToSurfaceFilter::New();

  imageToSurfaceFilter->SetInput(filteredImage);
  imageToSurfaceFilter->SetThreshold(50);
  imageToSurfaceFilter->SmoothOn();
  imageToSurfaceFilter->SetDecimate(ImageToSurfaceFilter::NoDecimation);

  m_Surface = imageToSurfaceFilter->GetOutput();

  ProgressBar::GetInstance()->Progress(1);

  // Mesh decimation

  if (decimation > 0.0f && decimation < 1.0f)
  {
    MITK_INFO << "Quadric mesh decimation..." << endl;

    vtkQuadricDecimation *quadricDecimation = vtkQuadricDecimation::New();
    quadricDecimation->SetInput(m_Surface->GetVtkPolyData());
    quadricDecimation->SetTargetReduction(decimation);
    quadricDecimation->AttributeErrorMetricOn();
    quadricDecimation->Update();

    vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
    cleaner->SetInput(quadricDecimation->GetOutput());
    cleaner->PieceInvariantOn();
    cleaner->ConvertLinesToPointsOn();
    cleaner->ConvertStripsToPolysOn();
    cleaner->PointMergingOn();
    cleaner->Update();

    m_Surface->SetVtkPolyData(cleaner->GetOutput());
  }

  ProgressBar::GetInstance()->Progress(1);

  return true;
}

void ShowSegmentationAsSmoothedSurface::ThreadedUpdateSuccessful()
{
  DataNode::Pointer node = LookForPointerTargetBelowGroupNode("Surface representation");
  bool addToTree = node.IsNull();

  if (addToTree)
  {
    node = DataNode::New();

    bool wireframe = false;
    GetParameter("Wireframe", wireframe);

    if (wireframe)
    {
      VtkRepresentationProperty *representation = dynamic_cast<VtkRepresentationProperty *>(
        node->GetProperty("material.representation"));

      if (representation != NULL)
        representation->SetRepresentationToWireframe();
    }

    node->SetProperty("opacity", FloatProperty::New(0.3));
    node->SetProperty("line width", IntProperty::New(1));
    node->SetProperty("scalar visibility", BoolProperty::New(false));

    UIDGenerator uidGenerator("Surface_");
    node->SetProperty("FILENAME", StringProperty::New(uidGenerator.GetUID() + ".vtk"));

    std::string groupNodeName = "surface";
    DataNode *groupNode = GetGroupNode();

    if (groupNode != NULL)
      groupNode->GetName(groupNodeName);

    node->SetProperty("name", StringProperty::New(groupNodeName));
  }

  node->SetData(m_Surface);

  if (addToTree)
  {
    DataNode* groupNode = GetGroupNode();

    if (groupNode != NULL)
    {
      groupNode->SetProperty("Surface representation", SmartPointerProperty::New(node));

      BaseProperty *colorProperty = groupNode->GetProperty("color");

      if (colorProperty != NULL)
        node->ReplaceProperty("color", colorProperty);
      else
        node->SetProperty("color", ColorProperty::New(1.0f, 1.0f, 0.0f));

      bool showResult = true;
      GetParameter("Show result", showResult);

      bool syncVisibility = false;
      GetParameter("Sync visibility", syncVisibility);

      Image::Pointer image;
      GetPointerParameter("Input", image);

      BaseProperty *organTypeProperty = image->GetProperty("organ type");

      if (organTypeProperty != NULL)
        m_Surface->SetProperty("organ type", organTypeProperty);

      BaseProperty *visibleProperty = groupNode->GetProperty("visible");

      if (visibleProperty != NULL && syncVisibility)
        node->ReplaceProperty("visible", visibleProperty);
      else
        node->SetProperty("visible", BoolProperty::New(showResult));
    }

    InsertBelowGroupNode(node);
  }

  StatusBar::GetInstance()->Clear();

  Superclass::ThreadedUpdateSuccessful();
}
