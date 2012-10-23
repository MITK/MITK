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
#include <itkTractDensityImageFilter.h>
#include <itkStatisticsImageFilter.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
    : m_BValue(1000)
    , m_NoiseVariance(0.004)
    , m_GreyMatterAdc(0.01)
    , m_DefaultBaseline(1)
    , m_MaxFA(0.9)
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

TractsToDWIImageFilter::DoubleDwiType::PixelType TractsToDWIImageFilter::SimulateMeasurement(ItkTensorType& T, float weight)
{
    typename DoubleDwiType::PixelType out;
    out.SetSize(m_GradientList.size());
    out.Fill(0);

    short s0 = m_DefaultBaseline;
//    if (m_SimulateBaseline)
//        s0 = (GetTensorL2Norm(T)/m_MaxBaseline)*m_SignalScale;

    for( unsigned int i=0; i<m_GradientList.size(); i++)
    {
        GradientType g = m_GradientList[i];

        if (g.GetNorm()>0.0001)
        {
            itk::DiffusionTensor3D<float> S;
            S[0] = g[0]*g[0];
            S[1] = g[1]*g[0];
            S[2] = g[2]*g[0];
            S[3] = g[1]*g[1];
            S[4] = g[2]*g[1];
            S[5] = g[2]*g[2];

            double D = T[0]*S[0] + T[1]*S[1] + T[2]*S[2] +
                       T[1]*S[1] + T[3]*S[3] + T[4]*S[4] +
                       T[2]*S[2] + T[4]*S[4] + T[5]*S[5];

            // check for corrupted tensor and generate signal
            if (D>=0)
                out[i] = weight*s0*exp ( -m_BValue * D );
        }
        else
            out[i] = s0;
    }

    return out;
}

void TractsToDWIImageFilter::GenerateData()
{
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    // initialize output dwi image
    m_DiffusionImage = OutputImageType::New();
    m_DiffusionImage->SetSpacing( m_Spacing );
    m_DiffusionImage->SetOrigin( m_Origin );
    m_DiffusionImage->SetDirection( m_DirectionMatrix );
    m_DiffusionImage->SetLargestPossibleRegion( m_ImageRegion );
    m_DiffusionImage->SetBufferedRegion( m_ImageRegion );
    m_DiffusionImage->SetRequestedRegion( m_ImageRegion );
    m_DiffusionImage->SetVectorLength( m_GradientList.size() );
    m_DiffusionImage->Allocate();
    typename OutputImageType::PixelType pix;
    pix.SetSize(m_GradientList.size());
    pix.Fill(0.0);
    m_DiffusionImage->FillBuffer(pix);
    this->SetNthOutput (0, m_DiffusionImage);

    // generate working double image because we work with small values (will be scaled later)
    DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
    doubleDwi->SetSpacing( m_Spacing );
    doubleDwi->SetOrigin( m_Origin );
    doubleDwi->SetDirection( m_DirectionMatrix );
    doubleDwi->SetLargestPossibleRegion( m_ImageRegion );
    doubleDwi->SetBufferedRegion( m_ImageRegion );
    doubleDwi->SetRequestedRegion( m_ImageRegion );
    doubleDwi->SetVectorLength( m_GradientList.size() );
    doubleDwi->Allocate();
    DoubleDwiType::PixelType doublePix;
    doublePix.SetSize(m_GradientList.size());
    doublePix.Fill(0.0);
    doubleDwi->FillBuffer(doublePix);

    // generate tract density image
    m_TractDensityImage = ItkFloatImgType::New();
    m_TractDensityImage->SetSpacing( m_Spacing );
    m_TractDensityImage->SetOrigin( m_Origin );
    m_TractDensityImage->SetDirection( m_DirectionMatrix );
    m_TractDensityImage->SetLargestPossibleRegion( m_ImageRegion );
    m_TractDensityImage->SetBufferedRegion( m_ImageRegion );
    m_TractDensityImage->SetRequestedRegion( m_ImageRegion );
    m_TractDensityImage->Allocate();
    m_TractDensityImage->FillBuffer(0.0);
    itk::TractDensityImageFilter< ItkFloatImgType >::Pointer tractDensityGenerator = itk::TractDensityImageFilter< ItkFloatImgType >::New();
    tractDensityGenerator->SetFiberBundle(m_FiberBundle);
    tractDensityGenerator->SetBinaryOutput(false);
    tractDensityGenerator->SetOutputAbsoluteValues(true);
    tractDensityGenerator->SetUpsamplingFactor(1);
    tractDensityGenerator->SetInputImage(m_TractDensityImage);
    tractDensityGenerator->SetUseImageGeometry(true);
    tractDensityGenerator->Update();
    typedef itk::Image<float,3> OutType;
    m_TractDensityImage = tractDensityGenerator->GetOutput();
    itk::StatisticsImageFilter< ItkFloatImgType >::Pointer statisticsFilter = itk::StatisticsImageFilter< ItkFloatImgType >::New();
    statisticsFilter->SetInput(tractDensityGenerator->GetOutput());
    statisticsFilter->Update();
    m_MaxDensity = statisticsFilter->GetMaximum();

    // generate signal
    ItkTensorType kernel; kernel.Fill(0.0);
    float ADC = 0.001;
    vnl_vector_fixed<double, 3> kernelDir; kernelDir[0]=1; kernelDir[1]=0; kernelDir[2]=0;

    float minSpacing = 1;
    if(m_Spacing[0]<m_Spacing[1] && m_Spacing[0]<m_Spacing[2])
        minSpacing = m_Spacing[0];
    else if (m_Spacing[1] < m_Spacing[2])
        minSpacing = m_Spacing[1];
    else
        minSpacing = m_Spacing[2];

    int numFibers = m_FiberBundle->GetNumFibers();

    if (numFibers<=0)
        itkExceptionMacro("Input fiber bundle contains no fibers!");

    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
//    fiberBundle->ResampleFibers(minSpacing);
    fiberBundle->DoFiberSmoothing(ceil(10.0*5.0/minSpacing));
    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    MITK_INFO << "Generating Signal";
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
//            itk::ContinuousIndex<float, 3> contIndex;
            m_DiffusionImage->TransformPhysicalPointToIndex(vertex, index);
//            m_TensorImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

            if (!m_ImageRegion.IsInside(index))
                continue;

            // prepare kernel (todo: catch crossings before calculating FA)
            float td = m_TractDensityImage->GetPixel(index);
            float FA = m_MaxFA*td/m_MaxDensity;
            float e1 = ADC*(1+2*FA/sqrt(3-2*FA*FA));
            float e2 = ADC*(1-FA/sqrt(3-2*FA*FA));
            kernel.SetElement(0, e1);
            kernel.SetElement(3, e2);
            kernel.SetElement(5, e2);

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

            doublePix = doubleDwi->GetPixel(index);
            doubleDwi->SetPixel(index, doublePix + SimulateMeasurement(tensor, 1.0));
        }
    }

    typedef ImageRegionIterator<DoubleDwiType>      IteratorOutputType;
    IteratorOutputType it (doubleDwi, m_ImageRegion);
    while(!it.IsAtEnd())
    {
        doublePix = it.Get();
        DoubleDwiType::IndexType index = it.GetIndex();
        m_DiffusionImage->SetPixel(index, doublePix);
        ++it;
    }

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
