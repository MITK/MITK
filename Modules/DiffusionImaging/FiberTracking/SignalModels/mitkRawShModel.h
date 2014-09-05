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

#ifndef _MITK_RawShModel_H
#define _MITK_RawShModel_H

#include <mitkDiffusionSignalModel.h>
#include <mitkDiffusionImage.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>

namespace mitk {

/**
  * \brief The spherical harmonic representation of a prototype diffusion weighted MR signal is used to obtain the direction dependent signal.
  *
  */

template< class ScalarType = double >
class RawShModel : public DiffusionSignalModel< ScalarType >
{
public:

    RawShModel();
    template< class OtherType >RawShModel(RawShModel<OtherType>* model)
    {
        this->m_CompartmentId = model->m_CompartmentId;
        this->m_T2 = model->GetT2();
        this->m_FiberDirection = model->GetFiberDirection();
        this->m_GradientList = model->GetGradientList();
        this->m_VolumeFractionImage = model->GetVolumeFractionImage();
        this->m_RandGen = model->GetRandomGenerator();

        this->m_AdcRange = model->GetAdcRange();
        this->m_FaRange = model->GetFaRange();
        this->m_ShCoefficients = model->GetShCoefficients();
        this->m_B0Signal = model->GetB0Signals();
        this->m_SphCoords = model->GetSphericalCoordinates();
        this->m_ShOrder = model->GetShOrder();
        this->m_ModelIndex = model->GetModelIndex();
        this->m_MaxNumKernels = model->GetMaxNumKernels();
    }
    ~RawShModel();

    typedef itk::Image< double, 3 >                                         ItkDoubleImageType;
    typedef itk::Image< unsigned char, 3 >                                  ItkUcharImageType;
    typedef itk::Image< itk::DiffusionTensor3D< double >, 3 >               TensorImageType;
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,2,QBALL_ODFSIZE> QballFilterType;
    typedef typename DiffusionSignalModel< ScalarType >::PixelType          PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType       GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;
    typedef itk::Matrix< double, 3, 3 >                                     MatrixType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    bool SetShCoefficients(vnl_vector< double > shCoefficients, double b0);
    void SetFiberDirection(GradientType fiberDirection);
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }
    void SetFaRange(double min, double max){ m_FaRange.first = min; m_FaRange.second = max; }
    void SetAdcRange(double min, double max){ m_AdcRange.first = min; m_AdcRange.second = max; }
    void SetMaxNumKernels(unsigned int max){ m_MaxNumKernels = max; }
    unsigned int GetNumberOfKernels();
    std::pair< double, double > GetFaRange(){ return m_FaRange; }
    std::pair< double, double > GetAdcRange(){ return m_AdcRange; }
    unsigned int GetMaxNumKernels(){ return m_MaxNumKernels; }
    void Clear();

    std::vector< vnl_vector< double > > GetShCoefficients(){ return m_ShCoefficients; }
    std::vector< double > GetB0Signals(){ return m_B0Signal; }
    vnl_matrix<double> GetSphericalCoordinates(){ return m_SphCoords; }
    unsigned int GetShOrder(){ return m_ShOrder; }
    int GetModelIndex(){ return m_ModelIndex; }

    double GetBaselineSignal(int index){ return m_B0Signal.at(index); }
    vnl_vector< double > GetCoefficients(int listIndex){ return m_ShCoefficients.at(listIndex); }

    bool SampleKernels(DiffusionImage<short>::Pointer diffImg, ItkUcharImageType::Pointer maskImage, TensorImageType::Pointer tensorImage=NULL, QballFilterType::CoefficientImageType::Pointer coeffImage=NULL, ItkDoubleImageType::Pointer adcImage=NULL);

protected:

    void Cart2Sph( GradientListType gradients );
    void RandomModel();

    std::vector< vnl_vector< double > > m_ShCoefficients;
    std::vector< double >               m_B0Signal;
    std::vector< GradientType >         m_PrototypeMaxDirection;
    vnl_matrix<double>                  m_SphCoords;
    std::pair< double, double >         m_AdcRange;
    std::pair< double, double >         m_FaRange;
    unsigned int                        m_ShOrder;
    int                                 m_ModelIndex;
    unsigned int                        m_MaxNumKernels;
};

}

#include "mitkRawShModel.cpp"

#endif

