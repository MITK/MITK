/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_MINMAXIMAGEFILTERWITHINDEX_H
#define MITK_MINMAXIMAGEFILTERWITHINDEX_H

#include <MitkImageStatisticsExports.h>

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>



namespace itk
{
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


    RealType GetMin() const
    {
        return m_Min;
    }

    RealType GetMax() const
    {
        return m_Max;
    }

    IndexType GetMinIndex() const
    {
        return m_MinIndex;
    }

    IndexType GetMaxIndex() const
    {
        return m_MaxIndex;
    }

protected:
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

#include "mitkMinMaxImageFilterWithIndex.hxx"


#endif
