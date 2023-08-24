/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __EXPONENTIAL_SATURATION_MODEL_PARAMETERIZER_H
#define __EXPONENTIAL_SATURATION_MODEL_PARAMETERIZER_H

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkExponentialSaturationModel.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExponentialSaturationModelParameterizer : public
    mitk::ConcreteModelParameterizerBase<mitk::ExponentialSaturationModel>
  {
  public:
    typedef ExponentialSaturationModelParameterizer Self;
    typedef mitk::ConcreteModelParameterizerBase<mitk::ExponentialSaturationModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ExponentialSaturationModelParameterizer,
      mitk::ConcreteModelParameterizerBase<mitk::ExponentialSaturationModel>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelBaseType ModelBaseType;
    typedef Superclass::ModelBasePointer ModelBasePointer;

    typedef Superclass::ModelType ModelType;
    typedef ModelType::Pointer ModelPointer;

    typedef Superclass::StaticParameterValueType StaticParameterValueType;
    typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef  Superclass::StaticParameterMapType StaticParameterMapType;

    typedef Superclass::IndexType IndexType;

    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ExponentialSaturationModelParameterizer();

    ~ExponentialSaturationModelParameterizer() override;

  private:

    //No copy constructor allowed
    ExponentialSaturationModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __EXPONENTIAL_SATURATION_MODEL_PARAMETERIZER_H
