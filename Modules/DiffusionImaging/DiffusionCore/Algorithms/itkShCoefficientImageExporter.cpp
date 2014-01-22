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
#ifndef __itkShCoefficientImageExporter_cpp
#define __itkShCoefficientImageExporter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkShCoefficientImageExporter.h"
#include <itkImageRegionIterator.h>
#include <boost/math/special_functions.hpp>

using namespace boost::math;

namespace itk {

template< class PixelType, int ShOrder >
ShCoefficientImageExporter< PixelType, ShOrder >::ShCoefficientImageExporter()
{

}

template< class PixelType, int ShOrder >
void ShCoefficientImageExporter< PixelType, ShOrder >
::GenerateData()
{
    if (m_InputImage.IsNull())
        return;

    Vector<double, 4> spacing4;
    Point<float, 4> origin4;
    Matrix<double, 4, 4> direction4; direction4.SetIdentity();
    ImageRegion<4> imageRegion4;

    Vector<double, 3> spacing3 = m_InputImage->GetSpacing();
    Point<float, 3> origin3 = m_InputImage->GetOrigin();
    Matrix<double, 3, 3> direction3 = m_InputImage->GetDirection();
    ImageRegion<3> imageRegion3 = m_InputImage->GetLargestPossibleRegion();

    spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
    origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin4[3] = 0;
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
            direction4[r][c] = direction3[r][c];

    imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
    imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
    imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
    imageRegion4.SetSize(3, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder );

    m_OutputImage = CoefficientImageType::New();
    m_OutputImage->SetSpacing( spacing4 );
    m_OutputImage->SetOrigin( origin4 );
    m_OutputImage->SetDirection( direction4 );
    m_OutputImage->SetRegions( imageRegion4 );
    m_OutputImage->Allocate();
    m_OutputImage->FillBuffer(0.0);

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    InputIteratorType it(m_InputImage, m_InputImage->GetLargestPossibleRegion());
    int numCoeffs = imageRegion4.GetSize(3);

    while(!it.IsAtEnd())
    {
        CoefficientImageType::IndexType index;
        index[0] = it.GetIndex()[0];
        index[1] = it.GetIndex()[1];
        index[2] = it.GetIndex()[2];

        for (int i=0; i<numCoeffs; i++)
        {
            index[3] = i;
            m_OutputImage->SetPixel(index, it.Get()[i]);
        }

        ++it;
    }
}

}

#endif // __itkShCoefficientImageExporter_cpp
