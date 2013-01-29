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
template< class ScalarType >
T2SmearingArtifact< ScalarType >::T2SmearingArtifact()
    : m_ReadoutPulseLength(1)
{

}

template< class ScalarType >
T2SmearingArtifact< ScalarType >::~T2SmearingArtifact()
{

}

template< class ScalarType >
typename T2SmearingArtifact< ScalarType >::ComplexSliceType::Pointer T2SmearingArtifact< ScalarType >::AddArtifact(typename ComplexSliceType::Pointer slice)
{
    itk::ImageRegion<2> region = slice->GetLargestPossibleRegion();
    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            typename ComplexSliceType::IndexType idx;
            idx[0]=x; idx[1]=y;
            std::complex< double > pix = slice->GetPixel(idx);

            double t = m_ReadoutPulseLength*(y+(double)x/region.GetSize(0));
            std::complex< double > newPix(pix.real()*exp(-t/this->m_T2), pix.imag()*exp(-t/this->m_T2));
            slice->SetPixel(idx, newPix);
        }
    return slice;
}
