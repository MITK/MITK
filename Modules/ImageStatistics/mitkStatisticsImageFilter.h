/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// This file is based on ITK's itkStatisticsImageFilter.h

#ifndef mitkStatisticsImageFilter
#define mitkStatisticsImageFilter

#include <mitkCommon.h>

#include <itkArray.h>
#include <itkCompensatedSummation.h>
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
    /** Standard Self type alias */
    using Self = StatisticsImageFilter;
    using Superclass = itk::ImageSink<TInputImage>;
    using Pointer = itk::SmartPointer<Self>;
    using ConstPointer = itk::SmartPointer<const Self>;

    /** Method for creation bypassing the object factory. */
    itkFactorylessNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(StatisticsImageFilter, itk::ImageSink);

    /** Image related type alias. */
    using InputImagePointer = typename TInputImage::Pointer;

    using RegionType = typename TInputImage::RegionType;
    using SizeType = typename TInputImage::SizeType;
    using IndexType = typename TInputImage::IndexType;
    using PixelType = typename TInputImage::PixelType;

    /** Image related type alias. */
    static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;

    /** Type to use for computations. */
    using RealType = typename itk::NumericTraits<PixelType>::RealType;
    
    /** Smart Pointer type to a DataObject. */
    using DataObjectPointer = typename itk::DataObject::Pointer;

    template <typename T>
    using SimpleDataObjectDecorator = itk::SimpleDataObjectDecorator<T>;

    /** Type of DataObjects used for scalar outputs */
    using RealObjectType = SimpleDataObjectDecorator<RealType>;
    using PixelObjectType = SimpleDataObjectDecorator<PixelType>;

    /** Return the computed Minimum. */
    itkGetDecoratedOutputMacro(Minimum, PixelType);

    /** Return the computed Maximum. */
    itkGetDecoratedOutputMacro(Maximum, PixelType);

    /** Return the computed Mean. */
    itkGetDecoratedOutputMacro(Mean, RealType);

    /** Return the computed Standard Deviation. */
    itkGetDecoratedOutputMacro(Sigma, RealType);

    /** Return the computed Variance. */
    itkGetDecoratedOutputMacro(Variance, RealType);

    /** Return the computed Sum. */
    itkGetDecoratedOutputMacro(Sum, RealType);

    /** Return the computed Sum of Squares. */
    itkGetDecoratedOutputMacro(SumOfSquares, RealType);

    /** Return the computed Sum of Cubes. */
    itkGetDecoratedOutputMacro(SumOfCubes, RealType);

    /** Return the computed Sum of Quadruples. */
    itkGetDecoratedOutputMacro(SumOfQuadruples, RealType);

    /** Return the computed Skewness. */
    itkGetDecoratedOutputMacro(Skewness, RealType);

    /** Return the computed Kurtosis. */
    itkGetDecoratedOutputMacro(Kurtosis, RealType);

    /** Return the computed Mean of Positive Pixels. */
    itkGetDecoratedOutputMacro(MPP, RealType);

    /** Return the computed Entropy. */
    itkGetDecoratedOutputMacro(Entropy, RealType);

    /** Return the computed Uniformity. */
    itkGetDecoratedOutputMacro(Uniformity, RealType);

    /** Return the computed UPP. */
    itkGetDecoratedOutputMacro(UPP, RealType);

    /** Return the computed Median. */
    itkGetDecoratedOutputMacro(Median, RealType);

    using DataObjectIdentifierType = itk::ProcessObject::DataObjectIdentifierType;
    using Superclass::MakeOutput;
    
    /** Make a DataObject of the correct type to be used as the specified output. */
    DataObjectPointer MakeOutput(const DataObjectIdentifierType& name) override;

#ifdef ITK_USE_CONCEPT_CHECKING
    // Begin concept checking
    itkConceptMacro(InputHasNumericTraitsCheck, (itk::Concept::HasNumericTraits<PixelType>));
    // End concept checking
#endif

  protected:
    StatisticsImageFilter();
    ~StatisticsImageFilter() override = default;

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
    itkSetDecoratedOutputMacro(Entropy, RealType);
    itkSetDecoratedOutputMacro(Uniformity, RealType);
    itkSetDecoratedOutputMacro(UPP, RealType);
    itkSetDecoratedOutputMacro(Median, RealType);

    /** Initialize some accumulators before the threads run. */
    void BeforeStreamedGenerateData() override;
    
    void ThreadedStreamedGenerateData(const RegionType&) override;
    
    /** Set outputs to computed values from all regions. */
    void AfterStreamedGenerateData() override;

    void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  private:
    itk::CompensatedSummation<RealType> m_ThreadSum;
    itk::CompensatedSummation<RealType> m_SumOfPositivePixels;
    itk::CompensatedSummation<RealType> m_SumOfSquares;
    itk::CompensatedSummation<RealType> m_SumOfCubes;
    itk::CompensatedSummation<RealType> m_SumOfQuadruples;

    itk::SizeValueType m_Count;
    itk::SizeValueType m_CountOfPositivePixels;
    PixelType m_ThreadMin;
    PixelType m_ThreadMax;

    std::mutex m_Mutex;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkStatisticsImageFilter.hxx>
#endif

#endif
