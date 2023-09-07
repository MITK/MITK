/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExponentialDecayModelParameterizer_h
#define mitkExponentialDecayModelParameterizer_h

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkExponentialDecayModel.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExponentialDecayModelParameterizer : public
    mitk::ConcreteModelParameterizerBase<mitk::ExponentialDecayModel>
  {
  public:
    typedef ExponentialDecayModelParameterizer Self;
    typedef mitk::ConcreteModelParameterizerBase<mitk::ExponentialDecayModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ExponentialDecayModelParameterizer,
      mitk::ConcreteModelParameterizerBase<mitk::ExponentialDecayModel>);
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
    ExponentialDecayModelParameterizer();

    ~ExponentialDecayModelParameterizer() override;

  private:

    //No copy constructor allowed
    ExponentialDecayModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
