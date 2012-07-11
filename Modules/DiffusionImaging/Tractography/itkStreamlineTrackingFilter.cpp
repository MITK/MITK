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
    m_FaThreshold(0.2),
    m_StepSize(1),
    m_MaxLength(10000),
    m_SeedsPerVoxel(1),
    m_AngularThreshold(0.7),
    m_F(1.0),
    m_G(0.0)
{
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
}

template< class TTensorPixelType,
          class TPDPixelType>
double StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::RoundToNearest(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::BeforeThreadedGenerateData()
{
    m_FiberPolyData = FiberPolyDataType::New();
    m_Points = vtkPoints::New();
    m_Cells = vtkCellArray::New();

    typename InputImageType::Pointer inputImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    m_ImageSize.resize(3);
    m_ImageSize[0] = inputImage->GetLargestPossibleRegion().GetSize()[0];
    m_ImageSize[1] = inputImage->GetLargestPossibleRegion().GetSize()[1];
    m_ImageSize[2] = inputImage->GetLargestPossibleRegion().GetSize()[2];
    m_ImageSpacing.resize(3);
    m_ImageSpacing[0] = inputImage->GetSpacing()[0];
    m_ImageSpacing[1] = inputImage->GetSpacing()[1];
    m_ImageSpacing[2] = inputImage->GetSpacing()[2];

    if (m_StepSize<0.005)
    {
        float minSpacing;
        if(m_ImageSpacing[0]<m_ImageSpacing[1] && m_ImageSpacing[0]<m_ImageSpacing[2])
            minSpacing = m_ImageSpacing[0];
        else if (m_ImageSpacing[1] < m_ImageSpacing[2])
            minSpacing = m_ImageSpacing[1];
        else
            minSpacing = m_ImageSpacing[2];
        m_StepSize = 0.1*minSpacing;
    }

    m_PolyDataContainer = itk::VectorContainer< int, FiberPolyDataType >::New();
    for (int i=0; i<this->GetNumberOfThreads(); i++)
    {
        FiberPolyDataType poly = FiberPolyDataType::New();
        m_PolyDataContainer->InsertElement(i, poly);
    }

    if (m_MaskImage.IsNull())
    {
        itk::Vector<double, 3> spacing = inputImage->GetSpacing();
        itk::Point<double, 3> origin = inputImage->GetOrigin();
        itk::Matrix<double, 3, 3> direction = inputImage->GetDirection();
        ImageRegion<3> imageRegion = inputImage->GetLargestPossibleRegion();

        // initialize crossings image
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing );
        m_MaskImage->SetOrigin( origin );
        m_MaskImage->SetDirection( direction );
        m_MaskImage->SetRegions( imageRegion );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);

    }
    std::cout << "StreamlineTrackingFilter: Angular threshold: " << m_AngularThreshold << std::endl;
    std::cout << "StreamlineTrackingFilter: FA threshold: " << m_FaThreshold << std::endl;
    std::cout << "StreamlineTrackingFilter: stepsize: " << m_StepSize << " mm" << std::endl;
    std::cout << "StreamlineTrackingFilter: starting streamline tracking" << std::endl;
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
    FiberPolyDataType poly = m_PolyDataContainer->GetElement(threadId);
    vtkSmartPointer<vtkPoints> Points = vtkPoints::New();
    vtkSmartPointer<vtkCellArray> Cells = vtkCellArray::New();

    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typedef ImageRegionConstIterator< InputImageType >  InputIteratorType;
    typedef ImageRegionConstIterator< ItkUcharImgType >  MaskIteratorType;
    typedef typename InputImageType::PixelType          InputTensorType;
    typename InputImageType::Pointer inputImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    InputIteratorType it(inputImage, outputRegionForThread );
    MaskIteratorType mit(m_MaskImage, outputRegionForThread );
    it.GoToBegin();
    mit.GoToBegin();
    while( !it.IsAtEnd() )
    {
        if (mit.Value()==0)
        {
            ++mit;
            ++it;
            continue;
        }

        typename TensorType::EigenValuesArrayType eigenvalues;
        typename TensorType::EigenVectorsMatrixType eigenvectors;

        for (int s=0; s<m_SeedsPerVoxel; s++)
        {
            unsigned long counter = 0;
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            std::vector< vtkIdType > pointISs;
            typename InputImageType::IndexType index = it.GetIndex();
            itk::ContinuousIndex<double, 3> pos;
            itk::ContinuousIndex<double, 3> start;

            if (m_SeedsPerVoxel>1)
            {
                pos[0] = index[0]+(double)(rand()%99-49)/100;
                pos[1] = index[1]+(double)(rand()%99-49)/100;
                pos[2] = index[2]+(double)(rand()%99-49)/100;
            }
            else
            {
                pos[0] = index[0];
                pos[1] = index[1];
                pos[2] = index[2];
            }
            start = pos;

            int step = 0;
            vnl_vector_fixed<double,3> dirOld; dirOld.fill(0.0);
            // do forward tracking
            while (step < m_MaxLength)
            {
                ++step;

                index[0] = RoundToNearest(pos[0]);
                index[1] = RoundToNearest(pos[1]);
                index[2] = RoundToNearest(pos[2]);

                if (!inputImage->GetLargestPossibleRegion().IsInside(index))
                    break;

                typename InputImageType::PixelType tensor = inputImage->GetPixel(index);
                if(tensor.GetTrace()!=0 && tensor.GetFractionalAnisotropy()>m_FaThreshold)
                {
                    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
                    vnl_vector_fixed<double,3> dir;
                    dir[0] = eigenvectors(2, 0);
                    dir[1] = eigenvectors(2, 1);
                    dir[2] = eigenvectors(2, 2);
                    dir.normalize();

                    if (!dirOld.is_zero())
                    {
                        dir[0] = m_F*dir[0] + (1-m_F)*( (1-m_G)*dirOld[0] + m_G*(tensor[0]*dirOld[0] + tensor[1]*dirOld[1] + tensor[2]*dirOld[2]));
                        dir[1] = m_F*dir[1] + (1-m_F)*( (1-m_G)*dirOld[1] + m_G*(tensor[1]*dirOld[0] + tensor[3]*dirOld[1] + tensor[4]*dirOld[2]));
                        dir[2] = m_F*dir[2] + (1-m_F)*( (1-m_G)*dirOld[2] + m_G*(tensor[2]*dirOld[0] + tensor[4]*dirOld[1] + tensor[5]*dirOld[2]));
                        dir.normalize();

                        float angle = dot_product(dirOld, dir);
                        if (angle<0)
                            dir *= -1;
                        angle = fabs(dot_product(dirOld, dir));
                        if (angle<m_AngularThreshold)
                            break;
                    }

                    dirOld = dir;

                    dir *= m_StepSize;

                    itk::Point<double> worldPos;
                    inputImage->TransformContinuousIndexToPhysicalPoint( pos, worldPos );

                    vtkIdType id = Points->InsertNextPoint(worldPos.GetDataPointer());
                    pointISs.push_back(id);
                    counter++;

                    pos[0] += dir[0]/m_ImageSpacing[0];
                    pos[1] += dir[1]/m_ImageSpacing[1];
                    pos[2] += dir[2]/m_ImageSpacing[2];
                }
            }

            // insert reverse IDs
            while (!pointISs.empty())
            {
                container->GetPointIds()->InsertNextId(pointISs.back());
                pointISs.pop_back();
            }

            // do backward tracking
            index = it.GetIndex();
            pos = start;
            dirOld.fill(0.0);
            while (step < m_MaxLength)
            {
                ++step;

                index[0] = RoundToNearest(pos[0]);
                index[1] = RoundToNearest(pos[1]);
                index[2] = RoundToNearest(pos[2]);

                if (index[0] < 0 || index[0]>=m_ImageSize[0])
                    break;
                if (index[1] < 0 || index[1]>=m_ImageSize[1])
                    break;
                if (index[2] < 0 || index[2]>=m_ImageSize[2])
                    break;

                typename InputImageType::PixelType tensor = inputImage->GetPixel(index);
                if(tensor.GetTrace()!=0 && tensor.GetFractionalAnisotropy()>m_FaThreshold)
                {
                    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);

                    vnl_vector_fixed<double,3> dir;
                    dir[0] = eigenvectors(2, 0);
                    dir[1] = eigenvectors(2, 1);
                    dir[2] = eigenvectors(2, 2);
                    dir.normalize();
                    dir *= -1; // reverse direction

                    if (!dirOld.is_zero())
                    {
                        float angle = dot_product(dirOld, dir);
                        if (angle<0)
                            dir *= -1;
                        angle = fabs(dot_product(dirOld, dir));
                        if (angle<0.7)
                            break;
                    }
                    dirOld = dir;

                    dir *= m_StepSize;

                    itk::Point<double> worldPos;
                    inputImage->TransformContinuousIndexToPhysicalPoint( pos, worldPos );

                    vtkIdType id = Points->InsertNextPoint(worldPos.GetDataPointer());
                    container->GetPointIds()->InsertNextId(id);
                    counter++;

                    pos[0] += dir[0]/m_ImageSpacing[0];
                    pos[1] += dir[1]/m_ImageSpacing[1];
                    pos[2] += dir[2]/m_ImageSpacing[2];
                }
            }

            if (counter>0)
                Cells->InsertNextCell(container);
        }
        ++mit;
        ++it;
    }

    poly->SetPoints(Points);
    poly->SetLines(Cells);

    std::cout << "Thread " << threadId << " finished tracking" << std::endl;
}

template< class TTensorPixelType,
          class TPDPixelType>
vtkSmartPointer< vtkPolyData > StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::AddPolyData(FiberPolyDataType poly1, FiberPolyDataType poly2)
{
    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = poly1->GetLines();
    vtkSmartPointer<vtkPoints> vNewPoints = poly1->GetPoints();

    vtkSmartPointer<vtkCellArray> vLines = poly2->GetLines();
    vLines->InitTraversal();
    for( int i=0; i<vLines->GetNumberOfCells(); i++ )
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for( int j=0; j<numPoints; j++)
        {
            vtkIdType id = vNewPoints->InsertNextPoint(poly2->GetPoint(points[j]));
            container->GetPointIds()->InsertNextId(id);
        }
        vNewLines->InsertNextCell(container);
    }

    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    return vNewPolyData;
}
template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::AfterThreadedGenerateData()
{
    MITK_INFO << "Generating polydata ";
    m_FiberPolyData = m_PolyDataContainer->GetElement(0);
    for (int i=1; i<this->GetNumberOfThreads(); i++)
    {
        m_FiberPolyData = AddPolyData(m_FiberPolyData, m_PolyDataContainer->GetElement(i));
    }
    MITK_INFO << "done";
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
