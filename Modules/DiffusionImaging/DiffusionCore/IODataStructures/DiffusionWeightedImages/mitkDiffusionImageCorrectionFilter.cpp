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


#ifndef MITKDIFFUSIONIMAGECORRECTIONFILTER_CPP
#define MITKDIFFUSIONIMAGECORRECTIONFILTER_CPP

#include "mitkDiffusionImageCorrectionFilter.h"

#include <vnl/algo/vnl_symmetric_eigensystem.h>
#include <vnl/vnl_inverse.h>

template< typename TPixelType >
mitk::DiffusionImageCorrectionFilter<TPixelType>::DiffusionImageCorrectionFilter()
{

}

template< typename TPixelType >
typename mitk::DiffusionImageCorrectionFilter<TPixelType>::TransformMatrixType
mitk::DiffusionImageCorrectionFilter<TPixelType>
::GetRotationComponent(const TransformMatrixType &A)
{
  TransformMatrixType B;

  B = A * A.transpose();

  // get the eigenvalues and eigenvectors
  typedef double MType;
  vnl_vector< MType > eigvals;
  vnl_matrix< MType > eigvecs;
  vnl_symmetric_eigensystem_compute< MType > ( B, eigvecs, eigvals );

  vnl_matrix_fixed< MType, 3, 3 > eigvecs_fixed;
  eigvecs_fixed.set_columns(0, eigvecs );

  TransformMatrixType C;
  C.set_identity();

  vnl_vector_fixed< MType, 3 > eigvals_sqrt;
  for(unsigned int i=0; i<3; i++)
  {
    C(i,i) = std::sqrt( eigvals[i] );
  }

  TransformMatrixType S = vnl_inverse( eigvecs_fixed * C * vnl_inverse( eigvecs_fixed )) * A;

  return S;
}

template< typename TPixelType >
void mitk::DiffusionImageCorrectionFilter<TPixelType>
::CorrectDirections( const TransformsVectorType& transformations)
{
  if( m_SourceImage.IsNull() )
  {
    mitkThrow() << " No diffusion image given! ";
  }

  GradientDirectionContainerPointerType directions = m_SourceImage->GetDirections();
  GradientDirectionContainerPointerType corrected_directions =
      GradientDirectionContainerType::New();

  unsigned int transformed = 0;
  for(size_t i=0; i< directions->Size(); i++ )
  {

    // skip b-zero images
    if( directions->ElementAt(i).one_norm() <= 0.0 )
    {
      corrected_directions->push_back( directions->ElementAt(i) );
      continue;
    }

    GradientDirectionType corrected = GetRotationComponent(
                                          transformations.at(transformed))
                                        * directions->ElementAt(i);
    // store the corrected direction
    corrected_directions->push_back( corrected );
    transformed++;
  }

  // replace the old directions with the corrected ones
  m_SourceImage->SetDirections( corrected_directions );

}

#endif
