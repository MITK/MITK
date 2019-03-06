#include <MitkSegmentationExports.h>

#include <vtkDecimatePro.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricDecimation.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include <itkAddImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkMultiplyImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>

#include "mitkSurfaceUtils.h"
#include <mitkImageAccessByItk.h>
#include <mitkSurface.h>

namespace mitk
{

class MITKSEGMENTATION_EXPORT ShowSegmentationAsAgtkSurface
{
public:
  enum class SurfaceSmoothingType
  {
    None,
    Laplacian,
    WindowedSync,
  };

  enum class SurfaceDecimationType
  {
    None,
    DecimatePro,
    QuadricDecimation,
  };

  struct SurfaceComputingParameters
  {
    bool isBlurImage = false;
    float blurSigma = .3f;
    float levelValue = 0.f;
    SurfaceSmoothingType smoothingType = SurfaceSmoothingType::WindowedSync;
    int smoothingIterations = 15;
    float smoothingRelaxation = .1f;
    SurfaceDecimationType decimationType = SurfaceDecimationType::DecimatePro;
    float reduction = .25f;
    bool isResampling = true;
    float spacing = 1.f;
    bool isFillHoles = true;
    vtkSmartPointer<vtkCallbackCommand> vtkProgressCallback;
    vtkSmartPointer<vtkCallbackCommand> vtkEndProgressCallback;
  };

  template<typename TPixel, unsigned int VImageDimension>
  static void AccessItkAgtkSurfaceFilter(const itk::Image<TPixel, VImageDimension>* itkImage,
      SurfaceCreator::SurfaceCreationArgs surfaceArgs,
      ShowSegmentationAsAgtkSurface::SurfaceComputingParameters filterArgs,
      Surface::Pointer& output)
  {
    static const int BACKGROUND_VALUE = 0;
    static const int FOREGROUND_VALUE = 1;
    static const int PAD_SIZE = 10;
    static const double FEATURE_ANGLE = 60.;
    static const double PASS_BAND = .001;

    double sigma = filterArgs.blurSigma;

    typedef itk::Image<TPixel, VImageDimension> itkImageType;

    // Preprocessing
    itkImageType::RegionType region = itkImage->GetLargestPossibleRegion();
    itk::ImageRegionConstIteratorWithIndex<itkImageType> it(itkImage, region);
    itkImageType::IndexType minIndex = region.GetUpperIndex();
    itkImageType::IndexType maxIndex = region.GetIndex();
    size_t labeledPixels = 0;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
      if (it.Get() == FOREGROUND_VALUE) {
        labeledPixels++;
        itkImageType::IndexType index = it.GetIndex();

        for (size_t i = 0; i < VImageDimension; ++i) {
          minIndex[i] = std::min(minIndex[i], index[i]);
          maxIndex[i] = std::max(maxIndex[i], index[i]);
        }
      }
    }

    for (size_t i = 0; i < VImageDimension; ++i) {
      minIndex[i] = minIndex[i] - PAD_SIZE;
      maxIndex[i] = maxIndex[i] + PAD_SIZE;

      if (!filterArgs.isFillHoles) {
        minIndex[i] = std::max(minIndex[i], region.GetIndex()[i]);
        maxIndex[i] = std::min(maxIndex[i], region.GetUpperIndex()[i]);
      }
    }

    region.SetIndex(minIndex);
    region.SetUpperIndex(maxIndex);

    itkImageType::PointType origin;
    itkImage->TransformIndexToPhysicalPoint(region.GetIndex(), origin);

    typedef itk::NearestNeighborInterpolateImageFunction<itkImageType, double> InterpolatorType;
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

    typedef itk::Image<float, VImageDimension> FloatImage;
    typedef itk::ResampleImageFilter<itkImageType, FloatImage> ResampleImageFilterType;
    typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
    resampler->SetInput(itkImage);
    resampler->SetInterpolator(interpolator);
    resampler->SetSize(region.GetSize());
    resampler->SetOutputOrigin(origin);
    resampler->SetOutputSpacing(itkImage->GetSpacing());
    resampler->SetOutputDirection(itkImage->GetDirection());
    resampler->SetDefaultPixelValue(BACKGROUND_VALUE);
    resampler->Update();
    typename FloatImage::Pointer floatTmpImage = resampler->GetOutput();

    double levelValue = filterArgs.levelValue;
    if (!surfaceArgs.smooth) {
      levelValue = .5 * (BACKGROUND_VALUE + FOREGROUND_VALUE) - levelValue;
    } else {
      typedef itk::SignedMaurerDistanceMapImageFilter<FloatImage, FloatImage> DistanceMapType;
      typename DistanceMapType::Pointer distanceToForeground = DistanceMapType::New();
      distanceToForeground->SetInput(floatTmpImage);
      distanceToForeground->SetUseImageSpacing(true);
      distanceToForeground->SetBackgroundValue(BACKGROUND_VALUE);
      distanceToForeground->SetInsideIsPositive(false);

      typename DistanceMapType::Pointer distanceToBackground = DistanceMapType::New();
      distanceToBackground->SetInput(floatTmpImage);
      distanceToBackground->SetUseImageSpacing(true);
      distanceToBackground->SetBackgroundValue(FOREGROUND_VALUE);
      distanceToBackground->SetInsideIsPositive(true);

      typedef itk::AddImageFilter<FloatImage> AddImageFilterType;
      typename AddImageFilterType::Pointer add = AddImageFilterType::New();
      add->SetInput1(distanceToForeground->GetOutput());
      add->SetInput2(distanceToBackground->GetOutput());

      typedef itk::MultiplyImageFilter<FloatImage> MultiplyImageFilterType;
      MultiplyImageFilterType::Pointer multiply = MultiplyImageFilterType::New();
      multiply->SetInput(add->GetOutput());
      multiply->SetConstant(.5);

      typedef itk::DiscreteGaussianImageFilter<FloatImage, FloatImage> BlurImageFilterType;
      typename BlurImageFilterType::Pointer smoothingImage = BlurImageFilterType::New();
      smoothingImage->SetInput(multiply->GetOutput());
      smoothingImage->SetUseImageSpacing(false);
      smoothingImage->SetVariance(sigma * sigma);
      smoothingImage->Update();

      floatTmpImage = smoothingImage->GetOutput();
    }

    if (filterArgs.isResampling && filterArgs.spacing > 1.0e-05) {
      typedef itk::LinearInterpolateImageFunction<FloatImage, double> InterpolatorType;
      typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

      FloatImage::SizeType oldSize       = floatTmpImage->GetLargestPossibleRegion().GetSize();
      FloatImage::SpacingType oldSpacing = floatTmpImage->GetSpacing();

      FloatImage::SizeType newSize;
      FloatImage::SpacingType newSpacing;

      for (size_t n = 0; n < VImageDimension; ++n) {
        newSize[n] = (oldSize[n] - 1) * oldSpacing[n] / filterArgs.spacing + 1;
        newSpacing[n] = filterArgs.spacing;
      }

      typedef itk::ResampleImageFilter<FloatImage, FloatImage> ResampleImageFilterType;
      typename ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
      resampler->SetInput(floatTmpImage);
      resampler->SetInterpolator(interpolator);
      resampler->SetSize(newSize);
      resampler->SetOutputSpacing(newSpacing);
      resampler->SetOutputOrigin(floatTmpImage->GetOrigin());
      resampler->SetOutputDirection(floatTmpImage->GetDirection());
      resampler->Update();
      floatTmpImage = resampler->GetOutput();
    }


    typedef itk::ImageToVTKImageFilter<FloatImage> ItkToVtkConverter;
    typename ItkToVtkConverter::Pointer itkToVtk = ItkToVtkConverter::New();
    itkToVtk->SetInput(floatTmpImage);
    itkToVtk->Update();
    vtkSmartPointer<vtkImageData> vtkImage = itkToVtk->GetOutput();

    // Compute surface
    typedef vtkSmartPointer<vtkMarchingCubes> MarchingCubes;
    typename MarchingCubes mcubes = MarchingCubes::New();
    mcubes->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
    mcubes->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
    mcubes->SetInputData(vtkImage);
    mcubes->SetValue(0, levelValue);
    mcubes->SetComputeGradients(false);
    mcubes->SetComputeNormals(false);
    mcubes->SetComputeScalars(false);
    mcubes->Update();

    // Transform points
    vtkSmartPointer<vtkPolyData> polyData = mcubes->GetOutput();
    vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
    itk::Matrix<double, VImageDimension, VImageDimension> tmpDirection = floatTmpImage->GetDirection();
    itk::Vector<double, VImageDimension> tmpOrigin = floatTmpImage->GetOrigin().GetDataPointer();

    for (size_t n = 0; n < points->GetNumberOfPoints(); ++n) {
      itk::Vector<double, VImageDimension> point = points->GetPoint(n);
      point = tmpDirection * (point - tmpOrigin) + tmpOrigin;
      points->SetPoint(n, point.GetDataPointer());
    }

    // Decimation/*
    switch (filterArgs.decimationType) {
      case SurfaceDecimationType::DecimatePro: {
        typedef vtkSmartPointer<vtkDecimatePro> Decimate;
        typename Decimate decimate = Decimate::New();
        decimate->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
        decimate->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
        decimate->SetInputData(polyData);
        decimate->SetSplitting(false);
        decimate->SetErrorIsAbsolute(5);
        decimate->SetFeatureAngle(FEATURE_ANGLE);
        decimate->SetPreserveTopology(true);
        decimate->SetBoundaryVertexDeletion(false);
        decimate->SetDegree(10); // Standard value is 25
        decimate->SetTargetReduction(filterArgs.reduction);
        decimate->SetMaximumError(.002);
        decimate->Update();
        polyData = decimate->GetOutput();
        break;
      }
      case SurfaceDecimationType::QuadricDecimation: {
        typedef vtkSmartPointer<vtkQuadricDecimation> Decimation;
        typename Decimation decimate = Decimation::New();
        decimate->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
        decimate->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
        decimate->SetInputData(polyData);
        decimate->SetTargetReduction(filterArgs.reduction);
        decimate->Update();
        polyData = decimate->GetOutput();
        break;
      }
    }

    switch (filterArgs.smoothingType) {
      case SurfaceSmoothingType::Laplacian: {
        typedef vtkSmartPointer<vtkSmoothPolyDataFilter> SmoothPolyData;
        typename SmoothPolyData smooth = SmoothPolyData::New();
        smooth->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
        smooth->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
        smooth->SetInputData(polyData);
        smooth->SetNumberOfIterations(filterArgs.smoothingIterations);
        smooth->SetRelaxationFactor(filterArgs.smoothingRelaxation);
        smooth->SetFeatureAngle(FEATURE_ANGLE);
        smooth->SetConvergence(0);
        smooth->SetBoundarySmoothing(false);
        smooth->SetFeatureEdgeSmoothing(false);
        smooth->Update();
        polyData = smooth->GetOutput();
        break;
      }
      case SurfaceSmoothingType::WindowedSync: {
        typedef vtkSmartPointer<vtkWindowedSincPolyDataFilter> SmoothPolyData;
        SmoothPolyData smooth = SmoothPolyData::New();
        smooth->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
        smooth->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
        smooth->SetInputData(polyData);
        smooth->SetNumberOfIterations(filterArgs.smoothingIterations);
        smooth->SetFeatureEdgeSmoothing(false);
        smooth->SetFeatureAngle(FEATURE_ANGLE);
        smooth->SetPassBand(PASS_BAND);
        smooth->SetNonManifoldSmoothing(true);
        smooth->SetNormalizeCoordinates(true);
        smooth->SetBoundarySmoothing(false);
        smooth->Update();
        polyData = smooth->GetOutput();
        break;
      }
    }

    // Compute normals
    typedef vtkSmartPointer<vtkPolyDataNormals> PolyDataNormals;
    typename PolyDataNormals normals = PolyDataNormals::New();
    normals->AddObserver(vtkCommand::ProgressEvent, filterArgs.vtkProgressCallback);
    normals->AddObserver(vtkCommand::EndEvent, filterArgs.vtkEndProgressCallback);
    normals->SetInputData(polyData);
    normals->AutoOrientNormalsOn();
    normals->FlipNormalsOff();
    normals->ConsistencyOn();
    normals->SplittingOff();
    normals->ComputeCellNormalsOff();
    normals->ComputePointNormalsOn();
    normals->Update();
    polyData = normals->GetOutput();

    output = mitk::Surface::New();
    output->SetVtkPolyData(polyData);
  }
};

}

