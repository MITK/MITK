
#ifndef __itkDwiPhantomGenerationFilter_cpp
#define __itkDwiPhantomGenerationFilter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkDwiPhantomGenerationFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkOrientationDistributionFunction.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

#include <boost/progress.hpp>

namespace itk {

//#define QBALL_RECON_PI       3.14159265358979323846

template< class TOutputScalarType >
DwiPhantomGenerationFilter< TOutputScalarType >
::DwiPhantomGenerationFilter()
    : m_BValue(1000)
    , m_SignalScale(1000)
    , m_BaselineImages(0)
    , m_MaxBaseline(0)
    , m_MeanBaseline(0)
    , m_NoiseVariance(0.004)
    , m_GreyMatterAdc(0.01)
    , m_SimulateBaseline(true)
    , m_DefaultBaseline(1000)
{
    this->SetNumberOfRequiredOutputs (1);
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);

    typename OutputImageType::Pointer outImage = OutputImageType::New();

    outImage->SetSpacing( m_Spacing );   // Set the image spacing
    outImage->SetOrigin( m_Origin );     // Set the image origin
    outImage->SetDirection( m_DirectionMatrix );  // Set the image direction
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->SetVectorLength(QBALL_ODFSIZE);
    outImage->Allocate();
    outImage->FillBuffer(0);

    this->SetNthOutput (0, outImage);

}

template< class TOutputScalarType >
void DwiPhantomGenerationFilter< TOutputScalarType >
::GenerateTensors()
{
    MITK_INFO << "Generating tensors";

    for (int i=0; i<m_SignalRegions.size(); i++)
    {
        float ADC = m_TensorADC.at(i);
        float FA = m_TensorFA.at(i);
        itk::DiffusionTensor3D<float> kernel;
        kernel.Fill(0);
        float e1=ADC*(1+2*FA/sqrt(3-2*FA*FA));
        float e2=ADC*(1-FA/sqrt(3-2*FA*FA));
        float e3=e2;
        kernel.SetElement(0,e1);
        kernel.SetElement(3,e2);
        kernel.SetElement(5,e3);

        if (m_SimulateBaseline)
        {
            double l2 = GetTensorL2Norm(kernel);
            if (l2>m_MaxBaseline)
                m_MaxBaseline = l2;
        }

        MITK_INFO << "Kernel FA: " << kernel.GetFractionalAnisotropy();
        vnl_vector_fixed<double, 3> kernelDir; kernelDir[0]=1; kernelDir[1]=0; kernelDir[2]=0;

        itk::DiffusionTensor3D<float> tensor;
        vnl_vector_fixed<double, 3> dir = m_TensorDirection.at(i);
        MITK_INFO << "Tensor direction: " << dir;

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

        m_TensorList.push_back(tensor);
    }
}

template< class TOutputScalarType >
void DwiPhantomGenerationFilter< TOutputScalarType >::AddNoise(typename OutputImageType::PixelType& pix)
{
    for( unsigned int i=0; i<m_GradientList.size(); i++)
    {
        float signal = pix[i];
        float val = sqrt(pow(signal + m_SignalScale*m_RandGen->GetNormalVariate(0.0, m_NoiseVariance), 2) +  pow(m_SignalScale*m_RandGen->GetNormalVariate(0.0, m_NoiseVariance),2));
        pix[i] += val;
    }
}

template< class TOutputScalarType >
typename DwiPhantomGenerationFilter< TOutputScalarType >::OutputImageType::PixelType
DwiPhantomGenerationFilter< TOutputScalarType >::SimulateMeasurement(itk::DiffusionTensor3D<float>& T, float weight)
{
    typename OutputImageType::PixelType out;
    out.SetSize(m_GradientList.size());
    out.Fill(0);

    TOutputScalarType s0 = m_DefaultBaseline;
    if (m_SimulateBaseline)
        s0 = (GetTensorL2Norm(T)/m_MaxBaseline)*m_SignalScale;

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
            {
                D = weight*s0*exp ( -m_BValue * D );
                out[i] = static_cast<TOutputScalarType>( D );
            }
        }
        else
            out[i] = s0;
    }

    return out;
}

template< class TOutputScalarType >
void DwiPhantomGenerationFilter< TOutputScalarType >
::GenerateData()
{
    if (m_NoiseVariance < 0)
        m_NoiseVariance = 0.001;

    if (!m_SimulateBaseline)
    {
        MITK_INFO << "Baseline image values are set to default. Noise variance value is treated as SNR!";
        if (m_NoiseVariance <= 0)
            m_NoiseVariance = 0.0001;
        if (m_NoiseVariance>99)
            m_NoiseVariance = 0;
        else
        {
            m_NoiseVariance = m_DefaultBaseline/(m_NoiseVariance*m_SignalScale);
            m_NoiseVariance *= m_NoiseVariance;
        }
    }

    m_RandGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();

    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( m_Origin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->SetVectorLength(m_GradientList.size());
    outImage->Allocate();
    typename OutputImageType::PixelType pix;
    pix.SetSize(m_GradientList.size());
    pix.Fill(0.0);
    outImage->FillBuffer(pix);
    this->SetNthOutput (0, outImage);

    double minSpacing = m_Spacing[0];
    if (m_Spacing[1]<minSpacing)
        minSpacing = m_Spacing[1];
    if (m_Spacing[2]<minSpacing)
        minSpacing = m_Spacing[2];

    m_DirectionImageContainer = ItkDirectionImageContainer::New();
    for (int i=0; i<m_SignalRegions.size(); i++)
    {
        itk::Vector< float, 3 > nullVec; nullVec.Fill(0.0);
        ItkDirectionImage::Pointer img = ItkDirectionImage::New();
        img->SetSpacing( m_Spacing );
        img->SetOrigin( m_Origin );
        img->SetDirection( m_DirectionMatrix );
        img->SetRegions( m_ImageRegion );
        img->Allocate();
        img->FillBuffer(nullVec);
        m_DirectionImageContainer->InsertElement(m_DirectionImageContainer->Size(), img);
    }
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( m_Spacing );
    m_NumDirectionsImage->SetOrigin( m_Origin );
    m_NumDirectionsImage->SetDirection( m_DirectionMatrix );
    m_NumDirectionsImage->SetRegions( m_ImageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    m_SNRImage = ItkFloatImgType::New();
    m_SNRImage->SetSpacing( m_Spacing );
    m_SNRImage->SetOrigin( m_Origin );
    m_SNRImage->SetDirection( m_DirectionMatrix );
    m_SNRImage->SetRegions( m_ImageRegion );
    m_SNRImage->Allocate();
    m_SNRImage->FillBuffer(0);

    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    m_BaselineImages = 0;
    for( unsigned int i=0; i<m_GradientList.size(); i++)
        if (m_GradientList[i].GetNorm()<=0.0001)
            m_BaselineImages++;

    typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
    IteratorOutputType it (outImage, m_ImageRegion);

    // isotropic tensor
    itk::DiffusionTensor3D<float> isoTensor;
    isoTensor.Fill(0);
    float e1 = m_GreyMatterAdc;
    float e2 = m_GreyMatterAdc;
    float e3 = m_GreyMatterAdc;
    isoTensor.SetElement(0,e1);
    isoTensor.SetElement(3,e2);
    isoTensor.SetElement(5,e3);
    m_MaxBaseline = GetTensorL2Norm(isoTensor);

    GenerateTensors();

    // simulate measurement
    m_MeanBaseline = 0;
    double noiseStdev = sqrt(m_NoiseVariance);
    while(!it.IsAtEnd())
    {
        pix = it.Get();
        typename OutputImageType::IndexType index = it.GetIndex();

        int numDirs = 0;
        for (int i=0; i<m_SignalRegions.size(); i++)
        {
            ItkUcharImgType::Pointer region = m_SignalRegions.at(i);

            if (region->GetPixel(index)!=0)
            {
                numDirs++;
                pix += SimulateMeasurement(m_TensorList[i], m_TensorWeight[i]);

                // set direction image pixel
                ItkDirectionImage::Pointer img = m_DirectionImageContainer->GetElement(i);
                itk::Vector< float, 3 > pixel = img->GetPixel(index);
                vnl_vector_fixed<double, 3> dir = m_TensorDirection.at(i);
                dir.normalize();
                dir *= m_TensorWeight.at(i);
                pixel.SetElement(0, dir[0]);
                pixel.SetElement(1, dir[1]);
                pixel.SetElement(2, dir[2]);
                img->SetPixel(index, pixel);

                vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                itk::ContinuousIndex<double, 3> center;
                center[0] = index[0];
                center[1] = index[1];
                center[2] = index[2];
                itk::Point<double> worldCenter;
                outImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );
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
            }
        }

        if (numDirs>1)
        {
            for (int i=0; i<m_GradientList.size(); i++)
                pix[i] /= numDirs;
        }
        else if (numDirs==0)
        {
            if (m_SimulateBaseline)
                pix = SimulateMeasurement(isoTensor, 1.0);
            else
                pix.Fill(0.0);
        }

        m_MeanBaseline += pix[0];
        it.Set(pix);
        m_NumDirectionsImage->SetPixel(index, numDirs);
        if (m_NoiseVariance>0)
            m_SNRImage->SetPixel(index, pix[0]/(noiseStdev*m_SignalScale));
        ++it;
    }
    m_MeanBaseline /= m_ImageRegion.GetNumberOfPixels();
    if (m_NoiseVariance>0)
        MITK_INFO << "Mean SNR: " << m_MeanBaseline/(noiseStdev*m_SignalScale);
    else
        MITK_INFO << "No noise added";

    // add rician noise
    it.GoToBegin();
    while(!it.IsAtEnd())
    {
        pix = it.Get();
        AddNoise(pix);
        it.Set(pix);
        ++it;
    }

    // generate fiber bundle
    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);
}
template< class TOutputScalarType >
double DwiPhantomGenerationFilter< TOutputScalarType >::GetTensorL2Norm(itk::DiffusionTensor3D<float>& T)
{
    return sqrt(T[0]*T[0] + T[3]*T[3] + T[5]*T[5] + T[1]*T[2]*2.0 + T[2]*T[4]*2.0 + T[1]*T[4]*2.0);
}

}

#endif // __itkDwiPhantomGenerationFilter_cpp
