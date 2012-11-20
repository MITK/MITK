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

#include <math.h>

template< class ScalarType >
GibbsRingingArtifact< ScalarType >::GibbsRingingArtifact()
    : m_KspaceCropping(0.1)
{

}

template< class ScalarType >
GibbsRingingArtifact< ScalarType >::~GibbsRingingArtifact()
{

}

template< class ScalarType >
void GibbsRingingArtifact< ScalarType >::AddArtifact(typename ComplexSliceType::Pointer slice)
{
    itk::ImageRegion<2> region = slice->GetLargestPossibleRegion();
    itk::ImageRegionIterator<ComplexSliceType> it(slice, region);

    double x = (double)region.GetSize()[0]/2;
    double y = (double)region.GetSize()[1]/2;

    double radius = std::min(x,y);
    radius *= (1-m_KspaceCropping);

    if (region.GetSize()[0]%2 == 0)
        x -= 0.5;
    if (region.GetSize()[1]%2 == 0)
        y -= 0.5;

//    int x = std::ceil((double)region.GetSize()[0]*m_KspaceCropping);
//    int y = std::ceil((double)region.GetSize()[1]*m_KspaceCropping);
    while(!it.IsAtEnd())
    {
        typename ComplexSliceType::IndexType idx = it.GetIndex();
        double dist = sqrt((idx[0]-x)*(idx[0]-x)+(idx[1]-y)*(idx[1]-y));
        if (dist>radius)
            it.Set(0);
//        if (idx[0]<x)
//            it.Set(vcl_complex<double>(0,0));
//        if (idx[0]>region.GetSize()[0]-x)
//            it.Set(0);
//        if (idx[1]>region.GetSize()[1]-y)
//            it.Set(0);
//        if (idx[1]<y)
//            it.Set(0);
        ++it;
    }
}
