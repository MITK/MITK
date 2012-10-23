/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkTractsToDWIImageFilter.h"
#include <boost/progress.hpp>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
{
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);
}

TractsToDWIImageFilter::~TractsToDWIImageFilter()
{

}

void TractsToDWIImageFilter::GenerateData()
{
    // initialize output dwi image
    m_DiffusionImage = OutputImageType::New();
    m_DiffusionImage->SetSpacing( m_Spacing );
    m_DiffusionImage->SetOrigin( m_Origin );
    m_DiffusionImage->SetDirection( m_DirectionMatrix );
    m_DiffusionImage->SetLargestPossibleRegion( m_ImageRegion );
    m_DiffusionImage->SetVectorLength( m_GradientList.size() );
    m_DiffusionImage->Allocate();
    m_DiffusionImage->FillBuffer(0);
    this->SetNthOutput (0, m_DiffusionImage);

    // initialize tensor image
    m_TensorImage = ItkTensorImageType::New();
    m_TensorImage->SetSpacing( m_Spacing );
    m_TensorImage->SetOrigin( m_Origin );
    m_TensorImage->SetDirection( m_DirectionMatrix );
    m_TensorImage->SetLargestPossibleRegion( m_ImageRegion );
    m_TensorImage->Allocate();
    ItkTensorType kernel; kernel.Fill(0.0);
//    float IsoADC = 0.02;
//    kernel.SetElement(0, IsoADC);
//    kernel.SetElement(3, IsoADC);
//    kernel.SetElement(5, IsoADC);
    m_TensorImage->FillBuffer(kernel);

    // fill tensor image
    float ADC = 0.001;
    float FA = 0.6;
    float e1 = ADC*(1+2*FA/sqrt(3-2*FA*FA));
    float e2 = ADC*(1-FA/sqrt(3-2*FA*FA));
    float e3 = e2;
    kernel.SetElement(0, e1);
    kernel.SetElement(3, e2);
    kernel.SetElement(5, e3);
    vnl_vector_fixed<double, 3> kernelDir; kernelDir[0]=1; kernelDir[1]=0; kernelDir[2]=0;

    float minSpacing = 1;
    if(m_Spacing[0]<m_Spacing[1] && m_Spacing[0]<m_Spacing[2])
        minSpacing = m_Spacing[0];
    else if (m_Spacing[1] < m_Spacing[2])
        minSpacing = m_Spacing[1];
    else
        minSpacing = m_Spacing[2];

    int numFibers = m_FiberBundle->GetNumFibers();
    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    fiberBundle->ResampleFibers(minSpacing);
    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        ++disp;
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );
        if (numPoints<2)
            continue;

        for( int j=0; j<numPoints; j++)
        {
            double* temp = fiberPolyData->GetPoint(points[j]);
            itk::Point<float, 3> vertex = GetItkPoint(temp);
            vnl_vector<double> v = GetVnlVector(temp);

            vnl_vector<double> dir(3);
            if (j<numPoints-1)
                dir = GetVnlVector(fiberPolyData->GetPoint(points[j+1]))-v;
            else
                dir = v-GetVnlVector(fiberPolyData->GetPoint(points[j-1]));

            itk::Index<3> index;
            itk::ContinuousIndex<float, 3> contIndex;
            m_TensorImage->TransformPhysicalPointToIndex(vertex, index);
            m_TensorImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

            float frac_x = contIndex[0] - index[0];
            float frac_y = contIndex[1] - index[1];
            float frac_z = contIndex[2] - index[2];

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

            if (index[0] < 0 || index[0] >= m_ImageRegion.GetSize()[0]-1)
                continue;
            if (index[1] < 0 || index[1] >= m_ImageRegion.GetSize()[1]-1)
                continue;
            if (index[2] < 0 || index[2] >= m_ImageRegion.GetSize()[2]-1)
                continue;

            ItkTensorType tensor; tensor.Fill(0.0);
            dir.normalize();
            vnl_vector_fixed<double, 3> axis = vnl_cross_3d(kernelDir, dir); axis.normalize();
            vnl_quaternion<double> rotation(axis, acos(dot_product(kernelDir, dir)));
            rotation.normalize();
            vnl_matrix_fixed<double, 3, 3> matrix = rotation.rotation_matrix_transpose();

            vnl_matrix_fixed<double, 3, 3> tensorMatrix;
            tensorMatrix[0][0] = kernel[0]; tensorMatrix[0][1] = kernel[1]; tensorMatrix[0][2] = kernel[2];
            tensorMatrix[1][0] = kernel[1]; tensorMatrix[1][1] = kernel[3]; tensorMatrix[1][2] = kernel[4];
            tensorMatrix[2][0] = kernel[2]; tensorMatrix[2][1] = kernel[4]; tensorMatrix[2][2] = kernel[5];

            tensorMatrix = matrix.transpose()*tensorMatrix*matrix;
            tensor[0] = tensorMatrix[0][0]; tensor[1] = tensorMatrix[0][1]; tensor[2] = tensorMatrix[0][2];
            tensor[3] = tensorMatrix[1][1]; tensor[4] = tensorMatrix[1][2]; tensor[5] = tensorMatrix[2][2];

            ItkTensorType pix = m_TensorImage->GetPixel(index);
            for (int x=0; x<2; x++)
                for (int y=0; y<2; y++)
                    for (int z=0; z<2; z++)
                    {
                        itk::Index<3> newIndex;

                        float weight = 1;
                        if (x==0)
                            weight *= frac_x;
                        else
                            weight *= 1-frac_x;
                        if (y==0)
                            weight *= frac_y;
                        else
                            weight *= 1-frac_y;
                        if (z==0)
                            weight *= frac_z;
                        else
                            weight *= 1-frac_z;

                        newIndex[0] = index[0]+x;
                        newIndex[1] = index[1]+y;
                        newIndex[2] = index[2]+z;

                        m_TensorImage->SetPixel(newIndex, pix + tensor*weight);
                    }
        }
    }

    // normalize tensor image

    // generate signal

    // add noise
}

itk::Point<float, 3> TractsToDWIImageFilter::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(double point[3])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}

}
