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

#ifndef __itkStreamlineTrackingFilter_txx
#define __itkStreamlineTrackingFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkStreamlineTrackingFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

//#define QBALL_RECON_PI       M_PI

template< class TTensorPixelType, class TPDPixelType>
StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::StreamlineTrackingFilter():
    m_FaThres(0.2)
{
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::BeforeThreadedGenerateData()
{
    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_Points = vtkPoints::New();
    m_Cells = vtkCellArray::New();
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int )
{

    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typedef ImageRegionConstIterator< InputImageType >  InputIteratorType;
    typedef typename InputImageType::PixelType          InputTensorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    InputIteratorType it(inputImagePointer, outputRegionForThread );
    it.GoToBegin();
    while( !it.IsAtEnd() )
    {
        InputTensorType b = it.Get();
        TensorType tensor = b.GetDataPointer();


        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        typename TensorType::EigenValuesArrayType eigenvalues;
        typename TensorType::EigenVectorsMatrixType eigenvectors;
        if(tensor.GetTrace()!=0 && tensor.GetFractionalAnisotropy()>m_FaThres)
        {
            typename InputImageType::IndexType index = it.GetIndex();
            tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);

            int eIndex = 2;
            if( (eigenvalues[0] >= eigenvalues[1]) && (eigenvalues[0] >= eigenvalues[2]) )
              eIndex = 0;
            else if(eigenvalues[1] >= eigenvalues[2])
              eIndex = 1;

            vnl_vector_fixed<double,3> vec;
            vec[0] = eigenvectors(eIndex, 0);
            vec[1] = eigenvectors(eIndex, 1);
            vec[2] = eigenvectors(eIndex, 2);

            m_Cells->InsertNextCell(container);
        }
        ++it; // image iterator
    }

    std::cout << "One Thread finished tracking" << std::endl;
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::AfterThreadedGenerateData()
{
    m_FiberPolyData->SetPoints(m_Points);
    m_FiberPolyData->SetLines(m_Cells);
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
}

}
#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
