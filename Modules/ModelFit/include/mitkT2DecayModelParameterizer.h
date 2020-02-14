/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __T2_DECAY_MODEL_PARAMETERIZER_H
#define __T2_DECAY_MODEL_PARAMETERIZER_H

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkT2DecayModel.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT T2DecayModelParameterizer : public
    mitk::ConcreteModelParameterizerBase<mitk::T2DecayModel>
  {
  public:
    typedef T2DecayModelParameterizer Self;
    typedef mitk::ConcreteModelParameterizerBase<mitk::T2DecayModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(T2DecayModelParameterizer,
      mitk::ConcreteModelParameterizerBase<mitk::T2DecayModel>);
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
    T2DecayModelParameterizer();

    ~T2DecayModelParameterizer() override;

  private:

    //No copy constructor allowed
    T2DecayModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __LINEAR_MODEL_PARAMETERIZER_H
