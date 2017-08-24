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
#ifndef __itkNormalizeImageSlicesFilter_h_
#define __itkNormalizeImageSlicesFilter_h_

#include <itkImageToImageFilter.h>

namespace itk{

/**
* \brief Normalize slices along given axis to 0 mean and 1 std. */

template< class TPixelType >
class NormalizeImageSlicesFilter :
        public ImageToImageFilter< Image<  TPixelType, 3 >, Image< float, 3 > >
{

public:

    typedef NormalizeImageSlicesFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< TPixelType, 3 >, Image< float, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(NormalizeImageSlicesFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;

    itkSetMacro( Axis, unsigned int )

    virtual void Update() override{
      this->GenerateData();
    }

protected:
    NormalizeImageSlicesFilter();
    ~NormalizeImageSlicesFilter() {}

    void GenerateData() override;

private:

    unsigned int  m_Axis;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNormalizeImageSlicesFilter.cpp"
#endif

#endif //__itkNormalizeImageSlicesFilter_h_

