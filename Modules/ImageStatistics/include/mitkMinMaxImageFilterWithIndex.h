/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMinMaxImageFilterWithIndex_h
#define mitkMinMaxImageFilterWithIndex_h

#include <MitkImageStatisticsExports.h>

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>



namespace itk
{
/**
 * \brief Computes the minimum and maximum pixel values and their indices in an image.
 *
 * This ITK filter traverses all pixels of an image using multi-threaded
 * processing and determines the global minimum and maximum pixel values
 * along with their corresponding image indices.
 *
 * \tparam TInputImage The type of the input image.
 *
 * \sa MinMaxLabelImageFilterWithIndex
 */
template <typename TInputImage>
class MinMaxImageFilterWithIndex: public itk::ImageToImageFilter<TInputImage, TInputImage>
{
public:
    /** Standard Self typedef */
    typedef MinMaxImageFilterWithIndex                          Self;
    typedef ImageToImageFilter< TInputImage, TInputImage > Superclass;
    typedef SmartPointer< Self >                           Pointer;
    typedef SmartPointer< const Self >                     ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(MinMaxImageFilterWithIndex, ImageToImageFilter);

    typedef typename TInputImage::RegionType RegionType;
    typedef typename TInputImage::SizeType   SizeType;
    typedef typename TInputImage::IndexType  IndexType;
    typedef typename TInputImage::PixelType  PixelType;
    typedef typename NumericTraits< PixelType >::RealType RealType;

    /**
     * \brief Get the minimum pixel value found in the image.
     * \return The minimum pixel value.
     */
    RealType GetMin() const
    {
        return m_Min;
    }

    /**
     * \brief Get the maximum pixel value found in the image.
     * \return The maximum pixel value.
     */
    RealType GetMax() const
    {
        return m_Max;
    }

    /**
     * \brief Get the image index of the minimum pixel value.
     * \return The index of the minimum pixel.
     */
    IndexType GetMinIndex() const
    {
        return m_MinIndex;
    }

    /**
     * \brief Get the image index of the maximum pixel value.
     * \return The index of the maximum pixel.
     */
    IndexType GetMaxIndex() const
    {
        return m_MaxIndex;
    }

protected:
    MinMaxImageFilterWithIndex()
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
    std::vector<PixelType> m_ThreadMin;
    std::vector<PixelType> m_ThreadMax;
    std::vector<IndexType> m_ThreadMinIndex;
    std::vector<IndexType> m_ThreadMaxIndex;

    PixelType m_Min;
    PixelType m_Max;
    IndexType m_MinIndex;
    IndexType m_MaxIndex;
};
}

#include <mitkMinMaxImageFilterWithIndex.tpp>


#endif
