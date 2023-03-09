/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkThreeStepLinearModel_h
#define mitkThreeStepLinearModel_h

#include "mitkModelBase.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT ThreeStepLinearModel : public mitk::ModelBase
  {

  public:
    typedef ThreeStepLinearModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(ThreeStepLinearModel, ModelBase);

    static const std::string MODEL_DISPLAY_NAME;
    static const std::string NAME_PARAMETER_S0;
    static const std::string NAME_PARAMETER_t1;
    static const std::string NAME_PARAMETER_t2;
    static const std::string NAME_PARAMETER_a1;
    static const std::string NAME_PARAMETER_a2;
    static const std::string NAME_PARAMETER_b1;
    static const std::string NAME_PARAMETER_b2;

    static const std::string UNIT_PARAMETER_S0;
    static const std::string UNIT_PARAMETER_t1;
    static const std::string UNIT_PARAMETER_t2;
    static const std::string UNIT_PARAMETER_a1;
    static const std::string UNIT_PARAMETER_a2;
    static const std::string UNIT_PARAMETER_b1;
    static const std::string UNIT_PARAMETER_b2;

    static const unsigned int POSITION_PARAMETER_S0;
    static const unsigned int POSITION_PARAMETER_t1;
    static const unsigned int POSITION_PARAMETER_t2;
    static const unsigned int POSITION_PARAMETER_a1;
    static const unsigned int POSITION_PARAMETER_a2;
    static const unsigned int POSITION_PARAMETER_b1;
    static const unsigned int POSITION_PARAMETER_b2;

    static const unsigned int NUMBER_OF_PARAMETERS;

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    FunctionStringType GetFunctionString() const override;

    std::string GetXName() const override;

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;

    ParameterNamesType GetStaticParameterNames() const override;

    ParametersSizeType GetNumberOfStaticParameters() const override;

    ParameterNamesType GetDerivedParameterNames() const override;

    ParametersSizeType  GetNumberOfDerivedParameters() const override;

    ParamterUnitMapType GetDerivedParameterUnits() const override;


  protected:
    ThreeStepLinearModel() {};
    ~ThreeStepLinearModel() override{};


    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;
    DerivedParameterMapType ComputeDerivedParameters(const mitk::ModelBase::ParametersType&
        parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:

    //No copy constructor allowed
    ThreeStepLinearModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}


#endif
