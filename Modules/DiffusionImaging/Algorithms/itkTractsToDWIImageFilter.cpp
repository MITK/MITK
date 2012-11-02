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
#include <itkTractsToVectorImageFilter.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
    : m_BValue(1000)
    , m_NoiseVariance(0.004)
    , m_KernelFA(0.9)
    , m_MinCrossingAngle(0.7)
    , m_MaxCrossingComplexity(2)
    , m_SNR(15)
    , m_MaxBaseline(0)
    , m_WhiteMatterBaseline(200)
    , m_CsfBaseline(1000)
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

double TractsToDWIImageFilter::GetTensorL2Norm(itk::DiffusionTensor3D<float>& T)
{
    return sqrt(T[0]*T[0] + T[3]*T[3] + T[5]*T[5] + T[1]*T[2]*2.0 + T[2]*T[4]*2.0 + T[1]*T[4]*2.0);
}

TractsToDWIImageFilter::DoubleDwiType::PixelType TractsToDWIImageFilter::SimulateMeasurement(ItkTensorType& T, double baseline)
{
    typename DoubleDwiType::PixelType pix;
    pix.SetSize(m_GradientList.size());
    pix.Fill(0);

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
                pix[i] = baseline*exp ( -m_BValue * D );
        }
        else
            pix[i] = baseline;
    }

    return pix;
}

void TractsToDWIImageFilter::AddNoise(DoubleDwiType::PixelType& pix)
{
    for( unsigned int i=0; i<m_GradientList.size(); i++)
    {
        double signal = pix[i];
        pix[i] = sqrt(pow(signal + m_WhiteMatterBaseline*m_RandGen->GetNormalVariate(0.0, m_NoiseVariance), 2) +  m_WhiteMatterBaseline*pow(m_RandGen->GetNormalVariate(0.0, m_NoiseVariance),2));
    }
}

void TractsToDWIImageFilter::GenerateData()
{
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    int numFibers = m_FiberBundle->GetNumFibers();
    if (numFibers<=0)
        itkExceptionMacro("Input fiber bundle contains no fibers!");

    float minSpacing = 1;
    if(m_Spacing[0]<m_Spacing[1] && m_Spacing[0]<m_Spacing[2])
        minSpacing = m_Spacing[0];
    else if (m_Spacing[1] < m_Spacing[2])
        minSpacing = m_Spacing[1];
    else
        minSpacing = m_Spacing[2];

    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    fiberBundle->DoFiberSmoothing(ceil(10.0*2.0/minSpacing));

    m_RandGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();

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
    DoubleDwiType::PixelType anisoSignal;
    anisoSignal.SetSize(m_GradientList.size());
    anisoSignal.Fill(0.0);
    doubleDwi->FillBuffer(anisoSignal);

    // search crossing regions
    if (m_TissueMask.IsNull())
    {
        m_TissueMask = ItkUcharImgType::New();
        m_TissueMask->SetSpacing( m_Spacing );
        m_TissueMask->SetOrigin( m_Origin );
        m_TissueMask->SetDirection( m_DirectionMatrix );
        m_TissueMask->SetLargestPossibleRegion( m_ImageRegion );
        m_TissueMask->SetBufferedRegion( m_ImageRegion );
        m_TissueMask->SetRequestedRegion( m_ImageRegion );
        m_TissueMask->Allocate();
        m_TissueMask->FillBuffer(1);
    }
    else
        MITK_INFO << "Using tissue mask";
    itk::TractsToVectorImageFilter::Pointer crossingsFilter = itk::TractsToVectorImageFilter::New();
    crossingsFilter->SetFiberBundle(fiberBundle);
    crossingsFilter->SetAngularThreshold(m_MinCrossingAngle);
    crossingsFilter->SetUseFastClustering(true);
    crossingsFilter->SetFiberSampling(0);
    crossingsFilter->SetNormalizeVectors(false);
    crossingsFilter->SetMaxNumDirections(m_MaxCrossingComplexity);
    crossingsFilter->SetMaskImage(m_TissueMask);
    crossingsFilter->Update();
    ItkUcharImgType::Pointer numDirImage = crossingsFilter->GetNumDirectionsImage();

    // adjust SNR
    if (m_SNR <= 0)
        m_SNR = 0.0001;
    if (m_SNR>99)
        m_NoiseVariance = 0;
    else
    {
        m_NoiseVariance = 1/m_SNR;
        m_NoiseVariance *= m_NoiseVariance;
    }
    MITK_INFO << "Noise variance: " << m_NoiseVariance;

    ItkTensorType kernel; kernel.Fill(0.0);
    float ADC = 0.001;
    vnl_vector_fixed<double, 3> kernelDir; kernelDir[0]=1; kernelDir[1]=0; kernelDir[2]=0;

    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
    vLines->InitTraversal();

    MITK_INFO << "Generating signal of anisotropic compartment";
    m_MaxBaseline = 0;
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
            m_DiffusionImage->TransformPhysicalPointToIndex(vertex, index);

            if (!m_ImageRegion.IsInside(index))
                continue;

            float FA = m_KernelFA;
            float e1 = ADC*(1+2*FA/sqrt(3-2*FA*FA));
            float e2 = ADC*(1-FA/sqrt(3-2*FA*FA));
            kernel.SetElement(0, e1);
            kernel.SetElement(3, e2);
            kernel.SetElement(5, e2);

            ItkTensorType anisoCompTensor; anisoCompTensor.Fill(0.0);
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
            anisoCompTensor[0] = tensorMatrix[0][0]; anisoCompTensor[1] = tensorMatrix[0][1]; anisoCompTensor[2] = tensorMatrix[0][2];
            anisoCompTensor[3] = tensorMatrix[1][1]; anisoCompTensor[4] = tensorMatrix[1][2]; anisoCompTensor[5] = tensorMatrix[2][2];

            anisoSignal = doubleDwi->GetPixel(index);
            doubleDwi->SetPixel(index, anisoSignal + SimulateMeasurement(anisoCompTensor, 1));
            anisoSignal = doubleDwi->GetPixel(index);
            if (anisoSignal[0]>m_MaxBaseline)
                m_MaxBaseline = anisoSignal[0];
        }
    }

    // add isotropic diffusion, noise and rescale image
    itk::DiffusionTensor3D<float> isoCompTensor;
    isoCompTensor.Fill(0);
    isoCompTensor.SetElement(0,ADC);
    isoCompTensor.SetElement(3,ADC);
    isoCompTensor.SetElement(5,ADC);

    MITK_INFO << "Adding signal of isotropic compartment and noise";
    typedef ImageRegionIterator<DoubleDwiType>      IteratorOutputType;
    IteratorOutputType it (doubleDwi, m_ImageRegion);
    it.GoToBegin();

    itk::StatisticsImageFilter< ItkUcharImgType >::Pointer statsFilter = itk::StatisticsImageFilter< ItkUcharImgType >::New();
    statsFilter->SetInput(numDirImage);
    statsFilter->Update();
    unsigned char max = statsFilter->GetMaximum();

    DoubleDwiType::PixelType signal;
    signal.SetSize(m_GradientList.size());
    while(!it.IsAtEnd())
    {
        DoubleDwiType::IndexType index = it.GetIndex();
        signal.Fill(0.0);

        if (m_TissueMask->GetPixel(index)>0)
        {
            anisoSignal = it.Get();

    //        double v = max*anisoSignal[0]/m_MaxBaseline;        // anisotropic volume fraction
    //        if (numDirImage->GetPixel(index)>1)
    //            v /= numDirImage->GetPixel(index);
            double v = anisoSignal[0]/m_MaxBaseline;        // anisotropic volume fraction

            if (anisoSignal[0]>0)
                anisoSignal /= anisoSignal[0];              // normalize signal

            anisoSignal = v*m_WhiteMatterBaseline*anisoSignal;    // anisotropic signal compartment

            // add isotropic compartment
            signal = anisoSignal + (1-v)*SimulateMeasurement(isoCompTensor, m_CsfBaseline);
        }

        AddNoise(signal);                               // add rician noise
        m_DiffusionImage->SetPixel(index, signal);
        ++it;
    }
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


vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(Vector<float,3>& vector)
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = vector[0];
    vnlVector[1] = vector[1];
    vnlVector[2] = vector[2];
    return vnlVector;
}

}
