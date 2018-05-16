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
#ifndef __itkVectorImageToImageFilter_h_
#define __itkVectorImageToImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"

namespace itk{
/** \class VectorImageToImageFilter
 * \brief This class extracts an image out of a vectorimage
 *
 * An image is extracted at a designated index of a vectorimage. If non index is set, it is an index of 0 used.
 */

template< class TPixelType >
class VectorImageToImageFilter :
        public ImageToImageFilter< VectorImage < TPixelType, 3 >, Image < TPixelType, 3 > >
{

public:

    typedef VectorImageToImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage < TPixelType, 3 >, Image < TPixelType, 3 > >  Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(VectorImageToImageFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    itkSetMacro( Index, unsigned int)

    protected:
    VectorImageToImageFilter();
    ~VectorImageToImageFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const override;

    //void BeforeThreadedGenerateData();
    virtual void GenerateData() override;

    unsigned int m_Index;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVectorImageToImageFilter.txx"
#endif

#endif //__itkVectorImageToImageFilter_h_
