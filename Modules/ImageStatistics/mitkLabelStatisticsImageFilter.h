/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelStatisticsImageFilter_h
#define mitkLabelStatisticsImageFilter_h

// This file is based on ITK's itkLabelStatisticsImageFilter.h

#include <itkCompensatedSummation.h>
#include <itkHistogram.h>
#include <itkImageSink.h>
#include <itkNumericTraits.h>
#include <itkSimpleDataObjectDecorator.h>

#include <mutex>
#include <unordered_map>
#include <vector>

#include <mitkLabel.h>

namespace mitk
{
  template <typename TInputImage>
  class LabelStatisticsImageFilter : public itk::ImageSink<TInputImage>
  {
  public:
    using Self = LabelStatisticsImageFilter;
    using Superclass = itk::ImageSink<TInputImage>;
    using Pointer = itk::SmartPointer<Self>;
    using ConstPointer = itk::SmartPointer<const Self>;

    itkFactorylessNewMacro(Self);

    itkTypeMacro(LabelStatisticsImageFilter, itk::ImageSink);

    using IndexType = typename TInputImage::IndexType;
    using SizeType = typename TInputImage::SizeType;
    using RegionType = typename TInputImage::RegionType;
    using PixelType = typename TInputImage::PixelType;
    using LabelPixelType = typename mitk::Label::PixelType;

    static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;

    using RealType = typename itk::NumericTraits<PixelType>::RealType;

    using DataObjectPointer = typename itk::DataObject::Pointer;

    using RealObjectType = itk::SimpleDataObjectDecorator<RealType>;

    using BoundingBoxType = std::vector<itk::IndexValueType>;

    using HistogramType = itk::Statistics::Histogram<RealType>;
    using HistogramPointer = typename HistogramType::Pointer;

    class LabelStatistics
    {
    public:
      LabelStatistics();
      LabelStatistics(unsigned int size, RealType lowerBound, RealType upperBound);
      ~LabelStatistics();

      itk::SizeValueType m_Count;
      itk::SizeValueType m_CountOfPositivePixels;
      RealType m_Min;
      RealType m_Max;
      RealType m_Mean;
      itk::CompensatedSummation<RealType> m_Sum;
      itk::CompensatedSummation<RealType> m_SumOfPositivePixels;
      itk::CompensatedSummation<RealType> m_SumOfSquares;
      itk::CompensatedSummation<RealType> m_SumOfCubes;
      itk::CompensatedSummation<RealType> m_SumOfQuadruples;
      RealType m_Sigma;
      RealType m_Variance;
      RealType m_MPP;
      RealType m_Median;
      RealType m_Uniformity;
      RealType m_UPP;
      RealType m_Entropy;
      RealType m_Skewness;
      RealType m_Kurtosis;
      BoundingBoxType m_BoundingBox;
      HistogramPointer m_Histogram;
    };

    using MapType = std::unordered_map<LabelPixelType, LabelStatistics>;
    using MapIterator = typename MapType::iterator;
    using MapConstIterator = typename MapType::const_iterator;

    using ValidLabelValuesContainerType = std::vector<LabelPixelType>;
    const ValidLabelValuesContainerType& GetValidLabelValues() const;

    void SetHistogramParameters(
      const std::unordered_map<LabelPixelType, unsigned int>& sizes,
      const std::unordered_map<LabelPixelType, RealType>& lowerBounds,
      const std::unordered_map<LabelPixelType, RealType>& upperBounds);

    using LabelImageType = itk::Image<LabelPixelType, ImageDimension>;
    using ProcessObject = itk::ProcessObject;

    itkSetInputMacro(LabelInput, LabelImageType);
    itkGetInputMacro(LabelInput, LabelImageType);

    bool HasLabel(LabelPixelType label) const;
    unsigned int GetNumberOfObjects() const;
    unsigned int GetNumberOfLabels() const;

    PixelType GetMinimum(LabelPixelType label) const;
    PixelType GetMaximum(LabelPixelType label) const;
    RealType GetMean(LabelPixelType label) const;
    RealType GetSigma(LabelPixelType label) const;
    RealType GetVariance(LabelPixelType label) const;
    BoundingBoxType GetBoundingBox(LabelPixelType label) const;
    RegionType GetRegion(LabelPixelType label) const;
    RealType GetSum(LabelPixelType label) const;
    RealType GetSumOfSquares(LabelPixelType label) const;
    RealType GetSumOfCubes(LabelPixelType label) const;
    RealType GetSumOfQuadruples(LabelPixelType label) const;
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;
    RealType GetMPP(LabelPixelType label) const;
    itk::SizeValueType GetCount(LabelPixelType label) const;
    HistogramPointer GetHistogram(LabelPixelType label) const;
    RealType GetEntropy(LabelPixelType label) const;
    RealType GetUniformity(LabelPixelType label) const;
    RealType GetUPP(LabelPixelType label) const;
    RealType GetMedian(LabelPixelType label) const;

  protected:
    LabelStatisticsImageFilter();
    ~LabelStatisticsImageFilter();

    void BeforeStreamedGenerateData() override;
    void ThreadedStreamedGenerateData(const RegionType&) override;
    void AfterStreamedGenerateData() override;

    void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  private:
    const LabelStatistics& GetLabelStatistics(LabelPixelType label) const;
    const LabelStatistics& GetLabelHistogramStatistics(LabelPixelType label) const;

    void MergeMap(MapType& map1, MapType& map2) const;

    MapType m_LabelStatistics;
    ValidLabelValuesContainerType m_ValidLabelValues;

    bool m_ComputeHistograms;
    std::unordered_map<LabelPixelType, unsigned int> m_HistogramSizes;
    std::unordered_map<LabelPixelType, RealType> m_HistogramLowerBounds;
    std::unordered_map<LabelPixelType, RealType> m_HistogramUpperBounds;

    std::mutex m_Mutex;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkLabelStatisticsImageFilter.hxx>
#endif

#endif
