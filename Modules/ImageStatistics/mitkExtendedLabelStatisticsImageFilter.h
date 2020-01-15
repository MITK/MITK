/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkExtendedLabelStatisticsImageFilter
#define __mitkExtendedLabelStatisticsImageFilter

#include "itkLabelStatisticsImageFilter.h"

namespace itk
{
  /**
  * \class ExtendedLabelStatisticsImageFilter
  * \brief Extension of the itkLabelStatisticsImageFilter that also calculates the Skewness,Kurtosis,Entropy,Uniformity.
  *
  * This class inherits from the itkLabelStatisticsImageFilter and
  * uses its results for the calculation of seven additional coefficients:
  * the Skewness, Kurtosis, Uniformity, UPP, MPP, Entropy and Median
  *
  *
  */
  template< class TInputImage, class TLabelImage >
  class ExtendedLabelStatisticsImageFilter : public LabelStatisticsImageFilter< TInputImage,  TLabelImage >
  {
  public:

    typedef ExtendedLabelStatisticsImageFilter                      Self;
    typedef LabelStatisticsImageFilter < TInputImage, TLabelImage > Superclass;
    typedef SmartPointer< Self >                                    Pointer;
    typedef SmartPointer< const Self >                              ConstPointer;
    typedef typename Superclass::LabelPixelType                     LabelPixelType;
    typedef typename Superclass::RealType                           RealType;
    typedef typename Superclass::PixelType                          PixelType;
    typedef typename Superclass::MapIterator                        MapIterator;
    typedef typename Superclass::BoundingBoxType                    BoundingBoxType;
    typedef typename Superclass::RegionType                         RegionType;
    typedef  itk::Statistics::Histogram<double> HistogramType;

    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );
    itkTypeMacro(ExtendedLabelStatisticsImageFilter, LabelStatisticsImageFilter);


    /** \class LabelStatistics
     * \brief Statistics stored per label
     * \ingroup ITKImageStatistics
     */
    class LabelStatistics
    {
  public:

      // default constructor
      LabelStatistics()
      {
        // initialized to the default values
        m_Count = NumericTraits< IdentifierType >::ZeroValue();
        m_PositivePixelCount = NumericTraits< IdentifierType >::ZeroValue();
        m_Sum = NumericTraits< RealType >::ZeroValue();
        m_SumOfPositivePixels = NumericTraits< RealType >::ZeroValue();

        m_SumOfSquares = NumericTraits< RealType >::ZeroValue();
        m_SumOfCubes = NumericTraits< RealType >::ZeroValue();
        m_SumOfQuadruples = NumericTraits< RealType >::ZeroValue();

        // Set such that the first pixel encountered can be compared
        m_Minimum = NumericTraits< RealType >::max();
        m_Maximum = NumericTraits< RealType >::NonpositiveMin();

        // Default these to zero
        m_Mean = NumericTraits< RealType >::ZeroValue();
        m_Sigma = NumericTraits< RealType >::ZeroValue();
        m_Variance = NumericTraits< RealType >::ZeroValue();
        m_MPP = NumericTraits< RealType >::ZeroValue();
        m_Median = NumericTraits< RealType >::ZeroValue();
        m_Uniformity = NumericTraits< RealType >::ZeroValue();
        m_UPP = NumericTraits< RealType >::ZeroValue();
        m_Entropy = NumericTraits< RealType >::ZeroValue();
        m_Skewness = NumericTraits< RealType >::ZeroValue();
        m_Kurtosis = NumericTraits< RealType >::ZeroValue();

        unsigned int imageDimension = itkGetStaticConstMacro(ImageDimension);
        m_BoundingBox.resize(imageDimension * 2);
        for ( unsigned int i = 0; i < imageDimension * 2; i += 2 )
          {
          m_BoundingBox[i] = NumericTraits< IndexValueType >::max();
          m_BoundingBox[i + 1] = NumericTraits< IndexValueType >::NonpositiveMin();
          }
        m_Histogram = nullptr;
      }

      // constructor with histogram enabled
      LabelStatistics(int size, RealType lowerBound, RealType upperBound)
      {
        // initialized to the default values
        m_Count = NumericTraits< IdentifierType >::ZeroValue();
        m_PositivePixelCount = NumericTraits< IdentifierType >::ZeroValue();
        m_Sum = NumericTraits< RealType >::ZeroValue();
        m_SumOfPositivePixels = NumericTraits< RealType >::ZeroValue();

        m_SumOfSquares = NumericTraits< RealType >::ZeroValue();
        m_SumOfCubes = NumericTraits< RealType >::ZeroValue();
        m_SumOfQuadruples = NumericTraits< RealType >::ZeroValue();

        // Set such that the first pixel encountered can be compared
        m_Minimum = NumericTraits< RealType >::max();
        m_Maximum = NumericTraits< RealType >::NonpositiveMin();

        // Default these to zero
        m_Mean = NumericTraits< RealType >::ZeroValue();
        m_Sigma = NumericTraits< RealType >::ZeroValue();
        m_Variance = NumericTraits< RealType >::ZeroValue();
        m_MPP = NumericTraits< RealType >::ZeroValue();
        m_Median = NumericTraits< RealType >::ZeroValue();
        m_Uniformity = NumericTraits< RealType >::ZeroValue();
        m_UPP = NumericTraits< RealType >::ZeroValue();
        m_Entropy = NumericTraits< RealType >::ZeroValue();
        m_Skewness = NumericTraits< RealType >::ZeroValue();
        m_Kurtosis = NumericTraits< RealType >::ZeroValue();


        unsigned int imageDimension = itkGetStaticConstMacro(ImageDimension);
        m_BoundingBox.resize(imageDimension * 2);
        for ( unsigned int i = 0; i < imageDimension * 2; i += 2 )
          {
          m_BoundingBox[i] = NumericTraits< IndexValueType >::max();
          m_BoundingBox[i + 1] = NumericTraits< IndexValueType >::NonpositiveMin();
          }

        // Histogram
        m_Histogram = HistogramType::New();
        typename HistogramType::SizeType hsize;
        typename HistogramType::MeasurementVectorType lb;
        typename HistogramType::MeasurementVectorType ub;
        hsize.SetSize(1);
        lb.SetSize(1);
        ub.SetSize(1);
        m_Histogram->SetMeasurementVectorSize(1);
        hsize[0] = size;
        lb[0] = lowerBound;
        ub[0] = upperBound;
        m_Histogram->Initialize(hsize, lb, ub);
      }

      // need copy constructor because of smart pointer to histogram
      LabelStatistics(const LabelStatistics & l)
      {
        m_Count = l.m_Count;
        m_Minimum = l.m_Minimum;
        m_Maximum = l.m_Maximum;
        m_Mean = l.m_Mean;
        m_Sum = l.m_Sum;
        m_SumOfSquares = l.m_SumOfSquares;
        m_Sigma = l.m_Sigma;
        m_Variance = l.m_Variance;
        m_MPP = l.m_MPP;
        m_Median = l.m_Median;
        m_Uniformity = l.m_Uniformity;
        m_UPP = l.m_UPP;
        m_Entropy = l.m_Entropy;
        m_Skewness = l.m_Skewness;
        m_Kurtosis = l.m_Kurtosis;
        m_BoundingBox = l.m_BoundingBox;
        m_Histogram = l.m_Histogram;
        m_SumOfPositivePixels = l.m_SumOfPositivePixels;
        m_PositivePixelCount = l.m_PositivePixelCount;
        m_SumOfCubes = l.m_SumOfCubes;
        m_SumOfQuadruples = l.m_SumOfQuadruples;
      }

      // added for completeness
      LabelStatistics &operator= (const LabelStatistics& l)
      {
        if(this != &l)
          {
          m_Count = l.m_Count;
          m_Minimum = l.m_Minimum;
          m_Maximum = l.m_Maximum;
          m_Mean = l.m_Mean;
          m_Sum = l.m_Sum;
          m_SumOfSquares = l.m_SumOfSquares;
          m_Sigma = l.m_Sigma;
          m_Variance = l.m_Variance;
          m_MPP = l.m_MPP;
          m_Median = l.m_Median;
          m_Uniformity = l.m_Uniformity;
          m_UPP = l.m_UPP;
          m_Entropy = l.m_Entropy;
          m_Skewness = l.m_Skewness;
          m_Kurtosis = l.m_Kurtosis;
          m_BoundingBox = l.m_BoundingBox;
          m_Histogram = l.m_Histogram;
          m_SumOfPositivePixels = l.m_SumOfPositivePixels;
          m_PositivePixelCount = l.m_PositivePixelCount;
          m_SumOfCubes = l.m_SumOfCubes;
          m_SumOfQuadruples = l.m_SumOfQuadruples;
          }
        return *this;
      }

      IdentifierType  m_Count;
      RealType        m_Minimum;
      RealType        m_Maximum;
      RealType        m_Mean;
      RealType        m_Sum;
      RealType        m_SumOfSquares;
      RealType        m_Sigma;
      RealType        m_Variance;
      RealType        m_MPP;
      RealType        m_Median;
      RealType        m_Uniformity;
      RealType        m_UPP;
      RealType        m_Entropy;
      RealType        m_Skewness;
      RealType        m_Kurtosis;
      IdentifierType  m_PositivePixelCount;
      RealType        m_SumOfPositivePixels;
      RealType        m_SumOfCubes;
      RealType        m_SumOfQuadruples;
      typename Superclass::BoundingBoxType m_BoundingBox;
      typename HistogramType::Pointer m_Histogram;
    };

    /** Type of the map used to store data per label */
    typedef itksys::hash_map< LabelPixelType, LabelStatistics >                          MapType;
    typedef typename itksys::hash_map< LabelPixelType, LabelStatistics >::const_iterator StatisticsMapConstIterator;
    typedef typename itksys::hash_map< LabelPixelType, LabelStatistics >::iterator       StatisticsMapIterator;
    typedef IdentifierType                                                               MapSizeType;

    /** Type of the container used to store valid label values */
    typedef std::vector<LabelPixelType> ValidLabelValuesContainerType;

    /** Return the computed Minimum for a label. */
    RealType GetMinimum(LabelPixelType label) const;

    /** Return the computed Maximum for a label. */
    RealType GetMaximum(LabelPixelType label) const;

    /** Return the computed Mean for a label. */
    RealType GetMean(LabelPixelType label) const;

    /** Return the computed Standard Deviation for a label. */
    RealType GetSigma(LabelPixelType label) const;

    /** Return the computed Variance for a label. */
    RealType GetVariance(LabelPixelType label) const;

    /** Return the computed bounding box for a label. */
    BoundingBoxType GetBoundingBox(LabelPixelType label) const;

    /** Return the computed region. */
    RegionType GetRegion(LabelPixelType label) const;

    /** Return the compute Sum for a label. */
    RealType GetSum(LabelPixelType label) const;

    /** Return the number of pixels for a label. */
    MapSizeType GetCount(LabelPixelType label) const;

    /** Return the histogram for a label */
    HistogramType::Pointer GetHistogram(LabelPixelType label) const;

    /*getter method for the new statistics*/
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;
    RealType GetUniformity( LabelPixelType label) const;
    RealType GetMedian( LabelPixelType label) const;
    RealType GetEntropy( LabelPixelType label) const;
    RealType GetMPP( LabelPixelType label) const;
    RealType GetUPP( LabelPixelType label) const;

    bool             GetMaskingNonEmpty() const;

    std::list<int> GetRelevantLabels() const;


    /** specify global Histogram parameters. If the histogram parameters are set with this function, the same min and max value are used for all histograms.  */
    void SetHistogramParameters(const int numBins, RealType lowerBound,
                                RealType upperBound);

    /** specify Histogram parameters for each label individually. Labels in the label image that are not represented in the std::maps here will receive global parameters (if available) */
    void SetHistogramParametersForLabels(std::map<LabelPixelType, unsigned int> numBins, std::map<LabelPixelType, PixelType> lowerBound,
                                         std::map<LabelPixelType, PixelType> upperBound);

  protected:
    ExtendedLabelStatisticsImageFilter():
        m_GlobalHistogramParametersSet(false),
        m_MaskNonEmpty(false),
        m_LabelHistogramParametersSet(false),
        m_PreferGlobalHistogramParameters(false)
    {
        m_NumBins.set_size(1);
    }

    ~ExtendedLabelStatisticsImageFilter() override{}

    void AfterThreadedGenerateData() override;

    /** Initialize some accumulators before the threads run. */
    void BeforeThreadedGenerateData() override;

    /** Multi-thread version GenerateData. */
    void  ThreadedGenerateData(const typename TInputImage::RegionType &
                               outputRegionForThread,
                               ThreadIdType threadId) override;

    /** Does the specified label exist? Can only be called after a call
     * a call to Update(). */
    bool HasLabel(LabelPixelType label) const
    {
      return m_LabelStatistics.find(label) != m_LabelStatistics.end();
    }

  private:
    std::vector< MapType >        m_LabelStatisticsPerThread;
    MapType                       m_LabelStatistics;
    ValidLabelValuesContainerType m_ValidLabelValues;

    bool m_GlobalHistogramParametersSet;

    typename HistogramType::SizeType m_NumBins;

    RealType            m_LowerBound;
    RealType            m_UpperBound;

    bool                    m_MaskNonEmpty;

    bool m_LabelHistogramParametersSet;
    std::map<LabelPixelType, PixelType> m_LabelMin, m_LabelMax;
    std::map<LabelPixelType, unsigned int> m_LabelNBins;
    bool m_PreferGlobalHistogramParameters;

  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedLabelStatisticsImageFilter.hxx"
#endif

#endif
