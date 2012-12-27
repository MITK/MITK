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
#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>

template< class ScalarType >
TensorModel< ScalarType >::TensorModel()
    : m_KernelFA(0.6)
    , m_KernelADC(0.001)
    , m_BValue(1000)
{
    m_KernelDirection[0]=1; m_KernelDirection[1]=0; m_KernelDirection[2]=0;
    UpdateKernelTensor();
}

template< class ScalarType >
TensorModel< ScalarType >::~TensorModel()
{

}

template< class ScalarType >
void TensorModel< ScalarType >::UpdateKernelTensor()
{
    ItkTensorType tensor; tensor.Fill(0.0);
    float e1 = m_KernelADC*(1+2*m_KernelFA/sqrt(3-2*m_KernelFA*m_KernelFA));
    float e2 = m_KernelADC*(1-m_KernelFA/sqrt(3-2*m_KernelFA*m_KernelFA));
    tensor.SetElement(0, e1);
    tensor.SetElement(3, e2);
    tensor.SetElement(5, e2);

    m_KernelTensorMatrix.fill(0.0);
    m_KernelTensorMatrix[0][0] = e1; m_KernelTensorMatrix[1][1] = e2; m_KernelTensorMatrix[2][2] = e2;
}

template< class ScalarType >
void TensorModel< ScalarType >::SetKernelFA(float FA)
{
    m_KernelFA = FA;
    UpdateKernelTensor();
}

template< class ScalarType >
void TensorModel< ScalarType >::SetKernelADC(float ADC)
{
    m_KernelADC = ADC;
    UpdateKernelTensor();
}

template< class ScalarType >
void TensorModel< ScalarType >::SetKernelTensor(ItkTensorType& tensor)
{
    m_KernelTensorMatrix[0][0] = tensor[0]; m_KernelTensorMatrix[0][1] = tensor[1]; m_KernelTensorMatrix[0][2] = tensor[2];
    m_KernelTensorMatrix[1][0] = tensor[1]; m_KernelTensorMatrix[1][1] = tensor[3]; m_KernelTensorMatrix[1][2] = tensor[4];
    m_KernelTensorMatrix[2][0] = tensor[2]; m_KernelTensorMatrix[2][1] = tensor[4]; m_KernelTensorMatrix[2][2] = tensor[5];
}

template< class ScalarType >
typename TensorModel< ScalarType >::PixelType TensorModel< ScalarType >::SimulateMeasurement()
{
    PixelType signal; signal.SetSize(this->m_GradientList.size()); signal.Fill(0.0);

    ItkTensorType tensor; tensor.Fill(0.0);
    this->m_FiberDirection.Normalize();
    vnl_vector_fixed<double, 3> axis = itk::CrossProduct(m_KernelDirection, this->m_FiberDirection).GetVnlVector(); axis.normalize();
    vnl_quaternion<double> rotation(axis, acos(m_KernelDirection*this->m_FiberDirection));
    rotation.normalize();
    vnl_matrix_fixed<double, 3, 3> matrix = rotation.rotation_matrix_transpose();

    vnl_matrix_fixed<double, 3, 3> tensorMatrix = matrix.transpose()*m_KernelTensorMatrix*matrix;
    tensor[0] = tensorMatrix[0][0]; tensor[1] = tensorMatrix[0][1]; tensor[2] = tensorMatrix[0][2];
    tensor[3] = tensorMatrix[1][1]; tensor[4] = tensorMatrix[1][2]; tensor[5] = tensorMatrix[2][2];

    for( unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType g = this->m_GradientList[i];
        double bVal = g.GetNorm(); bVal *= bVal;

        if (bVal>0.0001)
        {
            itk::DiffusionTensor3D<float> S;
            S[0] = g[0]*g[0];
            S[1] = g[1]*g[0];
            S[2] = g[2]*g[0];
            S[3] = g[1]*g[1];
            S[4] = g[2]*g[1];
            S[5] = g[2]*g[2];

            double D = tensor[0]*S[0] + tensor[1]*S[1] + tensor[2]*S[2] +
                       tensor[1]*S[1] + tensor[3]*S[3] + tensor[4]*S[4] +
                       tensor[2]*S[2] + tensor[4]*S[4] + tensor[5]*S[5];

            // check for corrupted tensor and generate signal
            if (D>=0)
                signal[i] = exp ( -m_BValue * bVal * D );
        }
        else
            signal[i] = 1;
    }

    return signal;
}
