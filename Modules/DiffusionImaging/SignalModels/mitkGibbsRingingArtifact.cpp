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
typename GibbsRingingArtifact< ScalarType >::ComplexSliceType::Pointer GibbsRingingArtifact< ScalarType >::AddArtifact(typename ComplexSliceType::Pointer slice)
{
    itk::ImageRegion<2> region = slice->GetLargestPossibleRegion();

    int x = region.GetSize()[0]/m_KspaceCropping;
    int y = region.GetSize()[1]/m_KspaceCropping;

    typename ComplexSliceType::Pointer newSlice = ComplexSliceType::New();
    itk::ImageRegion<2> newRegion;
    newRegion.SetSize(0, x);
    newRegion.SetSize(1, y);

    newSlice->SetLargestPossibleRegion( newRegion );
    newSlice->SetBufferedRegion( newRegion );
    newSlice->SetRequestedRegion( newRegion );
    newSlice->Allocate();

    itk::ImageRegionIterator<ComplexSliceType> it(newSlice, newRegion);
    while(!it.IsAtEnd())
    {
        typename ComplexSliceType::IndexType idx;
        idx[0] = region.GetSize()[0]/2-x/2+it.GetIndex()[0];
        idx[1] = region.GetSize()[1]/2-y/2+it.GetIndex()[1];

        it.Set(slice->GetPixel(idx));

        ++it;
    }
    return newSlice;
}
