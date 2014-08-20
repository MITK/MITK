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
#ifndef __itkExtractDwiChannelFilter_h_
#define __itkExtractDwiChannelFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <mitkDiffusionImage.h>

namespace itk{
/** \class ExtractDwiChannelFilter
 *  \brief Remove spcified channels from diffusion-weighted image.
 */

template< class TInPixelType >
class ExtractDwiChannelFilter :
        public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TInPixelType, 3 > >
{

public:

    typedef ExtractDwiChannelFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TInPixelType, 3 > >  Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(ExtractDwiChannelFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType                                             InputImageType;
    typedef typename Superclass::OutputImageType                                            OutputImageType;
    typedef typename Superclass::OutputImageRegionType                                      OutputImageRegionType;

    itkSetMacro( ChannelIndex, unsigned int )

    protected:
        ExtractDwiChannelFilter();
    ~ExtractDwiChannelFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType );

    unsigned int    m_ChannelIndex;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExtractDwiChannelFilter.txx"
#endif

#endif //__itkExtractDwiChannelFilter_h_

