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
#ifndef __itkShCoefficientImageImporter_cpp
#define __itkShCoefficientImageImporter_cpp

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "itkShCoefficientImageImporter.h"
#include <itkImageRegionIterator.h>
#include <mitkDiffusionFunctionCollection.h>

namespace itk {

template< class PixelType, int ShOrder >
ShCoefficientImageImporter< PixelType, ShOrder >::ShCoefficientImageImporter()
{

}

template< class PixelType, int ShOrder >
void ShCoefficientImageImporter< PixelType, ShOrder >
::GenerateData()
{
  if (m_InputImage.IsNull())
    return;

  Vector<float, 4> spacing4 = m_InputImage->GetSpacing();
  Point<float, 4> origin4 = m_InputImage->GetOrigin();
  Matrix<double, 4, 4> direction4 = m_InputImage->GetDirection();
  ImageRegion<4> imageRegion4 = m_InputImage->GetLargestPossibleRegion();

  Vector<double, 3> spacing3;
  Point<float, 3> origin3;
  Matrix<double, 3, 3> direction3;
  ImageRegion<3> imageRegion3;

  spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
  origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction3[r][c] = direction4[r][c];
  imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
  imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
  imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

  m_CoefficientImage = CoefficientImageType::New();
  m_CoefficientImage->SetSpacing( spacing3 );
  m_CoefficientImage->SetOrigin( origin3 );
  m_CoefficientImage->SetDirection( direction3 );
  m_CoefficientImage->SetRegions( imageRegion3 );
  m_CoefficientImage->Allocate();
  Vector< PixelType, (ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder > nullVec2; nullVec2.Fill(0.0);
  m_CoefficientImage->FillBuffer(nullVec2);

  int x = imageRegion4.GetSize(0);
  int y = imageRegion4.GetSize(1);
  int z = imageRegion4.GetSize(2);
  int numCoeffs = imageRegion4.GetSize(3);

#ifdef WIN32
#pragma omp parallel for
#else
#pragma omp parallel for collapse(3)
#endif
  for (int a=0; a<x; a++)
    for (int b=0; b<y; b++)
      for (int c=0; c<z; c++)
      {
        typename InputImageType::IndexType index;
        index.SetElement(0,a);
        index.SetElement(1,b);
        index.SetElement(2,c);
        typename CoefficientImageType::PixelType pix;
        for (int d=0; d<numCoeffs; d++)
        {
          index.SetElement(3,d);
          pix[d] = m_InputImage->GetPixel(index);
        }
        typename CoefficientImageType::IndexType index2;
        index2.SetElement(0,a);
        index2.SetElement(1,b);
        index2.SetElement(2,c);
        m_CoefficientImage->SetPixel(index2, pix);
      }
}

}

#endif // __itkShCoefficientImageImporter_cpp
