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

template< class TTensorPixelType, class TPDPixelType>
DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType,
TPDPixelType>
::DiffusionTensorPrincipalDirectionImageFilter()
    : m_NormalizeVectors(true)
    , m_MaxEigenvalue(0.0)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TTensorPixelType,
          class TPDPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType,
TPDPixelType>
::BeforeThreadedGenerateData()
{
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    Vector<double,3> spacing = inputImagePointer->GetSpacing();
    mitk::Point3D origin = inputImagePointer->GetOrigin();
    itk::Matrix<double, 3, 3> direction = inputImagePointer->GetDirection();
    ImageRegion<3> imageRegion = inputImagePointer->GetLargestPossibleRegion();

    if (m_MaskImage.IsNull())
    {
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing );
        m_MaskImage->SetOrigin( origin );
        m_MaskImage->SetDirection( direction );
        m_MaskImage->SetRegions( imageRegion );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing );
    m_NumDirectionsImage->SetOrigin( origin );
    m_NumDirectionsImage->SetDirection( direction );
    m_NumDirectionsImage->SetRegions( imageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    itk::Vector< TPDPixelType, 3 > nullVec; nullVec.Fill(0.0);
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetSpacing( spacing );
    outputImage->SetOrigin( origin );
    outputImage->SetDirection( direction );
    outputImage->SetRegions( imageRegion );
    outputImage->Allocate();
    outputImage->FillBuffer(nullVec);
    this->SetNthOutput(0, outputImage);
}

template< class TTensorPixelType,
          class TPDPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType,
TPDPixelType>
::AfterThreadedGenerateData()
{
    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    typename OutputImageType::Pointer directionImage = static_cast< OutputImageType* >( this->ProcessObject::GetPrimaryOutput() );
    ImageRegionConstIterator< OutputImageType > it(directionImage, directionImage->GetLargestPossibleRegion() );

    mitk::Vector3D spacing = directionImage->GetSpacing();
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

        itk::Vector< float, 3 > pixel = directionImage->GetPixel(index);
        DirectionType dir; dir[0] = pixel[0]; dir[1] = pixel[1]; dir[2] = pixel[2];

        if (!m_NormalizeVectors && m_MaxEigenvalue>0)
            dir /= m_MaxEigenvalue;

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        itk::ContinuousIndex<double, 3> center;
        center[0] = index[0];
        center[1] = index[1];
        center[2] = index[2];
        itk::Point<double> worldCenter;
        directionImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

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
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);
}

template< class TTensorPixelType,
          class TPDPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType,
TPDPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{

    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typedef ImageRegionConstIterator< InputImageType >  InputIteratorType;
    typename InputImageType::Pointer inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

    ImageRegionIterator< OutputImageType > outIt(outputImage, outputRegionForThread);
    InputIteratorType inIt(inputImagePointer, outputRegionForThread );
    ImageRegionIterator< ItkUcharImgType > nIt(m_NumDirectionsImage, outputRegionForThread );

    while( !inIt.IsAtEnd() )
    {
        typename InputImageType::IndexType index = inIt.GetIndex();
        if (m_MaskImage->GetPixel(index)==0)
        {
            ++inIt;
            ++nIt;
            ++outIt;
            continue;
        }

        typename InputImageType::PixelType b = inIt.Get();
        TensorType tensor = b.GetDataPointer();

        typename OutputImageType::PixelType dir;
        typename TensorType::EigenValuesArrayType eigenvalues;
        typename TensorType::EigenVectorsMatrixType eigenvectors;
        if(tensor.GetTrace()!=0)
        {
            tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);

            vnl_vector_fixed<double,3> vec;
            vec[0] = eigenvectors(2,0);
            vec[1] = eigenvectors(2,1);
            vec[2] = eigenvectors(2,2);

            if (!m_NormalizeVectors)
                vec *= eigenvalues[2];

            if (eigenvalues[2]>m_MaxEigenvalue)
                m_MaxEigenvalue = eigenvalues[2];

            dir[0] = (TPDPixelType)vec[0];
            dir[1] = (TPDPixelType)vec[1];
            dir[2] = (TPDPixelType)vec[2];

            outIt.Set( dir );
            m_NumDirectionsImage->SetPixel(index, 1);
        }

        ++outIt;
        ++inIt;
        ++nIt;
    }

    std::cout << "One Thread finished extraction" << std::endl;
}

template< class TTensorPixelType,
          class TPDPixelType>
void DiffusionTensorPrincipalDirectionImageFilter< TTensorPixelType,
TPDPixelType>
::PrintSelf(std::ostream& os, Indent indent) const
{
}

}
#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
