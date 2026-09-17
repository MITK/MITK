/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMinMaxLabelmageFilterWithIndex_h
#define mitkMinMaxLabelmageFilterWithIndex_h

#include <MitkImageStatisticsExports.h>

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>

#include <unordered_map>
#include <vector>


namespace itk
{
/**
 * \brief Computes per-label and global minimum/maximum pixel values with their indices.
 *
 * This ITK filter computes the minimum and maximum pixel values (and their
 * indices) for each label in a label image, as well as the global extrema
 * across all labels. It uses multi-threaded processing for performance.
 *
 * \tparam TInputImage The type of the input intensity image.
 * \tparam TLabelImage The type of the label image.
 *
 * \sa MinMaxImageFilterWithIndex
 * \sa LabelStatisticsImageFilter
 */
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
     * \brief Container holding minimum/maximum values and their indices for a single label.
     *
     * Extrema always describe at least one pixel, so they are constructed from the
     * first pixel of the label rather than from sentinel values.
     */
    class LabelExtrema
    {
    public:
        PixelType m_Min, m_Max;
        IndexType m_MinIndex, m_MaxIndex;

        LabelExtrema(PixelType value, const IndexType& index):
            m_Min(value),
            m_Max(value),
            m_MinIndex(index),
            m_MaxIndex(index)
        {}
    };

    typedef typename std::unordered_map<LabelPixelType, LabelExtrema> ExtremaMapType;
    typedef typename ExtremaMapType::iterator                         ExtremaMapTypeIterator;
    typedef typename ExtremaMapType::const_iterator                   ExtremaMapTypeConstIterator;
    typedef typename ExtremaMapType::value_type                       MapValueType;

    /**
     * \brief Get the minimum pixel value for the given label.
     * \param[in] label The label value to query.
     * \return The minimum pixel value for that label.
     * \throws itk::ExceptionObject if the label does not occur in the label image.
     */
    PixelType GetMin(LabelPixelType label) const
    {
        return this->GetExtrema(label).m_Min;
    }

    /**
     * \brief Get the maximum pixel value for the given label.
     * \param[in] label The label value to query.
     * \return The maximum pixel value for that label.
     * \throws itk::ExceptionObject if the label does not occur in the label image.
     */
    PixelType GetMax(LabelPixelType label) const
    {
        return this->GetExtrema(label).m_Max;
    }

    /**
     * \brief Get all labels for which extrema have been computed.
     * \return A vector containing all label pixel values with computed min/max.
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

    /**
     * \brief Get the index of the minimum pixel value for the given label.
     * \param[in] label The label value to query.
     * \return The image index of the minimum pixel for that label; for a repeated minimum the first one in scan order.
     * \throws itk::ExceptionObject if the label does not occur in the label image.
     */
    IndexType GetMinIndex(LabelPixelType label) const
    {
        return this->GetExtrema(label).m_MinIndex;
    }

    /**
     * \brief Get the index of the maximum pixel value for the given label.
     * \param[in] label The label value to query.
     * \return The image index of the maximum pixel for that label; for a repeated maximum the first one in scan order.
     * \throws itk::ExceptionObject if the label does not occur in the label image.
     */
    IndexType GetMaxIndex(LabelPixelType label) const
    {
        return this->GetExtrema(label).m_MaxIndex;
    }

    /**
     * \brief Get the global minimum pixel value across all labels.
     * \return The global minimum pixel value.
     */
    PixelType GetGlobalMin() const
    {
        return m_GlobalMin;
    }

    /**
     * \brief Get the global maximum pixel value across all labels.
     * \return The global maximum pixel value.
     */
    PixelType GetGlobalMax() const
    {
        return m_GlobalMax;
    }

    /**
     * \brief Get the index of the global minimum pixel value.
     * \return The image index of the global minimum pixel.
     */
    IndexType GetGlobalMinIndex() const
    {
        return m_GlobalMinIndex;
    }

    /**
     * \brief Get the index of the global maximum pixel value.
     * \return The image index of the global maximum pixel.
     */
    IndexType GetGlobalMaxIndex() const
    {
        return m_GlobalMaxIndex;
    }

    /**
     * \brief Set the label image defining label regions.
     * \param[in] input Pointer to the label image.
     */
    void SetLabelInput(const TLabelImage *input)
    {
      // Process object is not const-correct so the const casting is required.
      this->SetNthInput( 1, const_cast< TLabelImage * >( input ) );
    }

    /**
     * \brief Get the label image.
     * \return Const pointer to the label image.
     */
    const TLabelImage * GetLabelInput() const
    {
      return itkDynamicCastInDebugMode< TLabelImage * >( const_cast< DataObject * >( this->ProcessObject::GetInput(1) ) );
    }

protected:
    MinMaxLabelImageFilterWithIndex()
    {
      this->DynamicMultiThreadingOff();
    }

    void AllocateOutputs() override;

    void ThreadedGenerateData(const RegionType &
                                          outputRegionForThread,
                                          ThreadIdType threadId) override;

    void BeforeThreadedGenerateData() override;

    void AfterThreadedGenerateData() override;

private:
    const LabelExtrema& GetExtrema(LabelPixelType label) const
    {
        const auto it = m_LabelExtrema.find(label);

        if (it == m_LabelExtrema.end())
        {
            itkExceptionMacro(<< "Label " << static_cast<typename NumericTraits<LabelPixelType>::PrintType>(label)
                              << " does not occur in the label image.");
        }

        return it->second;
    }

    std::vector<ExtremaMapType> m_ThreadExtrema;

    ExtremaMapType m_LabelExtrema;
    PixelType m_GlobalMin;
    PixelType m_GlobalMax;
    IndexType m_GlobalMinIndex, m_GlobalMaxIndex;
};
}

#include <mitkMinMaxLabelmageFilterWithIndex.tpp>


#endif
