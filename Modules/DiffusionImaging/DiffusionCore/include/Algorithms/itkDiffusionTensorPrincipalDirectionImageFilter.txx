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

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

//#define QBALL_RECON_PI       M_PI

template< class TTensorPixelType>
DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType>::DiffusionTensorPrincipalDirectionImageFilter()
    : m_NormalizeVectors(true)
    , m_MaxEigenvalue(0.0)
    , m_UsePolarCoordinates(false)
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
    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    typename OutputImageType::Pointer numDirImage = static_cast< OutputImageType* >( this->ProcessObject::GetPrimaryOutput() );
    ImageRegionConstIterator< OutputImageType > it(numDirImage, numDirImage->GetLargestPossibleRegion() );

    mitk::Vector3D spacing = numDirImage->GetSpacing();
    double minSpacing = spacing[0];
    if (spacing[1]<minSpacing)
        minSpacing = spacing[1];
    if (spacing[2]<minSpacing)
        minSpacing = spacing[2];

    while( !it.IsAtEnd() )
    {
        typename OutputImageType::IndexType index = it.GetIndex();
        if (m_MaskImage->GetPixel(index)==0)
        {
            ++it;
            continue;
        }

        typename PeakImageType::IndexType peakIndex;
        peakIndex[0] = it.GetIndex()[0];
        peakIndex[1] = it.GetIndex()[1];
        peakIndex[2] = it.GetIndex()[2];
        DirectionType dir;
        peakIndex[3] = 0;
        dir[0] = m_PeakImage->GetPixel(peakIndex);
        peakIndex[3] = 1;
        dir[1] = m_PeakImage->GetPixel(peakIndex);
        peakIndex[3] = 2;
        dir[2] = m_PeakImage->GetPixel(peakIndex);

        if (!m_NormalizeVectors && m_MaxEigenvalue>0 && !m_UsePolarCoordinates)
            dir /= m_MaxEigenvalue;

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        itk::ContinuousIndex<double, 3> center;
        center[0] = index[0];
        center[1] = index[1];
        center[2] = index[2];
        itk::Point<double> worldCenter;
        numDirImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

        itk::Point<double> worldStart;
        worldStart[0] = worldCenter[0]-dir[0]/2 * minSpacing;
        worldStart[1] = worldCenter[1]-dir[1]/2 * minSpacing;
        worldStart[2] = worldCenter[2]-dir[2]/2 * minSpacing;
        vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
        container->GetPointIds()->InsertNextId(id);
        itk::Point<double> worldEnd;
        worldEnd[0] = worldCenter[0]+dir[0]/2 * minSpacing;
        worldEnd[1] = worldCenter[1]+dir[1]/2 * minSpacing;
        worldEnd[2] = worldCenter[2]+dir[2]/2 * minSpacing;
        id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
        container->GetPointIds()->InsertNextId(id);
        m_VtkCellArray->InsertNextCell(container);

        ++it;
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundle::New(directionsPolyData);
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

            if (!m_NormalizeVectors)
                vec *= eigenvalues[2];

            if (eigenvalues[2]>m_MaxEigenvalue)
                m_MaxEigenvalue = eigenvalues[2];

            vnl_vector_fixed<float,3> out;
            if (m_UsePolarCoordinates)
            {
              if(vec[0] || vec[1] || vec[2])
              {
                out[0] = sqrt( vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] );
                out[1] = atan2( vec[1], vec[0] );
                out[2] = 0.5*M_PI - atan( vec[2] / sqrt( vec[0] * vec[0] + vec[1] * vec[1] ) );

                if(out[1]>M_PI)
                {
                  out[1] = out[1] - M_PI;
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
#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
