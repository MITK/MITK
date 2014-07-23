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
#ifndef __itkRemoveDwiChannelFilter_h_
#define __itkRemoveDwiChannelFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <mitkDiffusionImage.h>

namespace itk{
/** \class RemoveDwiChannelFilter
 *  \brief Remove spcified channels from diffusion-weighted image.
 */

template< class TInPixelType >
class RemoveDwiChannelFilter :
        public ImageToImageFilter< VectorImage< TInPixelType, 3 >, VectorImage< TInPixelType, 3 > >
{

public:

    typedef RemoveDwiChannelFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, VectorImage< TInPixelType, 3 > >  Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(RemoveDwiChannelFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType                                             InputImageType;
    typedef typename Superclass::OutputImageType                                            OutputImageType;
    typedef typename Superclass::OutputImageRegionType                                      OutputImageRegionType;
    typedef typename mitk::DiffusionImage< TInPixelType >::GradientDirectionType            DirectionType;
    typedef typename mitk::DiffusionImage< TInPixelType >::GradientDirectionContainerType   DirectionContainerType;

    void SetChannelIndices( std::vector< unsigned int > indices ){ m_ChannelIndices = indices; }
    void SetDirections( typename DirectionContainerType::Pointer directions ){ m_Directions = directions; }
    typename DirectionContainerType::Pointer GetNewDirections(){ return m_NewDirections; }

    protected:
        RemoveDwiChannelFilter();
    ~RemoveDwiChannelFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType id );

    std::vector< unsigned int >                 m_ChannelIndices;
    typename DirectionContainerType::Pointer    m_Directions;
    typename DirectionContainerType::Pointer    m_NewDirections;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRemoveDwiChannelFilter.txx"
#endif

#endif //__itkRemoveDwiChannelFilter_h_

