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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkDwiNormilzationFilter_h_
#define __itkDwiNormilzationFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkShiftScaleImageFilter.h>

namespace itk{
/** \class DwiNormilzationFilter
 *  \brief Normalizes the data vectors either using the specified reference value or the voxelwise baseline value.
 */

template< class TInPixelType >
class DwiNormilzationFilter :
        public ImageToImageFilter< VectorImage< TInPixelType, 3 >, VectorImage< TInPixelType, 3 > >
{

public:

    typedef DwiNormilzationFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, VectorImage< TInPixelType, 3 > >  Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(DwiNormilzationFilter, ImageToImageFilter)

    typedef itk::Image< double, 3 > DoubleImageType;
    typedef itk::Image< unsigned char, 3 > UcharImageType;
    typedef itk::Image< unsigned short, 3 > BinImageType;
    typedef itk::Image< TInPixelType, 3 > TInPixelImageType;
    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
    typedef mitk::DiffusionImage< short >::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionImage< short >::GradientDirectionContainerType::Pointer GradientContainerType;
    typedef itk::LabelStatisticsImageFilter< TInPixelImageType,BinImageType >  StatisticsFilterType;
    typedef itk::Statistics::Histogram< typename TInPixelImageType::PixelType  > HistogramType;
    typedef typename HistogramType::MeasurementType MeasurementType;
    typedef itk::ShiftScaleImageFilter<TInPixelImageType, DoubleImageType>  ShiftScaleImageFilterType;

    itkSetMacro( GradientDirections, GradientContainerType )
    itkSetMacro( ScalingFactor, TInPixelType )
    itkSetMacro( UseGlobalReference, bool )
    itkSetMacro( MaskImage, UcharImageType::Pointer )
    itkSetMacro( Reference, double )

    protected:
        DwiNormilzationFilter();
    ~DwiNormilzationFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    UcharImageType::Pointer m_MaskImage;
    GradientContainerType   m_GradientDirections;
    int                     m_B0Index;
    TInPixelType            m_ScalingFactor;
    bool                    m_UseGlobalReference;
    double                  m_Reference;

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDwiNormilzationFilter.txx"
#endif

#endif //__itkDwiNormilzationFilter_h_

