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

template< class ScalarType = double >
class TensorModel : public DiffusionSignalModel< ScalarType >
{
public:

    TensorModel();
    template< class OtherType >TensorModel(TensorModel<OtherType>* model)
    {
        this->m_CompartmentId = model->m_CompartmentId;
        this->m_T2 = model->GetT2();
        this->m_FiberDirection = model->GetFiberDirection();
        this->m_GradientList = model->GetGradientList();
        this->m_VolumeFractionImage = model->GetVolumeFractionImage();
        this->m_RandGen = model->GetRandomGenerator();

        this->m_BValue = model->GetBvalue();
        this->m_KernelDirection = model->GetKernelDirection();
        this->m_KernelTensorMatrix = model->GetKernelTensorMatrix();
    }
    ~TensorModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef itk::DiffusionTensor3D< ScalarType >                        ItkTensorType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    void SetBvalue(double bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    double GetBvalue() { return m_BValue; }
    void SetDiffusivity1(double d1){ m_KernelTensorMatrix[0][0] = d1; }
    void SetDiffusivity2(double d2){ m_KernelTensorMatrix[1][1] = d2; }
    void SetDiffusivity3(double d3){ m_KernelTensorMatrix[2][2] = d3; }
    double GetDiffusivity1() { return m_KernelTensorMatrix[0][0]; }
    double GetDiffusivity2() { return m_KernelTensorMatrix[1][1]; }
    double GetDiffusivity3() { return m_KernelTensorMatrix[2][2]; }

    void SetFiberDirection(GradientType fiberDirection){ this->m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }
    GradientType GetKernelDirection(){ return m_KernelDirection; }
    vnl_matrix_fixed<double, 3, 3> GetKernelTensorMatrix(){ return m_KernelTensorMatrix; }

protected:

    /** Calculates tensor matrix from FA and ADC **/
    void UpdateKernelTensor();
    GradientType                        m_KernelDirection;      ///< Direction of the kernel tensors principal eigenvector
    vnl_matrix_fixed<double, 3, 3>      m_KernelTensorMatrix;   ///< 3x3 matrix containing the kernel tensor values
    double                              m_BValue;               ///< b-value used to generate the artificial signal
};

}

#include "mitkTensorModel.cpp"

#endif

