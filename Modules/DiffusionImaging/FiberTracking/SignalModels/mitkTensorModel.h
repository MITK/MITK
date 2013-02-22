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

#ifndef _MITK_TensorModel_H
#define _MITK_TensorModel_H

#include <mitkDiffusionSignalModel.h>
#include <itkDiffusionTensor3D.h>

namespace mitk {

/**
  * \brief Generates  diffusion measurement employing a second rank tensor model: e^(-bg^TDg)
  *
  */

template< class ScalarType >
class TensorModel : public DiffusionSignalModel< ScalarType >
{
public:

    TensorModel();
    ~TensorModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef itk::DiffusionTensor3D< float >                             ItkTensorType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();

    void SetKernelDirection(GradientType kernelDirection){ m_KernelDirection = kernelDirection; }
    void SetKernelTensor(ItkTensorType& tensor);
    void SetKernelFA(float FA);
    void SetKernelADC(float ADC);
    void SetBvalue(float bValue) { m_BValue = bValue; }

protected:

    /** Calculates tensor matrix from FA and ADC **/
    void UpdateKernelTensor();
    GradientType                    m_KernelDirection;      ///< Direction of the kernel tensors principal eigenvector
    vnl_matrix_fixed<double, 3, 3>  m_KernelTensorMatrix;   ///< 3x3 matrix containing the kernel tensor values
    float                           m_KernelFA;             ///< FA of the kernel tensor
    float                           m_KernelADC;            ///< ADC of the kernel tensor
    float                           m_BValue;               ///< b-value used to generate the artificial signal
};

}

#include "mitkTensorModel.cpp"

#endif

