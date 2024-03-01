/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageStatisticsCalculator.h"
#include <mitkLabelStatisticsImageFilter.h>
#include <mitkStatisticsImageFilter.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageStatisticsConstants.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToItk.h>
#include <mitkMaskUtilities.h>
#include <mitkMinMaxImageFilterWithIndex.h>
#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <mitkitkMaskImageFilter.h>

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

  void ImageStatisticsCalculator::SetSecondaryMask(mitk::MaskGenerator *mask)
  {
    if (mask != m_SecondaryMaskGenerator)
    {
      m_SecondaryMaskGenerator = mask;
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
      for (unsigned int timeStep = 0; timeStep < m_Image->GetTimeSteps(); timeStep++)
      {
        if (m_MaskGenerator.IsNotNull())
        {
          m_MaskGenerator->SetTimeStep(timeStep);
          m_InternalMask = m_MaskGenerator->GetMask();
          if (m_MaskGenerator->GetReferenceImage().IsNotNull())
          {
            m_InternalImageForStatistics = m_MaskGenerator->GetReferenceImage();
          }
          else
          {
            m_InternalImageForStatistics = m_Image;
          }
        }
        else
        {
          m_InternalImageForStatistics = m_Image;
        }

        if (m_SecondaryMaskGenerator.IsNotNull())
        {
          m_SecondaryMaskGenerator->SetTimeStep(timeStep);
          m_SecondaryMask = m_SecondaryMaskGenerator->GetMask();
        }

        ImageTimeSelector::Pointer imgTimeSel = ImageTimeSelector::New();
        imgTimeSel->SetInput(m_InternalImageForStatistics);
        imgTimeSel->SetTimeNr(timeStep);
        imgTimeSel->UpdateLargestPossibleRegion();
        imgTimeSel->Update();
        m_ImageTimeSlice = imgTimeSel->GetOutput();

        // Calculate statistics with/without mask
        if (m_MaskGenerator.IsNull() && m_SecondaryMaskGenerator.IsNull())
        {
          // 1) calculate statistics unmasked:
          AccessByItk_2(m_ImageTimeSlice, InternalCalculateStatisticsUnmasked, timeGeometry, timeStep)
        }
        else
        {
          // 2) calculate statistics masked
          AccessByItk_2(m_ImageTimeSlice, InternalCalculateStatisticsMasked, timeGeometry, timeStep)
        }
      }
    }

    return m_StatisticContainer;
  }

  template <typename TPixel, unsigned int VImageDimension>
  void ImageStatisticsCalculator::InternalCalculateStatisticsUnmasked(
    typename itk::Image<TPixel, VImageDimension> *image, const TimeGeometry *timeGeometry, TimeStepType timeStep)
  {
    typedef typename itk::Image<TPixel, VImageDimension> ImageType;
    typedef typename mitk::StatisticsImageFilter<ImageType> ImageStatisticsFilterType;
    typedef typename itk::MinMaxImageFilterWithIndex<ImageType> MinMaxFilterType;

    auto statObj = ImageStatisticsContainer::ImageStatisticsObject();

    typename ImageStatisticsFilterType::Pointer statisticsFilter = ImageStatisticsFilterType::New();
    statisticsFilter->SetInput(image);
    statisticsFilter->SetCoordinateTolerance(0.001);
    statisticsFilter->SetDirectionTolerance(0.001);

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
      mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }

    auto voxelVolume = GetVoxelVolume<TPixel, VImageDimension>(image);

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
  double ImageStatisticsCalculator::GetVoxelVolume(typename itk::Image<TPixel, VImageDimension> *image) const
  {
    auto spacing = image->GetSpacing();
    double voxelVolume = 1.;
    for (unsigned int i = 0; i < image->GetImageDimension(); i++)
    {
      voxelVolume *= spacing[i];
    }
    return voxelVolume;
  }

  template <typename TPixel, unsigned int VImageDimension>
  void ImageStatisticsCalculator::InternalCalculateStatisticsMasked(typename itk::Image<TPixel, VImageDimension> *image,
                                                                    const TimeGeometry *timeGeometry,
                                                                    unsigned int timeStep)
  {
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef itk::Image<MaskPixelType, VImageDimension> MaskType;
    typedef typename MaskType::PixelType LabelPixelType;
    typedef LabelStatisticsImageFilter<ImageType> ImageStatisticsFilterType;
    typedef MaskUtilities<TPixel, VImageDimension> MaskUtilType;
    typedef typename itk::MinMaxLabelImageFilterWithIndex<ImageType, MaskType> MinMaxLabelFilterType;

    // workaround: if m_SecondaryMaskGenerator ist not null but m_MaskGenerator is! (this is the case if we request a
    // 'ignore zuero valued pixels' mask in the gui but do not define a primary mask)
    bool swapMasks = false;
    if (m_SecondaryMask.IsNotNull() && m_InternalMask.IsNull())
    {
      m_InternalMask = m_SecondaryMask;
      m_SecondaryMask = nullptr;
      swapMasks = true;
    }

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
      typename MaskType::Pointer noneConstMaskImage; //needed to work arround the fact that CastToItkImage currently does not support const itk images.
      // if the pixel type of the mask is not short, then we have to make a copy of m_InternalMask (and cast the values)
      CastToItkImage(m_InternalMask, noneConstMaskImage);
      maskImage = noneConstMaskImage;
    }

    // if we have a secondary mask (say a ignoreZeroPixelMask) we need to combine the masks (corresponds to AND)
    if (m_SecondaryMask.IsNotNull())
    {
      // dirty workaround for a bug when pf mask + any other mask is used in conjunction. We need a proper fix for this
      // (Fabian Isensee is responsible and probably working on it!)
      if (m_InternalMask->GetDimension() == 2 &&
          (m_SecondaryMask->GetDimension() == 3 || m_SecondaryMask->GetDimension() == 4))
      {
        Image::ConstPointer old_img = m_SecondaryMaskGenerator->GetReferenceImage();
        m_SecondaryMaskGenerator->SetInputImage(m_MaskGenerator->GetReferenceImage());
        m_SecondaryMask = m_SecondaryMaskGenerator->GetMask();
        m_SecondaryMaskGenerator->SetInputImage(old_img);
      }
      typename MaskType::ConstPointer secondaryMaskImage = MaskType::New();
      secondaryMaskImage = ImageToItkImage<MaskPixelType, VImageDimension>(m_SecondaryMask);

      // secondary mask should be a ignore zero value pixel mask derived from image. it has to be cropped to the mask
      // region (which may be planar or simply smaller)
      typename MaskUtilities<MaskPixelType, VImageDimension>::Pointer secondaryMaskMaskUtil =
        MaskUtilities<MaskPixelType, VImageDimension>::New();
      secondaryMaskMaskUtil->SetImage(secondaryMaskImage.GetPointer());
      secondaryMaskMaskUtil->SetMask(maskImage.GetPointer());
      typename MaskType::ConstPointer adaptedSecondaryMaskImage = secondaryMaskMaskUtil->ExtractMaskImageRegion();

      typename itk::MaskImageFilter2<MaskType, MaskType, MaskType>::Pointer maskFilter =
        itk::MaskImageFilter2<MaskType, MaskType, MaskType>::New();
      maskFilter->SetInput1(maskImage);
      maskFilter->SetInput2(adaptedSecondaryMaskImage);
      maskFilter->SetMaskingValue(
        1); // all pixels of maskImage where secondaryMaskImage==1 will be kept, all the others are set to 0
      maskFilter->UpdateLargestPossibleRegion();
      maskImage = maskFilter->GetOutput();
    }

    typename MaskUtilType::Pointer maskUtil = MaskUtilType::New();
    maskUtil->SetImage(image);
    maskUtil->SetMask(maskImage.GetPointer());

    // if mask is smaller than image, extract the image region where the mask is
    typename ImageType::ConstPointer adaptedImage = ImageType::New();

    adaptedImage = maskUtil->ExtractMaskImageRegion(); // this also checks mask sanity

    // find min, max, minindex and maxindex
    typename MinMaxLabelFilterType::Pointer minMaxFilter = MinMaxLabelFilterType::New();
    minMaxFilter->SetInput(adaptedImage);
    minMaxFilter->SetLabelInput(maskImage);
    minMaxFilter->UpdateLargestPossibleRegion();

    // set histogram parameters for each label individually (min/max may be different for each label)
    typedef typename std::unordered_map<LabelPixelType, ScalarType> MapType;

    std::vector<LabelPixelType> relevantLabels = minMaxFilter->GetRelevantLabels();
    MapType minVals;
    MapType maxVals;
    std::unordered_map<LabelPixelType, unsigned int> nBins;

    for (LabelPixelType label : relevantLabels)
    {
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
    imageStatisticsFilter->SetDirectionTolerance(0.001);
    imageStatisticsFilter->SetCoordinateTolerance(0.001);
    imageStatisticsFilter->SetInput(adaptedImage);
    imageStatisticsFilter->SetLabelInput(maskImage);
    imageStatisticsFilter->SetHistogramParameters(nBins, minVals, maxVals);
    imageStatisticsFilter->Update();

    const auto labels = imageStatisticsFilter->GetValidLabelValues();

    for (auto labelValue : labels)
    {
      ImageStatisticsContainer::ImageStatisticsObject statObj;

      // find min, max, minindex and maxindex
      // make sure to only look in the masked region, use a masker for this

      vnl_vector<int> minIndex, maxIndex;
      Point3D worldCoordinateMin;
      Point3D worldCoordinateMax;
      Point3D indexCoordinateMin;
      Point3D indexCoordinateMax;
      m_InternalImageForStatistics->GetGeometry()->IndexToWorld(minMaxFilter->GetMinIndex(labelValue), worldCoordinateMin);
      m_InternalImageForStatistics->GetGeometry()->IndexToWorld(minMaxFilter->GetMaxIndex(labelValue), worldCoordinateMax);
      m_Image->GetGeometry()->WorldToIndex(worldCoordinateMin, indexCoordinateMin);
      m_Image->GetGeometry()->WorldToIndex(worldCoordinateMax, indexCoordinateMax);

      minIndex.set_size(3);
      maxIndex.set_size(3);

      // for (unsigned int i=0; i < tmpMaxIndex.GetIndexDimension(); i++)
      for (unsigned int i = 0; i < 3; i++)
      {
        minIndex[i] = indexCoordinateMin[i];
        maxIndex[i] = indexCoordinateMax[i];
      }

      statObj.AddStatistic(ImageStatisticsConstants::MINIMUMPOSITION(), minIndex);
      statObj.AddStatistic(ImageStatisticsConstants::MAXIMUMPOSITION(), maxIndex);

      auto voxelVolume = GetVoxelVolume<TPixel, VImageDimension>(image);
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

      m_StatisticContainer->SetStatistics(labelValue, timeStep, statObj);
    }

    // swap maskGenerators back
    if (swapMasks)
    {
      m_SecondaryMask = m_InternalMask;
      m_InternalMask = nullptr;
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

    if (m_SecondaryMaskGenerator.IsNotNull())
    {
      const auto maskGeneratorTimeStamp = m_SecondaryMaskGenerator->GetMTime();
      if (maskGeneratorTimeStamp > statisticsTimeStamp) // there is a secondary mask generator and it has changed
      {
        return true;
      }
    }

    return false;
  }
} // namespace mitk
