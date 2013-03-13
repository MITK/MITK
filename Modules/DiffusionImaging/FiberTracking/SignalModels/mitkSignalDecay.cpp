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
SignalDecay< ScalarType >::SignalDecay()
{

}

template< class ScalarType >
SignalDecay< ScalarType >::~SignalDecay()
{

}

template< class ScalarType >
typename SignalDecay< ScalarType >::ComplexSliceType::Pointer SignalDecay< ScalarType >::AddArtifact(typename ComplexSliceType::Pointer slice)
{
    itk::ImageRegion<2> region = slice->GetLargestPossibleRegion();

    double dt = this->m_LineReadoutTime/region.GetSize(0);

    double from90 = this->m_TE - this->m_LineReadoutTime*region.GetSize(1)/2;
    double fromMaxEcho = - this->m_LineReadoutTime*region.GetSize(1)/2;

    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            typename ComplexSliceType::IndexType idx;
            idx[0]=x; idx[1]=y;
            std::complex< double > pix = slice->GetPixel(idx);

            double fact = exp(-from90/this->m_T2 -fabs(fromMaxEcho)/this->m_Tinhom);
            std::complex< double > newPix(fact*pix.real(), fact*pix.imag());
            slice->SetPixel(idx, newPix);

            from90 += dt;
            fromMaxEcho += dt;
        }
    return slice;
}
