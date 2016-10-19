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
#ifndef __itkAdcImageFilter_h_
#define __itkAdcImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionPropertyHelper.h>

namespace itk{
/** \class AdcImageFilter
 */

template< class TInPixelType, class TOutPixelType >
class AdcImageFilter :
        public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >
{

public:

    typedef AdcImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >  Superclass;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer GradientContainerType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(AdcImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    itkSetMacro( B_value, double )
    itkSetMacro( GradientDirections, GradientContainerType )

    protected:
        AdcImageFilter();
    ~AdcImageFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    double    m_B_value;
    GradientContainerType m_GradientDirections;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAdcImageFilter.txx"
#endif

#endif //__itkAdcImageFilter_h_

