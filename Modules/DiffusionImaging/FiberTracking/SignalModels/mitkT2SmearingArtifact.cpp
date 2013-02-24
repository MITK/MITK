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
    : m_UseT1(true)
    , m_UseT2(true)
    , m_UseT2star(true)
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
    double dt = (double)this->m_TE/(region.GetSize(0)*region.GetSize(1));

    double from90 = (double)this->m_TE/2;
    double from180 = -(double)this->m_TE/2;

    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            typename ComplexSliceType::IndexType idx;
            idx[0]=x; idx[1]=y;
            std::complex< double > pix = slice->GetPixel(idx);
            double fact = 0;

            if (m_UseT1)
                fact -= from90/this->m_T1;
            if (m_UseT2)
                fact -= from90/this->m_T2;
            if (m_UseT2star)
                fact -= fabs(from180)/this->m_T2star;
            fact = exp(fact);

            std::complex< double > newPix(fact*pix.real(), fact*pix.imag());
            slice->SetPixel(idx, newPix);

            from90 += dt;
            from180 += dt;
        }
    return slice;
}
