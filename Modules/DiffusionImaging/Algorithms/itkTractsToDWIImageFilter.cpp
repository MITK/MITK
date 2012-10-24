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
    , m_GreyMatterAdc(0.01)
    , m_DefaultBaseline(1)
    , m_MaxFA(0.9)
    , m_MinCrossingAngle(0.9)
    , m_MaxCrossingComplexity(3)
    , m_SNR(15)
    , m_SignalScale(1000)
    , m_MaxBaseline(0)
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

TractsToDWIImageFilter::DoubleDwiType::PixelType TractsToDWIImageFilter::SimulateMeasurement(ItkTensorType& T, double scaleSignal)
{
    typename DoubleDwiType::PixelType pix;
    pix.SetSize(m_GradientList.size());
    pix.Fill(0);

    short s0 = m_DefaultBaseline;

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
                pix[i] = scaleSignal*s0*exp ( -m_BValue * D );
        }
        else
            pix[i] = scaleSignal*s0;
    }

    return pix;
}

void TractsToDWIImageFilter::AddNoise(DoubleDwiType::PixelType& pix)
{
    for( unsigned int i=0; i<m_GradientList.size(); i++)
    {
        double signal = pix[i];
        double val = sqrt(pow(signal + m_SignalScale*m_RandGen->GetNormalVariate(0.0, m_NoiseVariance), 2) +  m_SignalScale*pow(m_RandGen->GetNormalVariate(0.0, m_NoiseVariance),2));
        pix[i] += val;
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
    fiberBundle->DoFiberSmoothing(ceil(10.0*5.0/minSpacing));

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

    // search crossing regions
    ItkUcharImgType::Pointer maskImage = ItkUcharImgType::New();
    maskImage->SetSpacing( m_Spacing );
    maskImage->SetOrigin( m_Origin );
    maskImage->SetDirection( m_DirectionMatrix );
    maskImage->SetLargestPossibleRegion( m_ImageRegion );
    maskImage->SetBufferedRegion( m_ImageRegion );
    maskImage->SetRequestedRegion( m_ImageRegion );
    maskImage->Allocate();
    maskImage->FillBuffer(1);
    itk::TractsToVectorImageFilter::Pointer crossingsFilter = itk::TractsToVectorImageFilter::New();
    crossingsFilter->SetFiberBundle(fiberBundle);
    crossingsFilter->SetAngularThreshold(m_MinCrossingAngle);
    crossingsFilter->SetUseFastClustering(true);
    crossingsFilter->SetFiberSampling(0);
    crossingsFilter->SetNormalizeVectors(false);
    crossingsFilter->SetMaxNumDirections(m_MaxCrossingComplexity);
    crossingsFilter->SetMaskImage(maskImage);
    crossingsFilter->Update();
    ItkUcharImgType::Pointer numDirImage = crossingsFilter->GetNumDirectionsImage();

    typedef itk::TractsToVectorImageFilter::DirectionImageContainerType DirectionImageContainerType;
    typedef itk::TractsToVectorImageFilter::ItkDirectionImageType       DirectionImageType;
    DirectionImageContainerType::Pointer dirImgContainer = crossingsFilter->GetDirectionImageContainer();

    // generate signal
    if (m_SNR <= 0)
        m_SNR = 0.0001;
    if (m_SNR>99)
        m_NoiseVariance = 0;
    else
    {
        m_NoiseVariance = m_DefaultBaseline/m_SNR;
        m_NoiseVariance *= m_NoiseVariance;
    }
    MITK_INFO << "Noise variance: " << m_NoiseVariance;

    ItkTensorType kernel; kernel.Fill(0.0);
    float ADC = 0.001;
    vnl_vector_fixed<double, 3> kernelDir; kernelDir[0]=1; kernelDir[1]=0; kernelDir[2]=0;

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
            m_DiffusionImage->TransformPhysicalPointToIndex(vertex, index);

            if (!m_ImageRegion.IsInside(index))
                continue;

            // prepare kernel
            float td = m_TractDensityImage->GetPixel(index);

            if (numDirImage->GetPixel(index)>1) // crossing fiber, adjust tract density
            {
                float maxMagn = 0;
                float magn = td;
                float minAngle = 0;
                for (int i=0; i<dirImgContainer->Size(); i++)
                {
                    DirectionImageType::Pointer dirImg = dirImgContainer->GetElement(i);
                    vnl_vector_fixed<double, 3> crossingDir = GetVnlVector(dirImg->GetPixel(index));

                    float crossingMagn = crossingDir.magnitude();
                    if (crossingMagn>maxMagn)
                        maxMagn = crossingMagn;
                    float angle = fabs(dot_product(crossingDir, dir));
                    if (angle>minAngle)
                    {
                        magn = crossingMagn;
                        minAngle = angle;
                    }
                }
                if (maxMagn>0)
                    td *= magn/maxMagn;
            }

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
            doubleDwi->SetPixel(index, doublePix + SimulateMeasurement(tensor, 1));
        }
    }

    // find maximum baseline value
    typedef ImageRegionIterator<DoubleDwiType>      IteratorOutputType;
    IteratorOutputType it (doubleDwi, m_ImageRegion);
    while(!it.IsAtEnd())
    {
        doublePix = it.Get();
        for( unsigned int i=0; i<m_GradientList.size(); i++)
            if (m_GradientList.at(i).GetNorm()<0.0001 && doublePix[i]>m_MaxBaseline)
                m_MaxBaseline = doublePix[i];
        ++it;
    }

    // add isotropic diffusion, noise and rescale image
    itk::DiffusionTensor3D<float> freeWaterTensor;
    freeWaterTensor.Fill(0);
    freeWaterTensor.SetElement(0,m_GreyMatterAdc);
    freeWaterTensor.SetElement(3,m_GreyMatterAdc);
    freeWaterTensor.SetElement(5,m_GreyMatterAdc);

    itk::DiffusionTensor3D<float> freeCompartmentTensor;
    freeCompartmentTensor.Fill(0);
    freeCompartmentTensor.SetElement(0,ADC);
    freeCompartmentTensor.SetElement(3,ADC);
    freeCompartmentTensor.SetElement(5,ADC);

    it.GoToBegin();
    while(!it.IsAtEnd())
    {
        DoubleDwiType::IndexType index = it.GetIndex();
        doublePix = it.Get();
        bool simulateIsotropicDiffusion = false;
        bool addIsotropicCompartment = false;
        for( unsigned int i=0; i<m_GradientList.size(); i++)
            if (m_GradientList.at(i).GetNorm()<0.0001)
            {
                if (doublePix[i]<=m_MaxBaseline/100)
                {
                    if (doublePix[i]<=0)
                        simulateIsotropicDiffusion = true;
                    else
                        addIsotropicCompartment = true;
                    break;
                }
            }
        if (addIsotropicCompartment)
            doublePix += SimulateMeasurement(freeWaterTensor, m_MaxBaseline/100);
        else if (simulateIsotropicDiffusion)
            doublePix = SimulateMeasurement(freeWaterTensor, 2*m_MaxBaseline);

        doublePix = m_SignalScale*doublePix/m_MaxBaseline;
        AddNoise(doublePix);
        m_DiffusionImage->SetPixel(index, doublePix);
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
