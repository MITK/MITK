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
    m_F(1.0),
    m_G(0.0),
    m_Interpolate(true),
    m_MinTractLength(0.0)
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

    m_InputImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    m_ImageSize.resize(3);
    m_ImageSize[0] = m_InputImage->GetLargestPossibleRegion().GetSize()[0];
    m_ImageSize[1] = m_InputImage->GetLargestPossibleRegion().GetSize()[1];
    m_ImageSize[2] = m_InputImage->GetLargestPossibleRegion().GetSize()[2];

    if (m_ImageSize[0]<3 || m_ImageSize[1]<3 || m_ImageSize[2]<3)
        m_Interpolate = false;

    m_ImageSpacing.resize(3);
    m_ImageSpacing[0] = m_InputImage->GetSpacing()[0];
    m_ImageSpacing[1] = m_InputImage->GetSpacing()[1];
    m_ImageSpacing[2] = m_InputImage->GetSpacing()[2];

    float minSpacing;
    if(m_ImageSpacing[0]<m_ImageSpacing[1] && m_ImageSpacing[0]<m_ImageSpacing[2])
        minSpacing = m_ImageSpacing[0];
    else if (m_ImageSpacing[1] < m_ImageSpacing[2])
        minSpacing = m_ImageSpacing[1];
    else
        minSpacing = m_ImageSpacing[2];
    if (m_StepSize<0.1*minSpacing)
    {
        m_StepSize = 0.1*minSpacing;
        m_PointPistance = 0.5*minSpacing;
    }

    m_PolyDataContainer = itk::VectorContainer< int, FiberPolyDataType >::New();
    for (int i=0; i<this->GetNumberOfThreads(); i++)
    {
        FiberPolyDataType poly = FiberPolyDataType::New();
        m_PolyDataContainer->InsertElement(i, poly);
    }

    if (m_SeedImage.IsNull())
    {
        // initialize mask image
        m_SeedImage = ItkUcharImgType::New();
        m_SeedImage->SetSpacing( m_InputImage->GetSpacing() );
        m_SeedImage->SetOrigin( m_InputImage->GetOrigin() );
        m_SeedImage->SetDirection( m_InputImage->GetDirection() );
        m_SeedImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
        m_SeedImage->Allocate();
        m_SeedImage->FillBuffer(1);
    }

    if (m_MaskImage.IsNull())
    {
        // initialize mask image
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( m_InputImage->GetSpacing() );
        m_MaskImage->SetOrigin( m_InputImage->GetOrigin() );
        m_MaskImage->SetDirection( m_InputImage->GetDirection() );
        m_MaskImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }

    m_FaImage = ItkFloatImgType::New();
    m_FaImage->SetSpacing( m_InputImage->GetSpacing() );
    m_FaImage->SetOrigin( m_InputImage->GetOrigin() );
    m_FaImage->SetDirection( m_InputImage->GetDirection() );
    m_FaImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
    m_FaImage->Allocate();
    m_FaImage->FillBuffer(0.0);

    m_PdImage = ItkPDImgType::New();
    m_PdImage->SetSpacing( m_InputImage->GetSpacing() );
    m_PdImage->SetOrigin( m_InputImage->GetOrigin() );
    m_PdImage->SetDirection( m_InputImage->GetDirection() );
    m_PdImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
    m_PdImage->Allocate();

    m_EmaxImage = ItkFloatImgType::New();
    m_EmaxImage->SetSpacing( m_InputImage->GetSpacing() );
    m_EmaxImage->SetOrigin( m_InputImage->GetOrigin() );
    m_EmaxImage->SetDirection( m_InputImage->GetDirection() );
    m_EmaxImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
    m_EmaxImage->Allocate();
    m_EmaxImage->FillBuffer(1.0);

    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typename TensorType::EigenValuesArrayType eigenvalues;
    typename TensorType::EigenVectorsMatrixType eigenvectors;
    for (int x=0; x<m_ImageSize[0]; x++)
        for (int y=0; y<m_ImageSize[1]; y++)
            for (int z=0; z<m_ImageSize[2]; z++)
            {
                typename InputImageType::IndexType index;
                index[0] = x; index[1] = y; index[2] = z;
                typename InputImageType::PixelType tensor = m_InputImage->GetPixel(index);

                vnl_vector_fixed<double,3> dir;
                tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
                dir[0] = eigenvectors(2, 0);
                dir[1] = eigenvectors(2, 1);
                dir[2] = eigenvectors(2, 2);
                dir.normalize();
                m_PdImage->SetPixel(index, dir);
                m_FaImage->SetPixel(index, tensor.GetFractionalAnisotropy());
                m_EmaxImage->SetPixel(index, 2/eigenvalues[2]);
            }

    if (m_Interpolate)
        std::cout << "StreamlineTrackingFilter: using trilinear interpolation" << std::endl;
    else
    {
        if (m_MinCurvatureRadius<0.0)
            m_MinCurvatureRadius = 0.1*minSpacing;

        std::cout << "StreamlineTrackingFilter: using nearest neighbor interpolation" << std::endl;
    }

    if (m_MinCurvatureRadius<0.0)
        m_MinCurvatureRadius = 0.5*minSpacing;

    std::cout << "StreamlineTrackingFilter: Min. curvature radius: " << m_MinCurvatureRadius << std::endl;
    std::cout << "StreamlineTrackingFilter: FA threshold: " << m_FaThreshold << std::endl;
    std::cout << "StreamlineTrackingFilter: stepsize: " << m_StepSize << " mm" << std::endl;
    std::cout << "StreamlineTrackingFilter: f: " << m_F << std::endl;
    std::cout << "StreamlineTrackingFilter: g: " << m_G << std::endl;
    std::cout << "StreamlineTrackingFilter: starting streamline tracking" << std::endl;
}

template< class TTensorPixelType, class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType, TPDPixelType>
::CalculateNewPosition(itk::ContinuousIndex<double, 3>& pos, vnl_vector_fixed<double,3>& dir, typename InputImageType::IndexType& index)
{
    vnl_matrix_fixed< double, 3, 3 > rot = m_InputImage->GetDirection().GetTranspose();
    dir = rot*dir;
    if (true)
    {
        dir *= m_StepSize;
        pos[0] += dir[0]/m_ImageSpacing[0];
        pos[1] += dir[1]/m_ImageSpacing[1];
        pos[2] += dir[2]/m_ImageSpacing[2];
        index[0] = RoundToNearest(pos[0]);
        index[1] = RoundToNearest(pos[1]);
        index[2] = RoundToNearest(pos[2]);
    }
    else
    {
        dir[0] /= m_ImageSpacing[0];
        dir[1] /= m_ImageSpacing[1];
        dir[2] /= m_ImageSpacing[2];

        int smallest = 0;
        float x = 100000;
        if (dir[0]>0)
        {
            if (fabs(fabs(RoundToNearest(pos[0])-pos[0])-0.5)>mitk::eps)
                x = fabs(pos[0]-RoundToNearest(pos[0])-0.5)/dir[0];
            else
                x = fabs(pos[0]-std::ceil(pos[0])-0.5)/dir[0];
        }
        else if (dir[0]<0)
        {
            if (fabs(fabs(RoundToNearest(pos[0])-pos[0])-0.5)>mitk::eps)
                x = -fabs(pos[0]-RoundToNearest(pos[0])+0.5)/dir[0];
            else
                x = -fabs(pos[0]-std::floor(pos[0])+0.5)/dir[0];
        }
        float s = x;

        float y = 100000;
        if (dir[1]>0)
        {
            if (fabs(fabs(RoundToNearest(pos[1])-pos[1])-0.5)>mitk::eps)
                y = fabs(pos[1]-RoundToNearest(pos[1])-0.5)/dir[1];
            else
                y = fabs(pos[1]-std::ceil(pos[1])-0.5)/dir[1];
        }
        else if (dir[1]<0)
        {
            if (fabs(fabs(RoundToNearest(pos[1])-pos[1])-0.5)>mitk::eps)
                y = -fabs(pos[1]-RoundToNearest(pos[1])+0.5)/dir[1];
            else
                y = -fabs(pos[1]-std::floor(pos[1])+0.5)/dir[1];
        }
        if (s>y)
        {
            s=y;
            smallest = 1;
        }

        float z = 100000;
        if (dir[2]>0)
        {
            if (fabs(fabs(RoundToNearest(pos[2])-pos[2])-0.5)>mitk::eps)
                z = fabs(pos[2]-RoundToNearest(pos[2])-0.5)/dir[2];
            else
                z = fabs(pos[2]-std::ceil(pos[2])-0.5)/dir[2];
        }
        else if (dir[2]<0)
        {
            if (fabs(fabs(RoundToNearest(pos[2])-pos[2])-0.5)>mitk::eps)
                z = -fabs(pos[2]-RoundToNearest(pos[2])+0.5)/dir[2];
            else
                z = -fabs(pos[2]-std::floor(pos[2])+0.5)/dir[2];
        }
        if (s>z)
        {
            s=z;
            smallest = 2;
        }

//        MITK_INFO << "---------------------------------------------";
//        MITK_INFO << "s: " << s;
//        MITK_INFO << "dir: " << dir;
//        MITK_INFO << "old: " << pos[0] << ", " << pos[1] << ", " << pos[2];

        pos[0] += dir[0]*s;
        pos[1] += dir[1]*s;
        pos[2] += dir[2]*s;

        switch (smallest)
        {
        case 0:
            if (dir[0]<0)
                index[0] = std::floor(pos[0]);
            else
                index[0] = std::ceil(pos[0]);
            index[1] = RoundToNearest(pos[1]);
            index[2] = RoundToNearest(pos[2]);
            break;

        case 1:
            if (dir[1]<0)
                index[1] = std::floor(pos[1]);
            else
                index[1] = std::ceil(pos[1]);
            index[0] = RoundToNearest(pos[0]);
            index[2] = RoundToNearest(pos[2]);
            break;

        case 2:
            if (dir[2]<0)
                index[2] = std::floor(pos[2]);
            else
                index[2] = std::ceil(pos[2]);
            index[1] = RoundToNearest(pos[1]);
            index[0] = RoundToNearest(pos[0]);
        }

//        float x = 100000;
//        if (dir[0]>0)
//            x = fabs(pos[0]-RoundToNearest(pos[0])-0.5)/dir[0];
//        else if (dir[0]<0)
//            x = -fabs(pos[0]-RoundToNearest(pos[0])+0.5)/dir[0];
//        float s = x;

//        float y = 100000;
//        if (dir[1]>0)
//            y = fabs(pos[1]-RoundToNearest(pos[1])-0.5)/dir[1];
//        else if (dir[1]<0)
//            y = -fabs(pos[1]-RoundToNearest(pos[1])+0.5)/dir[1];
//        if (s>y)
//            s=y;

//        float z = 100000;
//        if (dir[2]>0)
//            z = fabs(pos[2]-RoundToNearest(pos[2])-0.5)/dir[2];
//        else if (dir[2]<0)
//            z = -fabs(pos[2]-RoundToNearest(pos[2])+0.5)/dir[2];

//        if (s>z)
//            s=z;
//        s *= 1.001;

//        pos[0] += dir[0]*s;
//        pos[1] += dir[1]*s;
//        pos[2] += dir[2]*s;

//        index[0] = RoundToNearest(pos[0]);
//        index[1] = RoundToNearest(pos[1]);
//        index[2] = RoundToNearest(pos[2]);

//        MITK_INFO << "new: " << pos[0] << ", " << pos[1] << ", " << pos[2];
    }
}

template< class TTensorPixelType, class TPDPixelType>
bool StreamlineTrackingFilter< TTensorPixelType, TPDPixelType>
::IsValidPosition(itk::ContinuousIndex<double, 3>& pos, typename InputImageType::IndexType &index, vnl_vector_fixed< float, 8 >& interpWeights)
{
    if (!m_InputImage->GetLargestPossibleRegion().IsInside(index) || m_MaskImage->GetPixel(index)==0)
        return false;

    if (m_Interpolate)
    {
        float frac_x = pos[0] - index[0];
        float frac_y = pos[1] - index[1];
        float frac_z = pos[2] - index[2];

        if (frac_x<0)
        {
            index[0] -= 1;
            frac_x += 1;
        }
        if (frac_y<0)
        {
            index[1] -= 1;
            frac_y += 1;
        }
        if (frac_z<0)
        {
            index[2] -= 1;
            frac_z += 1;
        }

        frac_x = 1-frac_x;
        frac_y = 1-frac_y;
        frac_z = 1-frac_z;

        // int coordinates inside image?
        if (index[0] < 0 || index[0] >= m_ImageSize[0]-1)
            return false;
        if (index[1] < 0 || index[1] >= m_ImageSize[1]-1)
            return false;
        if (index[2] < 0 || index[2] >= m_ImageSize[2]-1)
            return false;

        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        typename InputImageType::IndexType tmpIdx;
        float FA = m_FaImage->GetPixel(index) * interpWeights[0];
        tmpIdx = index; tmpIdx[0]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[1];
        tmpIdx = index; tmpIdx[1]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[2];
        tmpIdx = index; tmpIdx[2]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[3];
        tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[4];
        tmpIdx = index; tmpIdx[1]++; tmpIdx[2]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[5];
        tmpIdx = index; tmpIdx[2]++; tmpIdx[0]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[6];
        tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        FA +=  m_FaImage->GetPixel(tmpIdx) * interpWeights[7];

        if (FA<m_FaThreshold)
            return false;
    }
    else if (m_FaImage->GetPixel(index)<m_FaThreshold)
        return false;

    return true;
}

template< class TTensorPixelType, class TPDPixelType>
float StreamlineTrackingFilter< TTensorPixelType, TPDPixelType>
::FollowStreamline(itk::ContinuousIndex<double, 3> pos, int dirSign, vtkPoints* points, std::vector< vtkIdType >& ids)
{
    float tractLength = 0;
    typedef itk::DiffusionTensor3D<TTensorPixelType>    TensorType;
    typename TensorType::EigenValuesArrayType eigenvalues;
    typename TensorType::EigenVectorsMatrixType eigenvectors;
    vnl_vector_fixed< float, 8 > interpWeights;

    typename InputImageType::IndexType index, indexOld;
    indexOld[0] = -1; indexOld[1] = -1; indexOld[2] = -1;
    itk::Point<double> worldPos;
    float distance = 0;
    float distanceInVoxel = 0;

    // starting index and direction
    index[0] = RoundToNearest(pos[0]);
    index[1] = RoundToNearest(pos[1]);
    index[2] = RoundToNearest(pos[2]);
    vnl_vector_fixed<double,3> dir = m_PdImage->GetPixel(index);
    dir *= dirSign;                     // reverse direction
    vnl_vector_fixed<double,3> dirOld = dir;
    if (dir.magnitude()<mitk::eps)
        return tractLength;

    for (int step=0; step< m_MaxLength/2; step++)
    {
        // get new position
        CalculateNewPosition(pos, dir, index);
        distance += m_StepSize;
        tractLength +=  m_StepSize;
        distanceInVoxel += m_StepSize;

        // is new position valid (inside image, above FA threshold etc.)
        if (!IsValidPosition(pos, index, interpWeights))   // if not add last point and end streamline
        {
            m_InputImage->TransformContinuousIndexToPhysicalPoint( pos, worldPos );
            ids.push_back(points->InsertNextPoint(worldPos.GetDataPointer()));
            return tractLength;
        }
        else if (distance>=m_PointPistance)
        {
            m_InputImage->TransformContinuousIndexToPhysicalPoint( pos, worldPos );
            ids.push_back(points->InsertNextPoint(worldPos.GetDataPointer()));
            distance = 0;
        }

        if (!m_Interpolate)                         // use nearest neighbour interpolation
        {
            if (indexOld!=index)                    // did we enter a new voxel? if yes, calculate new direction
            {
                dir = m_PdImage->GetPixel(index);   // get principal direction

                typename InputImageType::PixelType tensor = m_InputImage->GetPixel(index);
                float scale = m_EmaxImage->GetPixel(index);
                dir[0] = m_F*dir[0] + (1-m_F)*( (1-m_G)*dirOld[0] + scale*m_G*(tensor[0]*dirOld[0] + tensor[1]*dirOld[1] + tensor[2]*dirOld[2]));
                dir[1] = m_F*dir[1] + (1-m_F)*( (1-m_G)*dirOld[1] + scale*m_G*(tensor[1]*dirOld[0] + tensor[3]*dirOld[1] + tensor[4]*dirOld[2]));
                dir[2] = m_F*dir[2] + (1-m_F)*( (1-m_G)*dirOld[2] + scale*m_G*(tensor[2]*dirOld[0] + tensor[4]*dirOld[1] + tensor[5]*dirOld[2]));
                dir.normalize();

                float angle = dot_product(dirOld, dir);
                if (angle<0)
                {
                    dir *= -1;
                    angle *= -1;
                }

                float r = m_StepSize/(2*std::asin(std::acos(angle)/2));
                if (r<m_MinCurvatureRadius)
                    return tractLength;

                if (dir.magnitude()<mitk::eps)
                    dir = dirOld;
                else
                    dirOld = dir;
                indexOld = index;
                distanceInVoxel = 0;
            }
            else
                dir = dirOld;
        }
        else // use trilinear interpolation (weights calculated in IsValidPosition())
        {
            typename InputImageType::PixelType tensor = m_InputImage->GetPixel(index) * interpWeights[0];
            typename InputImageType::IndexType tmpIdx = index; tmpIdx[0]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[1];
            tmpIdx = index; tmpIdx[1]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[2];
            tmpIdx = index; tmpIdx[2]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[3];
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[4];
            tmpIdx = index; tmpIdx[1]++; tmpIdx[2]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[5];
            tmpIdx = index; tmpIdx[2]++; tmpIdx[0]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[6];
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
            tensor +=  m_InputImage->GetPixel(tmpIdx) * interpWeights[7];

            tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
            dir[0] = eigenvectors(2, 0);
            dir[1] = eigenvectors(2, 1);
            dir[2] = eigenvectors(2, 2);
            dir.normalize();

            float scale = 2/eigenvalues[2];
            dir[0] = m_F*dir[0] + (1-m_F)*( (1-m_G)*dirOld[0] + scale*m_G*(tensor[0]*dirOld[0] + tensor[1]*dirOld[1] + tensor[2]*dirOld[2]));
            dir[1] = m_F*dir[1] + (1-m_F)*( (1-m_G)*dirOld[1] + scale*m_G*(tensor[1]*dirOld[0] + tensor[3]*dirOld[1] + tensor[4]*dirOld[2]));
            dir[2] = m_F*dir[2] + (1-m_F)*( (1-m_G)*dirOld[2] + scale*m_G*(tensor[2]*dirOld[0] + tensor[4]*dirOld[1] + tensor[5]*dirOld[2]));
            dir.normalize();

            float angle = dot_product(dirOld, dir);
            if (angle<0)
            {
                dir *= -1;
                angle *= -1;
            }

            float r = m_StepSize/(2*std::asin(std::acos(angle)/2));
            if (r<m_MinCurvatureRadius)
                return tractLength;

            if (dir.magnitude()<mitk::eps)
                dir = dirOld;
            else
                dirOld = dir;

            indexOld = index;
        }
    }
    return tractLength;
}

template< class TTensorPixelType,
          class TPDPixelType>
void StreamlineTrackingFilter< TTensorPixelType,
TPDPixelType>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{
    FiberPolyDataType poly = m_PolyDataContainer->GetElement(threadId);
    vtkSmartPointer<vtkPoints> points = vtkPoints::New();
    vtkSmartPointer<vtkCellArray> Cells = vtkCellArray::New();

    typedef itk::DiffusionTensor3D<TTensorPixelType>        TensorType;
    typedef ImageRegionConstIterator< InputImageType >      InputIteratorType;
    typedef ImageRegionConstIterator< ItkUcharImgType >     MaskIteratorType;
    typedef ImageRegionConstIterator< ItkFloatImgType >     FloatIteratorType;
    typedef typename InputImageType::PixelType              InputTensorType;

    InputIteratorType   it(m_InputImage, outputRegionForThread );
    MaskIteratorType    mit(m_SeedImage, outputRegionForThread );
    FloatIteratorType   fit(m_FaImage, outputRegionForThread );
    MaskIteratorType    mit2(m_MaskImage, outputRegionForThread );


    it.GoToBegin();
    mit.GoToBegin();
    mit2.GoToBegin();
    fit.GoToBegin();
    itk::Point<double> worldPos;
    while( !it.IsAtEnd() )
    {
        if (mit.Value()==0 || fit.Value()<m_FaThreshold || mit2.Value()==0)
        {
            ++mit;
            ++mit2;
            ++it;
            ++fit;
            continue;
        }

        for (int s=0; s<m_SeedsPerVoxel; s++)
        {
            vtkSmartPointer<vtkPolyLine> line = vtkSmartPointer<vtkPolyLine>::New();
            std::vector< vtkIdType > pointIDs;
            typename InputImageType::IndexType index = it.GetIndex();
            itk::ContinuousIndex<double, 3> start;
            unsigned int counter = 0;

            if (m_SeedsPerVoxel>1)
            {
                start[0] = index[0]+(double)(rand()%99-49)/100;
                start[1] = index[1]+(double)(rand()%99-49)/100;
                start[2] = index[2]+(double)(rand()%99-49)/100;
            }
            else
            {
                start[0] = index[0];
                start[1] = index[1];
                start[2] = index[2];
            }

            // forward tracking
            float tractLength = FollowStreamline(start, 1, points, pointIDs);

            // add ids to line
            counter += pointIDs.size();
            while (!pointIDs.empty())
            {
                line->GetPointIds()->InsertNextId(pointIDs.back());
                pointIDs.pop_back();
            }

            // insert start point
            m_InputImage->TransformContinuousIndexToPhysicalPoint( start, worldPos );
            line->GetPointIds()->InsertNextId(points->InsertNextPoint(worldPos.GetDataPointer()));

            // backward tracking
            tractLength += FollowStreamline(start, -1, points, pointIDs);

            counter += pointIDs.size();

            if (tractLength<m_MinTractLength || counter<2)
                continue;

            // add ids to line
            for (int i=0; i<pointIDs.size(); i++)
                line->GetPointIds()->InsertNextId(pointIDs.at(i));

            Cells->InsertNextCell(line);
        }
        ++mit;
        ++mit2;
        ++it;
        ++fit;
    }
    poly->SetPoints(points);
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
