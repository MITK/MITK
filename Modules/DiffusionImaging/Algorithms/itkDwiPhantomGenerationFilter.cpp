
#ifndef __itkDwiPhantomGenerationFilter_cpp
#define __itkDwiPhantomGenerationFilter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkDwiPhantomGenerationFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkOrientationDistributionFunction.h"

namespace itk {

//#define QBALL_RECON_PI       3.14159265358979323846

template< class TOutputScalarType >
DwiPhantomGenerationFilter< TOutputScalarType >
::DwiPhantomGenerationFilter()
    : m_BValue(1000)
    , m_SignalScale(1.0)
    , m_BaselineImages(0)
    , m_MaxBaseline(0)
    , m_MeanBaseline(0)
    , m_SNR(4)
    , m_NoiseFactor(0)
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
::GenerateTensors(vnl_vector<double> num)
{
    MITK_INFO << "Generating tensors";

    m_MaxBaseline = 0;
    vnl_vector<double> signal; signal.set_size(m_SignalRegions.size());
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

        double l2 = GetTensorL2Norm(kernel);
        signal[i] = l2;
        if (l2>m_MaxBaseline)
            m_MaxBaseline = l2;

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
    if (m_SNR<=99)
    {
        m_MeanBaseline = 0;
        double sum = 0;
        for (int i=0; i<m_SignalRegions.size(); i++)
        {
            m_MeanBaseline += signal[i]/m_MaxBaseline*num[i];
            sum += num[i];
        }
        if (sum>0)
            m_MeanBaseline /= sum;

        m_NoiseFactor = m_MeanBaseline/m_SNR;
        m_NoiseFactor *= m_NoiseFactor;
    }
    else
        m_NoiseFactor = 0;
}



template< class TOutputScalarType >
typename DwiPhantomGenerationFilter< TOutputScalarType >::OutputImageType::PixelType
DwiPhantomGenerationFilter< TOutputScalarType >::SimulateMeasurement(itk::DiffusionTensor3D<float>& T, float weight)
{
    typename OutputImageType::PixelType out;
    out.SetSize(m_GradientList.size());
    out.Fill(0);


    double l2 = GetTensorL2Norm(T);
    TOutputScalarType b0 = (l2/m_MaxBaseline)*m_SignalScale;

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

            double res =
                    T[0]*S[0] + T[1]*S[1] + T[2]*S[2] +
                    T[1]*S[1] + T[3]*S[3] + T[4]*S[4] +
                    T[2]*S[2] + T[4]*S[4] + T[5]*S[5];

            // check for corrupted tensor
            if (res>=0)
            {
                res = weight*b0*exp ( -m_BValue * res );
                res = sqrt(pow(res + m_SignalScale*m_RandGen->GetNormalVariate(0.0, m_NoiseFactor), 2) +  m_SignalScale*pow(m_RandGen->GetNormalVariate(0.0, m_NoiseFactor),2));

                out[i] = static_cast<TOutputScalarType>( res );
            }
        }
        else
        {
            float val = 0;
            for (int k=0; k<10; k++)
            {
                val += sqrt(pow(b0 + m_SignalScale*m_RandGen->GetNormalVariate(0.0, m_NoiseFactor), 2) +  m_SignalScale*pow(m_RandGen->GetNormalVariate(0.0, m_NoiseFactor),2));
            }
            val /= 10;
            out[i] = val;
        }
    }


    return out;
}

template< class TOutputScalarType >
void DwiPhantomGenerationFilter< TOutputScalarType >
::GenerateData()
{
    if (m_SNR<=0.001)
        m_SNR = 0.001;

    MITK_INFO << "SNR: " << m_SNR;
    MITK_INFO << "Output signal scaled by " << m_SignalScale;
    m_RandGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();

    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );   // Set the image spacing
    outImage->SetOrigin( m_Origin );     // Set the image origin
    outImage->SetDirection( m_DirectionMatrix );  // Set the image direction
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

    m_BaselineImages = 0;
    for( unsigned int i=0; i<m_GradientList.size(); i++)
        if (m_GradientList[i].GetNorm()<=0.0001)
            m_BaselineImages++;

    typedef ImageRegionIterator<OutputImageType>      IteratorOutputType;
    IteratorOutputType it (outImage, m_ImageRegion);

    // estimate mean baseline
    vnl_vector< double > num; num.set_size(m_SignalRegions.size()); num.fill(0.0);
    while(!it.IsAtEnd())
    {
        for (int i=0; i<m_SignalRegions.size(); i++)
        {
            ItkUcharImgType::Pointer region = m_SignalRegions.at(i);
            typename OutputImageType::IndexType index = it.GetIndex();
            if (region->GetPixel(index)!=0)
                num[i] += 1;
        }
        ++it;
    }
    for (int i=0; i<m_SignalRegions.size(); i++)
        num[i] *= m_TensorWeight[i];
    it.GoToBegin();

    if (m_SignalRegions.empty())
    {
        ItkUcharImgType::Pointer binary = ItkUcharImgType::New();
        binary->SetSpacing( m_Spacing );   // Set the image spacing
        binary->SetOrigin( m_Origin );     // Set the image origin
        binary->SetDirection( m_DirectionMatrix );  // Set the image direction
        binary->SetLargestPossibleRegion( m_ImageRegion );
        binary->SetBufferedRegion( m_ImageRegion );
        binary->SetRequestedRegion( m_ImageRegion );
        binary->Allocate();
        binary->FillBuffer(1);
        m_SignalRegions.push_back(binary);

        // kernel tensor
        itk::DiffusionTensor3D<float> tensor;
        float ADC = 0.001;
        tensor.Fill(0);
        float e1=ADC;
        float e2=ADC;
        float e3=e2;
        tensor.SetElement(0,e1);
        tensor.SetElement(3,e2);
        tensor.SetElement(5,e3);
        m_TensorList.push_back(tensor);
        m_TensorWeight.push_back(1);

        m_MaxBaseline = GetTensorL2Norm(tensor);
    }
    else
        GenerateTensors(num);


    while(!it.IsAtEnd())
    {
        // generate artificial signal
        int count = 0;
        for (int i=0; i<m_SignalRegions.size(); i++)
        {
            ItkUcharImgType::Pointer region = m_SignalRegions.at(i);
            typename OutputImageType::IndexType index = it.GetIndex();

            if (region->GetPixel(index)!=0)
            {
                count++;
                pix = it.Get();
                it.Set(pix+SimulateMeasurement(m_TensorList[i], m_TensorWeight[i]));
            }
        }

        if (count>1)
        {
            pix = it.Get();
            for (int i=0; i<m_BaselineImages; i++)
                pix[i] /= count;
            it.Set(pix);
        }
        ++it;
    }
}
template< class TOutputScalarType >
double DwiPhantomGenerationFilter< TOutputScalarType >::GetTensorL2Norm(itk::DiffusionTensor3D<float>& T)
{
    return sqrt(T[0]*T[0] + T[3]*T[3] + T[5]*T[5] + T[1]*T[2]*2.0 + T[2]*T[4]*2.0 + T[1]*T[4]*2.0);
}

}

#endif // __itkDwiPhantomGenerationFilter_cpp
