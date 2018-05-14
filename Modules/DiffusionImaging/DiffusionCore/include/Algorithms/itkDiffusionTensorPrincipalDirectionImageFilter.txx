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

#ifndef __itkDiffusionTensorPrincipalDirectionImageFilter_txx
#define __itkDiffusionTensorPrincipalDirectionImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkDiffusionTensorPrincipalDirectionImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkArray.h"
#include "vnl/vnl_vector.h"
#include <boost/progress.hpp>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk {

template< class TTensorPixelType>
DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>::DiffusionTensorPrincipalDirectionImageFilter()
    : m_NormalizeVectors(true)
    , m_UsePolarCoordinates(false)
    , m_FaThreshold(0.2)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TTensorPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    Vector<double,3> spacing3 = inputImagePointer->GetSpacing();
    mitk::Point3D origin3 = inputImagePointer->GetOrigin();
    itk::Matrix<double, 3, 3> direction3 = inputImagePointer->GetDirection();
    ImageRegion<3> imageRegion3 = inputImagePointer->GetLargestPossibleRegion();

    if (m_MaskImage.IsNull())
    {
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing3 );
        m_MaskImage->SetOrigin( origin3 );
        m_MaskImage->SetDirection( direction3 );
        m_MaskImage->SetRegions( imageRegion3 );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }

    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetSpacing( spacing3 );
    outputImage->SetOrigin( origin3 );
    outputImage->SetDirection( direction3 );
    outputImage->SetRegions( imageRegion3 );
    outputImage->Allocate();
    outputImage->FillBuffer(0);
    this->SetNthOutput(0, outputImage);


    itk::Vector<double, 4> spacing4;
    itk::Point<float, 4> origin4;
    itk::Matrix<double, 4, 4> direction4;
    itk::ImageRegion<4> imageRegion4;

    spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
    origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin3[3] = 0;
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
        direction4[r][c] = direction3[r][c];
    direction4[3][3] = 1;
    imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
    imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
    imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
    imageRegion4.SetSize(3, 3);

    m_PeakImage = PeakImageType::New();
    m_PeakImage->SetSpacing( spacing4 );
    m_PeakImage->SetOrigin( origin4 );
    m_PeakImage->SetDirection( direction4 );
    m_PeakImage->SetRegions( imageRegion4 );
    m_PeakImage->Allocate();
    m_PeakImage->FillBuffer(0.0);
}

template< class TTensorPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>
::AfterThreadedGenerateData()
{

}

template< class TTensorPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{

    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typedef ImageRegionConstIterator< InputImageType >  InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

    ImageRegionIterator< OutputImageType > numDirectionsIterator(outputImage, outputRegionForThread);
    InputIteratorType tensorIterator(inputImagePointer, outputRegionForThread );

    while( !tensorIterator.IsAtEnd() )
    {
        typename InputImageType::IndexType index = tensorIterator.GetIndex();
        if (m_MaskImage->GetPixel(index)==0)
        {
            ++tensorIterator;
            ++numDirectionsIterator;
            continue;
        }

        typename InputImageType::PixelType b = tensorIterator.Get();
        TensorType tensor = b.GetDataPointer();

        typename PeakImageType::IndexType peakIndex;
        peakIndex[0] = tensorIterator.GetIndex()[0];
        peakIndex[1] = tensorIterator.GetIndex()[1];
        peakIndex[2] = tensorIterator.GetIndex()[2];

        typename TensorType::EigenValuesArrayType eigenvalues;
        typename TensorType::EigenVectorsMatrixType eigenvectors;
        if(tensor.GetTrace()!=0)
        {
            tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);

            vnl_vector_fixed<float,3> vec;
            vec[0] = eigenvectors(2,0);
            vec[1] = eigenvectors(2,1);
            vec[2] = eigenvectors(2,2);
            vec.normalize();

            vnl_vector_fixed<float,3> out; out.fill(0);

            float fa = tensor.GetFractionalAnisotropy();
            if (fa<m_FaThreshold)
              vec.fill(0.0);
            else
            {
              if (!m_NormalizeVectors)
                vec *= fa;

              if (m_UsePolarCoordinates)
              {
                if(vec[0] || vec[1] || vec[2])
                {
                  out[0] = sqrt( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
                  out[1] = atan2( vec[1], vec[0] );
                  out[2] = 0.5*itk::Math::pi - atan( vec[2] / sqrt( vec[0] * vec[0] + vec[1] * vec[1] ) );

                  if(out[1]>itk::Math::pi)
                  {
                    out[1] = out[1] - itk::Math::pi;
                  }
                }
                else
                {
                  out[0] = 0;
                  out[1] = 0;
                  out[2] = 0;
                }
              }
              else
              {
                out = vec;
              }
            }

            peakIndex[3] = 0;
            m_PeakImage->SetPixel(peakIndex, out[0]);
            peakIndex[3] = 1;
            m_PeakImage->SetPixel(peakIndex, out[1]);
            peakIndex[3] = 2;
            m_PeakImage->SetPixel(peakIndex, out[2]);

            numDirectionsIterator.Set( 1 );
        }

        ++numDirectionsIterator;
        ++tensorIterator;
    }

    std::cout << "One Thread finished extraction" << std::endl;
}

template< class TTensorPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>
::PrintSelf(std::ostream& , Indent ) const
{
}

}
#endif
