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
#ifndef __itkNonLocalMeansDenoisingFilter_h_
#define __itkNonLocalMeansDenoisingFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkNeighborhoodIterator.h>

namespace itk{
/** \class NonLocalMeansDenoisingFilter
 */

template< class TInPixelType, class TOutPixelType >
class NonLocalMeansDenoisingFilter :
        public ImageToImageFilter< VectorImage < TInPixelType, 3 >, VectorImage < TOutPixelType, 3 > >
{

public:

    typedef NonLocalMeansDenoisingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage < TInPixelType, 3 >, VectorImage < TOutPixelType, 3 > >  Superclass;
    typedef mitk::DiffusionImage< short >::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionImage< short >::GradientDirectionContainerType::Pointer GradientContainerType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(NonLocalMeansDenoisingFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    itkSetMacro( B_value, double )
    itkSetMacro( GradientDirections, GradientContainerType )

    itkSetMacro( H, unsigned int)
    void SetNRadius(unsigned int n);
    void SetVRadius(unsigned int v);

    protected:
        NonLocalMeansDenoisingFilter();
    ~NonLocalMeansDenoisingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    double    m_B_value;
    GradientContainerType m_GradientDirections;

    typename NeighborhoodIterator < VectorImage < TInPixelType, 3 > >::RadiusType m_V_Radius;
    typename NeighborhoodIterator < VectorImage < TInPixelType, 3 > >::RadiusType m_N_Radius;
    unsigned double m_H;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNonLocalMeansDenoisingFilter.cpp"
#endif

#endif //__itkNonLocalMeansDenoisingFilter_h_
