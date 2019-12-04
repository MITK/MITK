/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __GENERIC_PARAM_MODEL_PARAMETERIZER_H
#define __GENERIC_PARAM_MODEL_PARAMETERIZER_H

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkGenericParamModel.h"

namespace mitk
{
  /** Parameterizer for the GenricParamModel.
  */
  class MITKMODELFIT_EXPORT GenericParamModelParameterizer : public ConcreteModelParameterizerBase
    <GenericParamModel>
  {
  public:
    typedef GenericParamModelParameterizer Self;
    typedef ConcreteModelParameterizerBase<GenericParamModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(GenericParamModelParameterizer, ConcreteModelParameterizerBase);
    itkFactorylessNewMacro(Self);

    typedef typename Superclass::ModelBaseType ModelBaseType;
    typedef typename Superclass::ModelBasePointer ModelBasePointer;

    typedef typename Superclass::ModelType ModelType;
    typedef typename Superclass::ModelPointer ModelPointer;

    typedef typename Superclass::StaticParameterValueType StaticParameterValueType;
    typedef typename Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef typename Superclass::StaticParameterMapType StaticParameterMapType;


    typedef typename Superclass::IndexType IndexType;

    itkSetMacro(FunctionString, mitk::ModelBase::FunctionStringType);

    /**@pre The Number of paremeters must be between 1 and 10.*/
    itkSetClampMacro(NumberOfParameters, ParametersSizeType, 1, 10);

    mitk::ModelBase::FunctionStringType GetFunctionString() const override;

    using Superclass::GenerateParameterizedModel;
    ModelBasePointer GenerateParameterizedModel(const IndexType& currentPosition) const override;

    StaticParameterMapType GetGlobalStaticParameters() const override;

    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    GenericParamModelParameterizer();
    ~GenericParamModelParameterizer() override;

    mitk::ModelBase::FunctionStringType m_FunctionString;

    /**Number of parameters the model should offer / the function string contains.*/
    ParametersSizeType m_NumberOfParameters;

  private:

    //No copy constructor allowed
    GenericParamModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };

}
#endif // __GENERIC_PARAM_MODEL_PARAMETERIZER_H
