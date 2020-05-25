#include "mitkShowSegmentationAsAgtkSurface.h"

#include <itkMinimumMaximumImageCalculator.h>
#include <itkBinaryThresholdImageFilter.h>

#include <vtkCallbackCommand.h>
#include <vtkDecimatePro.h>
#include <vtkFlyingEdges3D.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>

namespace mitk
{

ShowSegmentationAsAgtkSurface::ShowSegmentationAsAgtkSurface()
{
  m_CurrentProgress = 0.f;
  m_ProgressWeight = 0.f;

  m_ProgressAccumulator = itk::ProgressAccumulator::New();

  m_VtkProgressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  m_VtkProgressCallback->SetCallback(this->vtkOnProgress);
  m_VtkProgressCallback->SetClientData(this);

  m_VtkEndCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  m_VtkEndCallback->SetCallback(this->vtkOnEnd);
  m_VtkEndCallback->SetClientData(this);
}

bool ShowSegmentationAsAgtkSurface::PreProcessing()
{
  static const int BACKGROUND_VALUE = 0;
  static const int FOREGROUND_VALUE = 1;
  static const int PAD_SIZE = 10;

  InputImageType::RegionType region = m_Input->GetLargestPossibleRegion();
  itk::ImageRegionConstIteratorWithIndex<InputImageType> it(m_Input, region);
  InputImageType::IndexType minIndex = region.GetUpperIndex();
  InputImageType::IndexType maxIndex = region.GetIndex();
  size_t labeledPixels = 0;

  for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
    if (it.Get() > BACKGROUND_VALUE) {
      labeledPixels++;
      InputImageType::IndexType index = it.GetIndex();

      for (size_t i = 0; i < DIM; ++i) {
        minIndex[i] = std::min(minIndex[i], index[i]);
        maxIndex[i] = std::max(maxIndex[i], index[i]);
      }
    }
  }

  for (size_t i = 0; i < DIM; ++i) {

    if (minIndex[i] > maxIndex[i]) {
      return false;
    }

    minIndex[i] = minIndex[i] - PAD_SIZE;
    maxIndex[i] = maxIndex[i] + PAD_SIZE;

    if (!m_FilterArgs.isFillHoles) {
      minIndex[i] = std::max(minIndex[i], region.GetIndex()[i]);
      maxIndex[i] = std::min(maxIndex[i], region.GetUpperIndex()[i]);
    }
  }

  region.SetIndex(minIndex);
  region.SetUpperIndex(maxIndex);

  InputImageType::PointType origin;
  m_Input->TransformIndexToPhysicalPoint(region.GetIndex(), origin);

  typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double> InterpolatorType;
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typedef itk::ResampleImageFilter<InputImageType, FloatImage> ResampleImageFilterType;
  typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
  m_ProgressAccumulator->RegisterInternalFilter(resampler, m_ProgressWeight);
  resampler->SetInterpolator(interpolator);
  resampler->SetSize(region.GetSize());
  resampler->SetOutputOrigin(origin);
  resampler->SetOutputSpacing(m_Input->GetSpacing());
  resampler->SetOutputDirection(m_Input->GetDirection());
  resampler->SetDefaultPixelValue(BACKGROUND_VALUE);

  auto maxCalculator = itk::MinimumMaximumImageCalculator<InputImageType>::New();
  maxCalculator->SetImage(m_Input);
  maxCalculator->ComputeMaximum();
  if (maxCalculator->GetMaximum() > FOREGROUND_VALUE) {
    using ThresholderType = itk::BinaryThresholdImageFilter<InputImageType, InputImageType>;
    ThresholderType::Pointer thresholder = ThresholderType::New();
    thresholder->SetInput(m_Input);
    thresholder->SetLowerThreshold(FOREGROUND_VALUE);
    thresholder->SetUpperThreshold(std::numeric_limits<unsigned char>::max());
    thresholder->SetOutsideValue(BACKGROUND_VALUE);
    thresholder->SetInsideValue(FOREGROUND_VALUE);
    thresholder->Update();

    resampler->SetInput(thresholder->GetOutput());
  } else {
    resampler->SetInput(m_Input);
  }

  resampler->Update();
  m_FloatTmpImage = resampler->GetOutput();

  typedef itk::SignedMaurerDistanceMapImageFilter<FloatImage, FloatImage> DistanceMapType;
  typename DistanceMapType::Pointer distanceToForeground = DistanceMapType::New();
  m_ProgressAccumulator->RegisterInternalFilter(distanceToForeground, m_ProgressWeight);
  distanceToForeground->SetInput(m_FloatTmpImage);
  distanceToForeground->SetUseImageSpacing(true);
  distanceToForeground->SetBackgroundValue(BACKGROUND_VALUE);
  distanceToForeground->SetInsideIsPositive(false);

  typename DistanceMapType::Pointer distanceToBackground = DistanceMapType::New();
  m_ProgressAccumulator->RegisterInternalFilter(distanceToBackground, m_ProgressWeight);
  distanceToBackground->SetInput(m_FloatTmpImage);
  distanceToBackground->SetUseImageSpacing(true);
  distanceToBackground->SetBackgroundValue(FOREGROUND_VALUE);
  distanceToBackground->SetInsideIsPositive(true);

  typedef itk::AddImageFilter<FloatImage> AddImageFilterType;
  typename AddImageFilterType::Pointer add = AddImageFilterType::New();
  m_ProgressAccumulator->RegisterInternalFilter(add, m_ProgressWeight);
  add->SetInput1(distanceToForeground->GetOutput());
  add->SetInput2(distanceToBackground->GetOutput());
  add->InPlaceOn();

  typedef itk::MultiplyImageFilter<FloatImage> MultiplyImageFilterType;
  MultiplyImageFilterType::Pointer multiply = MultiplyImageFilterType::New();
  m_ProgressAccumulator->RegisterInternalFilter(multiply, m_ProgressWeight);
  multiply->SetInput(add->GetOutput());
  multiply->SetConstant(.5);
  multiply->InPlaceOn();

  typedef itk::DiscreteGaussianImageFilter<FloatImage, FloatImage> BlurImageFilterType;
  typename BlurImageFilterType::Pointer smoothingImage = BlurImageFilterType::New();
  m_ProgressAccumulator->RegisterInternalFilter(smoothingImage, m_ProgressWeight);
  smoothingImage->SetInput(multiply->GetOutput());
  smoothingImage->SetUseImageSpacing(false);
  smoothingImage->SetVariance(m_FilterArgs.blurSigma * m_FilterArgs.blurSigma);
  smoothingImage->Update();

  m_FloatTmpImage = smoothingImage->GetOutput();

  double minSpacing = std::min(m_FilterArgs.spacing[0], m_FilterArgs.spacing[1]);
  minSpacing = std::min(minSpacing, m_FilterArgs.spacing[3]);

  if (m_FilterArgs.isResampling && minSpacing > 1.0e-05) {
    typedef itk::LinearInterpolateImageFunction<FloatImage, double> InterpolatorType;
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

    FloatImage::SizeType oldSize       = m_FloatTmpImage->GetLargestPossibleRegion().GetSize();
    FloatImage::SpacingType oldSpacing = m_FloatTmpImage->GetSpacing();

    FloatImage::SizeType newSize;
    FloatImage::SpacingType newSpacing;

    for (size_t n = 0; n < DIM; ++n) {
      newSize[n] = (oldSize[n] - 1) * oldSpacing[n] / m_FilterArgs.spacing[n] + 1;
      newSpacing[n] = m_FilterArgs.spacing[n];
    }

    typedef itk::ResampleImageFilter<FloatImage, FloatImage> ResampleImageFilterType;
    typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
    m_ProgressAccumulator->RegisterInternalFilter(resampler, m_ProgressWeight);
    resampler->SetInput(m_FloatTmpImage);
    resampler->SetInterpolator(interpolator);
    resampler->SetSize(newSize);
    resampler->SetOutputSpacing(newSpacing);
    resampler->SetOutputOrigin(m_FloatTmpImage->GetOrigin());
    resampler->SetOutputDirection(m_FloatTmpImage->GetDirection());
    resampler->Update();
    m_FloatTmpImage = resampler->GetOutput();
  }

  return true;
}

void ShowSegmentationAsAgtkSurface::ComputeSurface()
{
  typedef itk::ImageToVTKImageFilter<FloatImage> ItkToVtkConverter;
  typename ItkToVtkConverter::Pointer itkToVtk = ItkToVtkConverter::New();
  m_ProgressAccumulator->RegisterInternalFilter(itkToVtk, m_ProgressWeight);
  itkToVtk->SetInput(m_FloatTmpImage);
  itkToVtk->Update();
  vtkSmartPointer<vtkImageData> vtkImage = itkToVtk->GetOutput();

  m_CurrentProgress = m_ProgressAccumulator->GetAccumulatedProgress();
  m_ProgressAccumulator->UnregisterAllFilters();

  itk::Matrix<double, DIM, DIM> tmpDirection = m_FloatTmpImage->GetDirection();
  itk::Vector<double, DIM> tmpOrigin = m_FloatTmpImage->GetOrigin().GetDataPointer();
  m_FloatTmpImage->DisconnectPipeline();
  m_FloatTmpImage = nullptr;

  typedef vtkSmartPointer<vtkFlyingEdges3D> FlyingEdges;
  FlyingEdges flyingEdges = FlyingEdges::New();
  flyingEdges->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
  flyingEdges->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
  flyingEdges->SetInputData(vtkImage);
  flyingEdges->ComputeNormalsOff();
  flyingEdges->ComputeGradientsOn();
  flyingEdges->ComputeScalarsOn();

  flyingEdges->Update();
  m_Output = flyingEdges->GetOutput();

  vtkSmartPointer<vtkPoints> points = m_Output->GetPoints();

  for (size_t n = 0; n < points->GetNumberOfPoints(); ++n) {
    itk::Vector<double, DIM> point = points->GetPoint(n);
    point = tmpDirection * (point - tmpOrigin) + tmpOrigin;
    points->SetPoint(n, point.GetDataPointer());
  }
}

void ShowSegmentationAsAgtkSurface::PostProcessing()
{
  static const double FEATURE_ANGLE = 60.;
  static const double PASS_BAND = .001;

  switch (m_FilterArgs.decimationType) {
    case SurfaceDecimationType::DecimatePro: {
      typedef vtkSmartPointer<vtkDecimatePro> Decimate;
      Decimate decimate = Decimate::New();
      decimate->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      decimate->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      decimate->SetInputData(m_Output);
      decimate->SetSplitting(false);
      decimate->SetErrorIsAbsolute(5);
      decimate->SetFeatureAngle(FEATURE_ANGLE);
      decimate->SetPreserveTopology(true);
      decimate->SetBoundaryVertexDeletion(false);
      decimate->SetDegree(10); // Standard value is 25
      decimate->SetTargetReduction(m_FilterArgs.reduction);
      decimate->SetMaximumError(.002);
      decimate->Update();
      m_Output = decimate->GetOutput();
      break;
    }
    case SurfaceDecimationType::QuadricDecimation: {
      typedef vtkSmartPointer<vtkQuadricDecimation> Decimation;
      Decimation decimate = Decimation::New();
      decimate->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      decimate->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      decimate->SetInputData(m_Output);
      decimate->SetTargetReduction(m_FilterArgs.reduction);
      decimate->Update();
      m_Output = decimate->GetOutput();
      break;
    }
  }

  switch (m_FilterArgs.smoothingType) {
    case SurfaceSmoothingType::Laplacian: {
      typedef vtkSmartPointer<vtkSmoothPolyDataFilter> SmoothPolyData;
      SmoothPolyData smooth = SmoothPolyData::New();
      smooth->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      smooth->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      smooth->SetInputData(m_Output);
      smooth->SetNumberOfIterations(m_FilterArgs.smoothingIterations);
      smooth->SetRelaxationFactor(m_FilterArgs.smoothingRelaxation);
      smooth->SetFeatureAngle(FEATURE_ANGLE);
      smooth->SetConvergence(0);
      smooth->SetBoundarySmoothing(false);
      smooth->SetFeatureEdgeSmoothing(false);
      smooth->Update();
      m_Output = smooth->GetOutput();
      break;
    }
    case SurfaceSmoothingType::WindowedSync: {
      typedef vtkSmartPointer<vtkWindowedSincPolyDataFilter> SmoothPolyData;
      SmoothPolyData smooth = SmoothPolyData::New();
      smooth->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
      smooth->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
      smooth->SetInputData(m_Output);
      smooth->SetNumberOfIterations(m_FilterArgs.smoothingIterations);
      smooth->SetFeatureEdgeSmoothing(false);
      smooth->SetFeatureAngle(FEATURE_ANGLE);
      smooth->SetPassBand(PASS_BAND);
      smooth->SetNonManifoldSmoothing(true);
      smooth->SetNormalizeCoordinates(true);
      smooth->SetBoundarySmoothing(false);
      smooth->Update();
      m_Output = smooth->GetOutput();
      break;
    }
  }

  // Compute normals
  typedef vtkSmartPointer<vtkPolyDataNormals> PolyDataNormals;
  PolyDataNormals normals = PolyDataNormals::New();
  normals->AddObserver(vtkCommand::ProgressEvent, m_VtkProgressCallback);
  normals->AddObserver(vtkCommand::EndEvent, m_VtkEndCallback);
  normals->SetInputData(m_Output);
  normals->AutoOrientNormalsOn();
  normals->FlipNormalsOff();
  normals->ConsistencyOn();
  normals->SplittingOff();
  normals->ComputeCellNormalsOff();
  normals->ComputePointNormalsOn();
  normals->Update();
  m_Output = normals->GetOutput();
}

void ShowSegmentationAsAgtkSurface::GenerateData()
{
  double minSpacing = std::min(m_FilterArgs.spacing[0], m_FilterArgs.spacing[1]);
  minSpacing = std::min(minSpacing, m_FilterArgs.spacing[3]);

  m_ProgressWeight = 1.f / (1 // Resample
      + (float)m_SurfaceArgs.smooth * 5 // Smoothing
      + (float)(m_FilterArgs.isResampling && minSpacing > 1.0e-05) // Additional resampling
      + 1 // Itk To Vtk
      + 1 // Marching Cubes
      + (float)(m_FilterArgs.decimationType != SurfaceDecimationType::None) // Decimation
      + (float)(m_FilterArgs.smoothingType != SurfaceSmoothingType::None) // Smoothing
      + 1); // Compute Normals

  m_ProgressAccumulator->ResetProgress();
  m_ProgressAccumulator->SetMiniPipelineFilter(this);
  m_ProgressAccumulator->UnregisterAllFilters();

  if (PreProcessing()) {
    ComputeSurface();
    PostProcessing();
  } else {
    MITK_WARN << "Creating empty surface!";
    m_Output = vtkSmartPointer<vtkPolyData>::New();
  }

  m_ProgressAccumulator->SetMiniPipelineFilter(nullptr);
  m_ProgressAccumulator->UnregisterAllFilters();

  UpdateProgress(1.);
}

void ShowSegmentationAsAgtkSurface::vtkOnProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
    void* clientData, void* vtkNotUsed(callData))
{
  vtkPolyDataAlgorithm* filter = static_cast<vtkPolyDataAlgorithm*>(caller);

  auto builder = static_cast<mitk::ShowSegmentationAsAgtkSurface*>(clientData);

  builder->UpdateProgress(builder->GetProgress() + filter->GetProgress() * builder->GetProgressWeight());

  if (builder->GetAbortGenerateData()) {
    throw itk::ProcessAborted();
  }
}

void ShowSegmentationAsAgtkSurface::vtkOnEnd(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
    void* clientData, void* vtkNotUsed(callData))
{
  auto builder = static_cast<mitk::ShowSegmentationAsAgtkSurface*>(clientData);

  builder->SetProgress(builder->GetProgress() + builder->GetProgressWeight());
}

}

