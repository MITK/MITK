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

#ifndef itkLocalIntensityFilter_h
#define itkLocalIntensityFilter_h

#include "itkImageToImageFilter.h"
#include "itkNumericTraits.h"
#include "itkArray.h"
#include "itkSimpleDataObjectDecorator.h"

namespace itk
{

  template< typename TInputImage >
  class ITK_TEMPLATE_EXPORT LocalIntensityFilter :
    public ImageToImageFilter< TInputImage, TInputImage >
  {
  public:
    /** Standard Self typedef */
    typedef LocalIntensityFilter                           Self;
    typedef ImageToImageFilter< TInputImage, TInputImage > Superclass;
    typedef SmartPointer< Self >                           Pointer;
    typedef SmartPointer< const Self >                     ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(LocalIntensityFilter, ImageToImageFilter);

    /** Image related typedefs. */
    typedef typename TInputImage::Pointer InputImagePointer;

    typedef typename TInputImage::RegionType RegionType;
    typedef typename TInputImage::SizeType   SizeType;
    typedef typename TInputImage::IndexType  IndexType;
    typedef typename TInputImage::PixelType  PixelType;

    typedef Image<unsigned short, TInputImage::ImageDimension> MaskImageType;

    /** Image related typedefs. */
    itkStaticConstMacro(ImageDimension, unsigned int,
      TInputImage::ImageDimension);

    /** Type to use for computations. */
    typedef typename NumericTraits< PixelType >::RealType RealType;

    /** Smart Pointer type to a DataObject. */
    typedef typename DataObject::Pointer DataObjectPointer;

    /** Type of DataObjects used for scalar outputs */
    typedef SimpleDataObjectDecorator< RealType >  RealObjectType;
    typedef SimpleDataObjectDecorator< PixelType > PixelObjectType;

    /** Return the computed Mean. */
    RealType GetLocalPeak() const
    {
      return this->GetLocalPeakOutput()->Get();
    }
    RealObjectType * GetLocalPeakOutput();

    const RealObjectType * GetLocalPeakOutput() const;

    /** Return the computed Standard Deviation. */
    RealType GetGlobalPeak() const
    {
      return this->GetGlobalPeakOutput()->Get();
    }
    RealObjectType * GetGlobalPeakOutput();

    const RealObjectType * GetGlobalPeakOutput() const;

    /** Return the computed Variance. */
    RealType GetLocalMaximum() const
    {
      return this->GetLocalMaximumOutput()->Get();
    }
    RealObjectType * GetLocalMaximumOutput();

    const RealObjectType * GetLocalMaximumOutput() const;

    void SetMask(typename MaskImageType::Pointer mask)
    {
      m_Mask = mask;
    }

    itkSetMacro(Range, double);
    itkGetMacro(Range, double);
    itkGetConstMacro(Range, double);

    /** Make a DataObject of the correct type to be used as the specified
    * output. */
    typedef ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;
    using Superclass::MakeOutput;
    virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx) ITK_OVERRIDE;

#ifdef ITK_USE_CONCEPT_CHECKING
    // Begin concept checking
    itkConceptMacro(InputHasNumericTraitsCheck,
      (Concept::HasNumericTraits< PixelType >));
    // End concept checking
#endif

  protected:
    LocalIntensityFilter();
    ~LocalIntensityFilter() ITK_OVERRIDE {}
    void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

    /** Pass the input through unmodified. Do this by Grafting in the
    *  AllocateOutputs method.
    */
    void AllocateOutputs() ITK_OVERRIDE;

    /** Initialize some accumulators before the threads run. */
    void BeforeThreadedGenerateData() ITK_OVERRIDE;

    /** Do final mean and variance computation from data accumulated in threads.
    */
    void AfterThreadedGenerateData() ITK_OVERRIDE;

    /** Multi-thread version GenerateData. */
    void  ThreadedGenerateData(const RegionType &
      outputRegionForThread,
      ThreadIdType threadId) ITK_OVERRIDE;

    // Override since the filter needs all the data for the algorithm
    void GenerateInputRequestedRegion() ITK_OVERRIDE;

    // Override since the filter produces all of its output
    void EnlargeOutputRequestedRegion(DataObject *data) ITK_OVERRIDE;

  private:
    ITK_DISALLOW_COPY_AND_ASSIGN(LocalIntensityFilter);

    Array< RealType >       m_ThreadLocalMaximum;
    Array< RealType >       m_ThreadLocalPeakValue;
    Array< RealType >       m_ThreadGlobalPeakValue;
    typename MaskImageType::Pointer m_Mask;
    double m_Range;
  }; // end of class
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLocalIntensityFilter.hxx"
#endif

#endif

