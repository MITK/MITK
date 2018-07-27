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

    virtual ParametersType GetDefaultInitialParameterization() const override;

  protected:
    T2DecayModelParameterizer();

    virtual ~T2DecayModelParameterizer();

  private:

    //No copy constructor allowed
    T2DecayModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __LINEAR_MODEL_PARAMETERIZER_H
