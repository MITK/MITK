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

template< class ScalarType >
class MitkDiffusionImaging_EXPORT TensorModel : public DiffusionSignalModel< ScalarType >
{
public:

    TensorModel();
    ~TensorModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef itk::DiffusionTensor3D< float >                             ItkTensorType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;

    PixelType SimulateMeasurement();
    void SetKernelDirection(vnl_vector_fixed<double, 3> kernelDirection){ m_KernelDirection = kernelDirection; }
    void SetKernelTensor(ItkTensorType& tensor);
    void SetKernelFA(float FA);
    void SetKernelADC(float ADC);
    void SetBvalue(float bValue) { m_BValue = bValue; }

protected:

    void UpdateKernelTensor();

    vnl_vector_fixed<double, 3>     m_KernelDirection;
    vnl_matrix_fixed<double, 3, 3>  m_KernelTensorMatrix;
    float                           m_KernelFA;
    float                           m_KernelADC;
    float                           m_BValue;
};

}

#include "mitkTensorModel.cpp"

#endif

