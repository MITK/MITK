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

namespace itk{
/** \class DwiNormilzationFilter
 *  \brief Max-Normalizes the data vectors either using the global baseline maximum or the voxelwise baseline value.
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

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
    typedef mitk::DiffusionImage< short >::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionImage< short >::GradientDirectionContainerType::Pointer GradientContainerType;

    itkSetMacro( GradientDirections, GradientContainerType )
    itkSetMacro( NewMax, TInPixelType )
    itkSetMacro( UseGlobalMax, bool )

    protected:
        DwiNormilzationFilter();
    ~DwiNormilzationFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    GradientContainerType   m_GradientDirections;
    int                     m_B0Index;
    TInPixelType            m_NewMax;
    bool                    m_UseGlobalMax;
    double                  m_GlobalMax;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDwiNormilzationFilter.txx"
#endif

#endif //__itkDwiNormilzationFilter_h_

