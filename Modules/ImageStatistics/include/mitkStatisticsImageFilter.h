/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStatisticsImageFilter_h
#define mitkStatisticsImageFilter_h

// This file is based on ITK's itkStatisticsImageFilter.h

#include <mitkCommon.h>

#include <itkArray.h>
#include <itkCompensatedSummation.h>
#include <itkHistogram.h>
#include <itkImageSink.h>
#include <itkNumericTraits.h>
#include <itkSimpleDataObjectDecorator.h>

#include <mutex>

namespace mitk
{
  template <typename TInputImage>
  class StatisticsImageFilter : public itk::ImageSink<TInputImage>
  {
  public:
    using Self = StatisticsImageFilter;
    using Superclass = itk::ImageSink<TInputImage>;
    using Pointer = itk::SmartPointer<Self>;
    using ConstPointer = itk::SmartPointer<const Self>;

    itkFactorylessNewMacro(Self);

    itkTypeMacro(StatisticsImageFilter, itk::ImageSink);

    using RegionType = typename TInputImage::RegionType;
    using PixelType = typename TInputImage::PixelType;

    using RealType = typename itk::NumericTraits<PixelType>::RealType;

    using HistogramType = typename itk::Statistics::Histogram<RealType>;
    using HistogramPointer = itk::SmartPointer<HistogramType>;
    
    using DataObjectPointer = typename itk::DataObject::Pointer;

    template <typename T>
    using SimpleDataObjectDecorator = itk::SimpleDataObjectDecorator<T>;

    using RealObjectType = SimpleDataObjectDecorator<RealType>;
    using PixelObjectType = SimpleDataObjectDecorator<PixelType>;
    using ProcessObject = itk::ProcessObject;

    itkGetDecoratedOutputMacro(Minimum, PixelType);
    itkGetDecoratedOutputMacro(Maximum, PixelType);
    itkGetDecoratedOutputMacro(Mean, RealType);
    itkGetDecoratedOutputMacro(Sigma, RealType);
    itkGetDecoratedOutputMacro(Variance, RealType);
    itkGetDecoratedOutputMacro(Sum, RealType);
    itkGetDecoratedOutputMacro(SumOfSquares, RealType);
    itkGetDecoratedOutputMacro(SumOfCubes, RealType);
    itkGetDecoratedOutputMacro(SumOfQuadruples, RealType);
    itkGetDecoratedOutputMacro(Skewness, RealType);
    itkGetDecoratedOutputMacro(Kurtosis, RealType);
    itkGetDecoratedOutputMacro(MPP, RealType);
    itkGetDecoratedOutputMacro(Histogram, HistogramPointer);
    itkGetDecoratedOutputMacro(Entropy, RealType);
    itkGetDecoratedOutputMacro(Uniformity, RealType);
    itkGetDecoratedOutputMacro(UPP, RealType);
    itkGetDecoratedOutputMacro(Median, RealType);

    void SetHistogramParameters(unsigned int size, RealType lowerBound, RealType upperBound);

    using DataObjectIdentifierType = itk::ProcessObject::DataObjectIdentifierType;
    using Superclass::MakeOutput;
    
    /** Make a DataObject of the correct type to be used as the specified output. */
    DataObjectPointer MakeOutput(const DataObjectIdentifierType& name) override;

  protected:
    StatisticsImageFilter();
    ~StatisticsImageFilter();

    itkSetDecoratedOutputMacro(Minimum, PixelType);
    itkSetDecoratedOutputMacro(Maximum, PixelType);
    itkSetDecoratedOutputMacro(Mean, RealType);
    itkSetDecoratedOutputMacro(Sigma, RealType);
    itkSetDecoratedOutputMacro(Variance, RealType);
    itkSetDecoratedOutputMacro(Sum, RealType);
    itkSetDecoratedOutputMacro(SumOfSquares, RealType);
    itkSetDecoratedOutputMacro(SumOfCubes, RealType);
    itkSetDecoratedOutputMacro(SumOfQuadruples, RealType);
    itkSetDecoratedOutputMacro(Skewness, RealType);
    itkSetDecoratedOutputMacro(Kurtosis, RealType);
    itkSetDecoratedOutputMacro(MPP, RealType);
    itkSetDecoratedOutputMacro(Histogram, HistogramPointer);
    itkSetDecoratedOutputMacro(Entropy, RealType);
    itkSetDecoratedOutputMacro(Uniformity, RealType);
    itkSetDecoratedOutputMacro(UPP, RealType);
    itkSetDecoratedOutputMacro(Median, RealType);

    void BeforeStreamedGenerateData() override;
    void ThreadedStreamedGenerateData(const RegionType&) override;
    void AfterStreamedGenerateData() override;

    void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  private:
    HistogramPointer CreateInitializedHistogram() const;

    bool m_ComputeHistogram;
    unsigned int m_HistogramSize;
    RealType m_HistogramLowerBound;
    RealType m_HistogramUpperBound;
    HistogramPointer m_Histogram;

    itk::CompensatedSummation<RealType> m_Sum;
    itk::CompensatedSummation<RealType> m_SumOfPositivePixels;
    itk::CompensatedSummation<RealType> m_SumOfSquares;
    itk::CompensatedSummation<RealType> m_SumOfCubes;
    itk::CompensatedSummation<RealType> m_SumOfQuadruples;

    itk::SizeValueType m_Count;
    itk::SizeValueType m_CountOfPositivePixels;
    PixelType m_Min;
    PixelType m_Max;

    std::mutex m_Mutex;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkStatisticsImageFilter.hxx>
#endif

#endif
