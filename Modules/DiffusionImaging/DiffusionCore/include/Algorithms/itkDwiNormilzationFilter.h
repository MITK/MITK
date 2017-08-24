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
#include <itkLabelStatisticsImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkShiftScaleImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>

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

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(DwiNormilzationFilter, ImageToImageFilter)

    typedef itk::Image< double, 3 > DoubleImageType;
    typedef itk::Image< unsigned char, 3 > UcharImageType;
    typedef itk::Image< unsigned short, 3 > BinImageType;
    typedef itk::Image< TInPixelType, 3 > TInPixelImageType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer GradientContainerType;

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typedef ImageRegionConstIterator< UcharImageType > MaskIteratorType;


    itkSetMacro( GradientDirections, GradientContainerType )
    itkSetMacro( ScalingFactor, TInPixelType )
    itkSetMacro( MaskImage, UcharImageType::Pointer )
    itkSetMacro( NewMean, double )
    itkSetMacro( NewStdev, double )

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
    double                  m_Mean;
    double                  m_Stdev;
    double                  m_NewMean;
    double                  m_NewStdev;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDwiNormilzationFilter.txx"
#endif

#endif //__itkDwiNormilzationFilter_h_

