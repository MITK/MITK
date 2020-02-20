/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_MINMAXLABELIMAGEFILTERWITHINDEX_H
#define MITK_MINMAXLABELIMAGEFILTERWITHINDEX_H

#include <MitkImageStatisticsExports.h>

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include "itksys/hash_map.hxx"


namespace itk
{
template <typename TInputImage, typename TLabelImage>
class MinMaxLabelImageFilterWithIndex: public itk::ImageToImageFilter<TInputImage, TInputImage>
{
public:
    /** Standard Self typedef */
    typedef MinMaxLabelImageFilterWithIndex                          Self;
    typedef ImageToImageFilter< TInputImage, TInputImage > Superclass;
    typedef SmartPointer< Self >                           Pointer;
    typedef SmartPointer< const Self >                     ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(MinMaxLabelImageFilterWithIndex, ImageToImageFilter);

    typedef typename TInputImage::RegionType RegionType;
    typedef typename TInputImage::SizeType   SizeType;
    typedef typename TInputImage::IndexType  IndexType;
    typedef typename TInputImage::PixelType  PixelType;
    typedef typename NumericTraits< PixelType >::RealType RealType;

    typedef typename TLabelImage::RegionType LabelRegionType;
    typedef typename TLabelImage::SizeType   LabelSizeType;
    typedef typename TLabelImage::IndexType  LabelIndexType;
    typedef typename TLabelImage::PixelType  LabelPixelType;

    /**
     * @brief The LabelExtrema class is just a container for global min/max values and their indices as well as all min and max values (+indices) of the mask labels
     */
    class LabelExtrema
    {
    public:
        PixelType m_Min, m_Max;
        IndexType m_MinIndex, m_MaxIndex;

        LabelExtrema():
            m_Min(std::numeric_limits<PixelType>::max()),
            m_Max(std::numeric_limits<PixelType>::min())
        {}
    };

    typedef typename itksys::hash_map<LabelPixelType, LabelExtrema>     ExtremaMapType;
    typedef typename ExtremaMapType::iterator                   ExtremaMapTypeIterator;
    typedef typename ExtremaMapType::const_iterator             ExtremaMapTypeConstIterator;
    typedef typename ExtremaMapType::value_type MapValueType;

    PixelType GetMin(LabelPixelType label) const
    {
        ExtremaMapTypeConstIterator it = m_LabelExtrema.find(label);
        if (it == m_LabelExtrema.end())
        {
            MITK_ERROR << "invalid label";
        }

        return (*it).second.m_Min;
    }

    PixelType GetMax(LabelPixelType label) const
    {
        ExtremaMapTypeConstIterator it = m_LabelExtrema.find(label);
        if (it == m_LabelExtrema.end())
        {
            MITK_ERROR << "invalid label";
        }

        return (*it).second.m_Max;
    }

    /**
     * @brief Returns a std::vector containing all labels for which min and max values (and indices) have been computed
     */
    std::vector<LabelPixelType> GetRelevantLabels() const
    {
        std::vector<LabelPixelType> labels;
        for (auto&& it:m_LabelExtrema)
        {
            labels.push_back(it.first);
        }
        return labels;
    }

    IndexType GetMinIndex(LabelPixelType label) const
    {
        ExtremaMapTypeConstIterator it = m_LabelExtrema.find(label);
        if (it == m_LabelExtrema.end())
        {
            MITK_ERROR << "invalid label";
        }

        return (*it).second.m_MinIndex;

    }

    IndexType GetMaxIndex(LabelPixelType label) const
    {
        ExtremaMapTypeConstIterator it = m_LabelExtrema.find(label);
        if (it == m_LabelExtrema.end())
        {
            MITK_ERROR << "invalid label";
        }

        return (*it).second.m_MaxIndex;

    }

    PixelType GetGlobalMin() const
    {
        return m_GlobalMin;
    }

    PixelType GetGlobalMax() const
    {
        return m_GlobalMax;
    }

    IndexType GetGlobalMinIndex() const
    {
        return m_GlobalMinIndex;
    }

    IndexType GetGlobalMaxIndex() const
    {
        return m_GlobalMaxIndex;
    }

    /** Set the label image */
    void SetLabelInput(const TLabelImage *input)
    {
      // Process object is not const-correct so the const casting is required.
      this->SetNthInput( 1, const_cast< TLabelImage * >( input ) );
    }

    /** Get the label image */
    const TLabelImage * GetLabelInput() const
    {
      return itkDynamicCastInDebugMode< TLabelImage * >( const_cast< DataObject * >( this->ProcessObject::GetInput(1) ) );
    }

protected:
    void AllocateOutputs() override;

    void ThreadedGenerateData(const RegionType &
                                          outputRegionForThread,
                                          ThreadIdType threadId) override;

    void BeforeThreadedGenerateData() override;

    void AfterThreadedGenerateData() override;

private:
    std::vector<ExtremaMapType> m_ThreadExtrema;

    ExtremaMapType m_LabelExtrema;
    PixelType m_GlobalMin;
    PixelType m_GlobalMax;
    IndexType m_GlobalMinIndex, m_GlobalMaxIndex;
};
}

#include "mitkMinMaxLabelmageFilterWithIndex.hxx"


#endif
