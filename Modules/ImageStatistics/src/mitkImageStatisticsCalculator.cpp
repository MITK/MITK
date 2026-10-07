/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageStatisticsCalculator.h>
#include <mitkLabelStatisticsImageFilter.h>
#include <mitkStatisticsImageFilter.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageStatisticsConstants.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToItk.h>
#include <mitkMinMaxImageFilterWithIndex.h>
#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <mitkNodePredicateGeometry.h>

#include <itkImageRegionConstIterator.h>
#include <itkRegionOfInterestImageFilter.h>

#include <map>

namespace
{
  /** The statistics filters read one scalar value per voxel. */
  bool HasScalarPixelValues(const mitk::Image* image)
  {
    const auto& pixelType = image->GetPixelType();
    return pixelType.GetPixelType() == itk::IOPixelEnum::SCALAR && pixelType.GetNumberOfComponents() == 1;
  }

  double ComputeVoxelVolume(const mitk::Image* image)
  {
    const auto spacing = image->GetGeometry()->GetSpacing();
    double voxelVolume = 1.;

    for (unsigned int i = 0; i < image->GetDimension(); ++i)
      voxelVolume *= spacing[i];

    return voxelVolume;
  }
}

namespace mitk
{
  void ImageStatisticsCalculator::SetInputImage(const mitk::Image *image)
  {
    if (image != m_Image)
    {
      m_Image = image;
      this->Modified();
    }
  }

  void ImageStatisticsCalculator::SetMask(mitk::MaskGenerator *mask)
  {
    if (mask != m_MaskGenerator)
    {
      m_MaskGenerator = mask;
      this->Modified();
    }
  }

  void ImageStatisticsCalculator::SetNBinsForHistogramStatistics(unsigned int nBins)
  {
    if (nBins != m_nBinsForHistogramStatistics)
    {
      m_nBinsForHistogramStatistics = nBins;
      this->Modified();
      this->m_UseBinSizeOverNBins = false;
    }
    if (m_UseBinSizeOverNBins)
    {
      this->Modified();
      this->m_UseBinSizeOverNBins = false;
    }
  }

  unsigned int ImageStatisticsCalculator::GetNBinsForHistogramStatistics() const
  {
    return m_nBinsForHistogramStatistics;
  }

  void ImageStatisticsCalculator::SetBinSizeForHistogramStatistics(double binSize)
  {
    if (binSize != m_binSizeForHistogramStatistics)
    {
      m_binSizeForHistogramStatistics = binSize;
      this->Modified();
      this->m_UseBinSizeOverNBins = true;
    }
    if (!m_UseBinSizeOverNBins)
    {
      this->Modified();
      this->m_UseBinSizeOverNBins = true;
    }
  }

  double ImageStatisticsCalculator::GetBinSizeForHistogramStatistics() const { return m_binSizeForHistogramStatistics; }

  mitk::ImageStatisticsContainer* ImageStatisticsCalculator::GetStatistics()
  {
    if (m_Image.IsNull())
    {
      mitkThrow() << "no image";
    }

    if (!m_Image->IsInitialized())
    {
      mitkThrow() << "Image not initialized!";
    }

    if (IsUpdateRequired())
    {
      auto timeGeometry = m_Image->GetTimeGeometry();
      m_StatisticContainer = ImageStatisticsContainer::New();
      m_StatisticContainer->SetTimeGeometry(timeGeometry->Clone());

      // always compute statistics on all timesteps
      for (TimeStepType timeStep = 0; timeStep < m_Image->GetTimeSteps(); timeStep++)
      {
        const unsigned int numbersOfMasks = m_MaskGenerator.IsNotNull() ? m_MaskGenerator->GetNumberOfMasks() : 1;
        auto timePoint = timeGeometry->TimeStepToTimePoint(timeStep);

        for (unsigned int maskID = 0; maskID < numbersOfMasks; ++maskID)
        {
          // A mask generator may compute its mask on a reference image of its own
          // (e.g. the slice of a planar figure); the statistics then run on that image.
          Image::ConstPointer imageForStatistics = m_Image;
          m_InternalMask = nullptr;

          if (m_MaskGenerator.IsNotNull())
          {
            m_MaskGenerator->SetTimePoint(timePoint);
            m_InternalMask = m_MaskGenerator->GetMask(maskID);

            const auto referenceImage = m_MaskGenerator->GetReferenceImage();

            if (referenceImage.IsNotNull())
              imageForStatistics = referenceImage;
          }

          m_ImageTimeSlice = SelectImageByTimeStep(imageForStatistics, timeStep);

          // verbose makes IsSubGeometry log the check that failed; it stays silent on success
          if (m_InternalMask.IsNotNull() && !IsSubGeometry(*m_InternalMask->GetGeometry(), *m_ImageTimeSlice->GetGeometry(),
                                                           NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
                                                           NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION,
                                                           true))
          {
            mitkThrow() << "Mask is not a sub geometry of the image (see the log for the failed check): "
                           "it has to lie on the voxel grid of the image and within its extent.";
          }

          // Decided on the input image: the slice a planar figure provides as reference image
          // keeps only the first component of a multi-component image.
          if (!HasScalarPixelValues(m_Image))
          {
            // Only the statistics that do not read pixel values, e.g. for RGB images
            if (m_MaskGenerator.IsNull())
            {
              this->CalculateVoxelCountStatisticsUnmasked(timeStep);
            }
            else
            {
              AccessByItk_1(m_InternalMask, InternalCalculateVoxelCountStatisticsMasked, timeStep)
            }
          }
          else if (m_MaskGenerator.IsNull())
          {
            // 1) calculate statistics unmasked:
            AccessByItk_1(m_ImageTimeSlice, InternalCalculateStatisticsUnmasked, timeStep)
          }
          else
          {
            // 2) calculate statistics masked
            AccessByItk_1(m_ImageTimeSlice, InternalCalculateStatisticsMasked, timeStep)
          }
        }
      }
    }

    return m_StatisticContainer;
  }

  template <typename TPixel, unsigned int VImageDimension>
  void ImageStatisticsCalculator::InternalCalculateStatisticsUnmasked(
    const itk::Image<TPixel, VImageDimension> *image, TimeStepType timeStep)
  {
    typedef typename itk::Image<TPixel, VImageDimension> ImageType;
    typedef typename mitk::StatisticsImageFilter<ImageType> ImageStatisticsFilterType;
    typedef typename itk::MinMaxImageFilterWithIndex<ImageType> MinMaxFilterType;

    auto statObj = ImageStatisticsContainer::ImageStatisticsObject();

    typename ImageStatisticsFilterType::Pointer statisticsFilter = ImageStatisticsFilterType::New();
    statisticsFilter->SetInput(image);
    statisticsFilter->SetCoordinateTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION);
    statisticsFilter->SetDirectionTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION);

    // TODO: this is single threaded. Implement our own image filter that does this multi threaded
    //        typename itk::MinimumMaximumImageCalculator<ImageType>::Pointer imgMinMaxFilter =
    //        itk::MinimumMaximumImageCalculator<ImageType>::New(); imgMinMaxFilter->SetImage(image);
    //        imgMinMaxFilter->Compute();
    vnl_vector<int> minIndex, maxIndex;

    typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
    minMaxFilter->SetInput(image);
    minMaxFilter->UpdateLargestPossibleRegion();
    typename ImageType::PixelType minval = minMaxFilter->GetMin();
    typename ImageType::PixelType maxval = minMaxFilter->GetMax();

    typename ImageType::IndexType tmpMinIndex = minMaxFilter->GetMinIndex();
    typename ImageType::IndexType tmpMaxIndex = minMaxFilter->GetMaxIndex();

    //        typename ImageType::IndexType tmpMinIndex = imgMinMaxFilter->GetIndexOfMinimum();
    //        typename ImageType::IndexType tmpMaxIndex = imgMinMaxFilter->GetIndexOfMaximum();

    minIndex.set_size(tmpMaxIndex.GetIndexDimension());
    maxIndex.set_size(tmpMaxIndex.GetIndexDimension());

    for (unsigned int i = 0; i < tmpMaxIndex.GetIndexDimension(); i++)
    {
      minIndex[i] = tmpMinIndex[i];
      maxIndex[i] = tmpMaxIndex[i];
    }

    statObj.AddStatistic(mitk::ImageStatisticsConstants::MINIMUMPOSITION(), minIndex);
    statObj.AddStatistic(mitk::ImageStatisticsConstants::MAXIMUMPOSITION(), maxIndex);

    // convert m_binSize in m_nBins if necessary
    unsigned int nBinsForHistogram;
    if (m_UseBinSizeOverNBins)
    {
      nBinsForHistogram = std::max(static_cast<double>(std::ceil(maxval - minval)) / m_binSizeForHistogramStatistics,
                                   10.); // do not allow less than 10 bins
    }
    else
    {
      nBinsForHistogram = m_nBinsForHistogramStatistics;
    }

    statisticsFilter->SetHistogramParameters(nBinsForHistogram, minval, maxval);

    try
    {
      statisticsFilter->Update();
    }
    catch (const itk::ExceptionObject &e)
    {
      mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.GetDescription();
    }

    auto voxelVolume = ComputeVoxelVolume(m_ImageTimeSlice);

    auto numberOfPixels = image->GetLargestPossibleRegion().GetNumberOfPixels();
    auto volume = static_cast<double>(numberOfPixels) * voxelVolume;
    auto variance = statisticsFilter->GetSigma() * statisticsFilter->GetSigma();
    auto rms =
      std::sqrt(std::pow(statisticsFilter->GetMean(), 2.) + statisticsFilter->GetVariance()); // variance = sigma^2

    statObj.AddStatistic(ImageStatisticsConstants::NUMBEROFVOXELS(),
                         static_cast<ImageStatisticsContainer::VoxelCountType>(numberOfPixels));
    statObj.AddStatistic(ImageStatisticsConstants::VOLUME(), volume);
    statObj.AddStatistic(ImageStatisticsConstants::MEAN(), statisticsFilter->GetMean());
    statObj.AddStatistic(ImageStatisticsConstants::MINIMUM(),
                         static_cast<ImageStatisticsContainer::RealType>(statisticsFilter->GetMinimum()));
    statObj.AddStatistic(ImageStatisticsConstants::MAXIMUM(),
                         static_cast<ImageStatisticsContainer::RealType>(statisticsFilter->GetMaximum()));
    statObj.AddStatistic(ImageStatisticsConstants::STANDARDDEVIATION(), statisticsFilter->GetSigma());
    statObj.AddStatistic(ImageStatisticsConstants::VARIANCE(), variance);
    statObj.AddStatistic(ImageStatisticsConstants::SKEWNESS(), statisticsFilter->GetSkewness());
    statObj.AddStatistic(ImageStatisticsConstants::KURTOSIS(), statisticsFilter->GetKurtosis());
    statObj.AddStatistic(ImageStatisticsConstants::RMS(), rms);
    statObj.AddStatistic(ImageStatisticsConstants::MPP(), statisticsFilter->GetMPP());
    statObj.AddStatistic(ImageStatisticsConstants::ENTROPY(), statisticsFilter->GetEntropy());
    statObj.AddStatistic(ImageStatisticsConstants::MEDIAN(), statisticsFilter->GetMedian());
    statObj.AddStatistic(ImageStatisticsConstants::UNIFORMITY(), statisticsFilter->GetUniformity());
    statObj.AddStatistic(ImageStatisticsConstants::UPP(), statisticsFilter->GetUPP());
    statObj.m_Histogram = statisticsFilter->GetHistogram();

    m_StatisticContainer->SetStatistics(ImageStatisticsContainer::NO_MASK_LABEL_VALUE, timeStep, statObj);
  }

  template <typename TPixel, unsigned int VImageDimension>
  void ImageStatisticsCalculator::InternalCalculateStatisticsMasked(const itk::Image<TPixel, VImageDimension> *image,
    TimeStepType timeStep)
  {
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<MaskPixelType, VImageDimension> MaskType;
    typedef typename MaskType::PixelType LabelPixelType;
    typedef LabelStatisticsImageFilter<ImageType> ImageStatisticsFilterType;
    typedef typename itk::MinMaxLabelImageFilterWithIndex<ImageType, MaskType> MinMaxLabelFilterType;
    typedef itk::RegionOfInterestImageFilter<ImageType, ImageType> RegionOfInterestFilterType;

    const BaseGeometry* referenceGeometry = m_ImageTimeSlice->GetGeometry();
    const BaseGeometry* maskGeometry = m_InternalMask->GetGeometry();

    // A mask may cover only a sub-region of the image. The filters then run on
    // that region and report indices relative to it.
    itk::Index<3> maskOffset;
    referenceGeometry->WorldToIndex(maskGeometry->GetOrigin(), maskOffset);

    // Maps an index of the (possibly cropped) statistics image to an index of the
    // input image. The reference image may be a 2D slice (planar figure masks), so
    // the index has to go through world coordinates. On rotated geometries the
    // inverse transform carries floating-point noise, so the result is rounded
    // rather than truncated.
    const BaseGeometry* imageGeometry = m_Image->GetGeometry(timeStep);
    auto toImageIndex = [&](const typename ImageType::IndexType& index)
    {
      itk::Index<3> referenceIndex;
      referenceIndex.Fill(0);

      for (unsigned int i = 0; i < VImageDimension; ++i)
        referenceIndex[i] = index[i] + maskOffset[i];

      Point3D world;
      referenceGeometry->IndexToWorld(referenceIndex, world);

      itk::Index<3> imageIndex;
      imageGeometry->WorldToIndex(world, imageIndex);

      vnl_vector<int> result(3);

      for (unsigned int i = 0; i < 3; ++i)
        result[i] = static_cast<int>(imageIndex[i]);

      return result;
    };

    // maskImage has to have the same dimension as image
    typename MaskType::ConstPointer maskImage = MaskType::New();
    try
    {
      // try to access the pixel values directly (no copying or casting). Only works if mask pixels are of pixelType
      // unsigned short
      maskImage = ImageToItkImage<MaskPixelType, VImageDimension>(m_InternalMask);
    }
    catch (const itk::ExceptionObject &)
    {
      typename MaskType::Pointer noneConstMaskImage; //needed to work around the fact that CastToItkImage currently does not support const itk images.
      // if the pixel type of the mask is not short, then we have to make a copy of m_InternalMask (and cast the values)
      CastToItkImage(m_InternalMask, noneConstMaskImage);
      maskImage = noneConstMaskImage;
    }

    typename ImageType::ConstPointer adaptedImage = image;

    if (maskImage->GetLargestPossibleRegion().GetSize() != image->GetLargestPossibleRegion().GetSize())
    {
      typename ImageType::RegionType maskRegion = maskImage->GetLargestPossibleRegion();
      typename ImageType::IndexType regionIndex;

      for (unsigned int i = 0; i < VImageDimension; ++i)
        regionIndex[i] = maskOffset[i];

      maskRegion.SetIndex(regionIndex);

      auto regionOfInterestFilter = RegionOfInterestFilterType::New();
      regionOfInterestFilter->SetInput(image);
      regionOfInterestFilter->SetRegionOfInterest(maskRegion);
      regionOfInterestFilter->Update();
      adaptedImage = regionOfInterestFilter->GetOutput();
    }

    // find min, max, minindex and maxindex
    typename MinMaxLabelFilterType::Pointer minMaxFilter = MinMaxLabelFilterType::New();
    minMaxFilter->SetInput(adaptedImage);
    minMaxFilter->SetLabelInput(maskImage);
    minMaxFilter->SetCoordinateTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION);
    minMaxFilter->SetDirectionTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION);
    minMaxFilter->UpdateLargestPossibleRegion();

    // set histogram parameters for each label individually (min/max may be different for each label)
    typedef typename std::unordered_map<LabelPixelType, ScalarType> MapType;

    std::vector<LabelPixelType> relevantLabels = minMaxFilter->GetRelevantLabels();
    MapType minVals;
    MapType maxVals;
    std::unordered_map<LabelPixelType, unsigned int> nBins;

    for (LabelPixelType label : relevantLabels)
    {
      // The background is discarded below, so asking for its histogram would
      // only pay for a median accumulator over every voxel outside the mask.
      if (static_cast<LabelPixelType>(ImageStatisticsContainer::NO_MASK_LABEL_VALUE) == label)
        continue;

      minVals[label] = static_cast<ScalarType>(minMaxFilter->GetMin(label));
      maxVals[label] = static_cast<ScalarType>(minMaxFilter->GetMax(label));

      unsigned int nBinsForHistogram;
      if (m_UseBinSizeOverNBins)
      {
        nBinsForHistogram =
          std::max(static_cast<double>(std::ceil(minMaxFilter->GetMax(label) - minMaxFilter->GetMin(label))) /
                     m_binSizeForHistogramStatistics,
                   10.); // do not allow less than 10 bins
      }
      else
      {
        nBinsForHistogram = m_nBinsForHistogramStatistics;
      }

      nBins[label] = nBinsForHistogram;
    }

    typename ImageStatisticsFilterType::Pointer imageStatisticsFilter = ImageStatisticsFilterType::New();
    imageStatisticsFilter->SetCoordinateTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION);
    imageStatisticsFilter->SetDirectionTolerance(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION);
    imageStatisticsFilter->SetInput(adaptedImage);
    imageStatisticsFilter->SetLabelInput(maskImage);
    imageStatisticsFilter->SetHistogramParameters(nBins, minVals, maxVals);
    imageStatisticsFilter->Update();

    const auto labels = imageStatisticsFilter->GetValidLabelValues();

    for (auto labelValue : labels)
    {
      if (labelValue == ImageStatisticsContainer::NO_MASK_LABEL_VALUE)
      {
        //we ignore the background of a mask if we compute mask statistics.
        continue;
      }

      ImageStatisticsContainer::ImageStatisticsObject statObj;

      statObj.AddStatistic(ImageStatisticsConstants::MINIMUMPOSITION(), toImageIndex(minMaxFilter->GetMinIndex(labelValue)));
      statObj.AddStatistic(ImageStatisticsConstants::MAXIMUMPOSITION(), toImageIndex(minMaxFilter->GetMaxIndex(labelValue)));

      auto voxelVolume = ComputeVoxelVolume(m_ImageTimeSlice);
      auto numberOfVoxels =
        static_cast<unsigned long>(imageStatisticsFilter->GetCount(labelValue));
      auto volume = static_cast<double>(numberOfVoxels) * voxelVolume;
      auto rms = std::sqrt(std::pow(imageStatisticsFilter->GetMean(labelValue), 2.) +
                           imageStatisticsFilter->GetVariance(labelValue)); // variance = sigma^2
      auto variance = imageStatisticsFilter->GetSigma(labelValue) * imageStatisticsFilter->GetSigma(labelValue);

      statObj.AddStatistic(ImageStatisticsConstants::NUMBEROFVOXELS(), numberOfVoxels);
      statObj.AddStatistic(ImageStatisticsConstants::VOLUME(), volume);
      statObj.AddStatistic(ImageStatisticsConstants::MEAN(), imageStatisticsFilter->GetMean(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::MINIMUM(),
        static_cast<ImageStatisticsContainer::RealType>(imageStatisticsFilter->GetMinimum(labelValue)));
      statObj.AddStatistic(ImageStatisticsConstants::MAXIMUM(),
        static_cast<ImageStatisticsContainer::RealType>(imageStatisticsFilter->GetMaximum(labelValue)));
      statObj.AddStatistic(ImageStatisticsConstants::STANDARDDEVIATION(), imageStatisticsFilter->GetSigma(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::VARIANCE(), variance);
      statObj.AddStatistic(ImageStatisticsConstants::SKEWNESS(), imageStatisticsFilter->GetSkewness(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::KURTOSIS(), imageStatisticsFilter->GetKurtosis(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::RMS(), rms);
      statObj.AddStatistic(ImageStatisticsConstants::MPP(), imageStatisticsFilter->GetMPP(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::ENTROPY(), imageStatisticsFilter->GetEntropy(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::MEDIAN(), imageStatisticsFilter->GetMedian(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::UNIFORMITY(), imageStatisticsFilter->GetUniformity(labelValue));
      statObj.AddStatistic(ImageStatisticsConstants::UPP(), imageStatisticsFilter->GetUPP(labelValue));
      statObj.m_Histogram = imageStatisticsFilter->GetHistogram(labelValue);

      if (m_StatisticContainer->StatisticsExist(labelValue, timeStep))
        mitkThrow() << "Invalid state/input data. Statistic for a specific label/time step pair was computed more then once. Conflicting label ID: "
        << labelValue << " ; conflicting time step: " << timeStep;
      m_StatisticContainer->SetStatistics(labelValue, timeStep, statObj);
    }
  }

  void ImageStatisticsCalculator::CalculateVoxelCountStatisticsUnmasked(TimeStepType timeStep)
  {
    ImageStatisticsContainer::VoxelCountType numberOfVoxels = 1;

    for (unsigned int i = 0; i < m_ImageTimeSlice->GetDimension(); ++i)
      numberOfVoxels *= m_ImageTimeSlice->GetDimension(i);

    ImageStatisticsContainer::ImageStatisticsObject statObj;
    statObj.AddStatistic(ImageStatisticsConstants::NUMBEROFVOXELS(), numberOfVoxels);
    statObj.AddStatistic(ImageStatisticsConstants::VOLUME(), static_cast<double>(numberOfVoxels) * ComputeVoxelVolume(m_ImageTimeSlice));

    m_StatisticContainer->SetStatistics(ImageStatisticsContainer::NO_MASK_LABEL_VALUE, timeStep, statObj);
  }

  template <typename TPixel, unsigned int VImageDimension>
  void ImageStatisticsCalculator::InternalCalculateVoxelCountStatisticsMasked(
    const itk::Image<TPixel, VImageDimension> *mask, TimeStepType timeStep)
  {
    std::map<LabelIndex, ImageStatisticsContainer::VoxelCountType> voxelCounts;

    for (itk::ImageRegionConstIterator<itk::Image<TPixel, VImageDimension>> it(mask, mask->GetLargestPossibleRegion()); !it.IsAtEnd(); ++it)
      ++voxelCounts[static_cast<LabelIndex>(it.Get())];

    // The voxel volume of the image, as in the masked statistics of scalar images: the mask
    // of a 2D image is a 3D image with a single slice.
    const auto voxelVolume = ComputeVoxelVolume(m_ImageTimeSlice);

    for (const auto& [labelValue, numberOfVoxels] : voxelCounts)
    {
      if (labelValue == ImageStatisticsContainer::NO_MASK_LABEL_VALUE)
        continue;

      ImageStatisticsContainer::ImageStatisticsObject statObj;
      statObj.AddStatistic(ImageStatisticsConstants::NUMBEROFVOXELS(), numberOfVoxels);
      statObj.AddStatistic(ImageStatisticsConstants::VOLUME(), static_cast<double>(numberOfVoxels) * voxelVolume);

      if (m_StatisticContainer->StatisticsExist(labelValue, timeStep))
        mitkThrow() << "Invalid state/input data. Statistic for a specific label/time step pair was computed more then once. Conflicting label ID: "
        << labelValue << " ; conflicting time step: " << timeStep;
      m_StatisticContainer->SetStatistics(labelValue, timeStep, statObj);
    }
  }

  bool ImageStatisticsCalculator::IsUpdateRequired() const
  {
    const auto thisClassTimeStamp = this->GetMTime();
    const auto inputImageTimeStamp = m_Image->GetMTime();

    if (m_StatisticContainer.IsNull())
    {
      return true;
    }

    const auto statisticsTimeStamp = m_StatisticContainer->GetMTime();

    if (thisClassTimeStamp > statisticsTimeStamp) // inputs have changed
    {
      return true;
    }

    if (inputImageTimeStamp > statisticsTimeStamp) // image has changed
    {
      return true;
    }

    if (m_MaskGenerator.IsNotNull())
    {
      const auto maskGeneratorTimeStamp = m_MaskGenerator->GetMTime();
      if (maskGeneratorTimeStamp > statisticsTimeStamp) // there is a mask generator and it has changed
      {
        return true;
      }
    }

    return false;
  }
} // namespace mitk
