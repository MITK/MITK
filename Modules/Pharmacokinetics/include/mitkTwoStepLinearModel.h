/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoStepLinearModel_h
#define mitkTwoStepLinearModel_h

#include "mitkModelBase.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT TwoStepLinearModel : public mitk::ModelBase
  {

  public:
    typedef TwoStepLinearModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(TwoStepLinearModel, ModelBase)

    static const std::string MODELL_NAME;
    static const std::string NAME_PARAMETER_y1;
    static const std::string NAME_PARAMETER_t;
    static const std::string NAME_PARAMETER_a1;
    static const std::string NAME_PARAMETER_a2;

    static const unsigned int POSITION_PARAMETER_y1;
    static const unsigned int POSITION_PARAMETER_t;
    static const unsigned int POSITION_PARAMETER_a1;
    static const unsigned int POSITION_PARAMETER_a2;

    static const unsigned int NUMBER_OF_PARAMETERS;

    virtual std::string GetModelDisplayName() const override;

    virtual std::string GetModelType() const override;

    virtual FunctionStringType GetFunctionString() const override;

    virtual std::string GetXName() const override;

    virtual ParameterNamesType GetParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfParameters() const override;

    virtual ParameterNamesType GetStaticParameterNames() const override;

    virtual ParametersSizeType GetNumberOfStaticParameters() const override;

    virtual ParameterNamesType GetDerivedParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfDerivedParameters() const override;


  protected:
    TwoStepLinearModel() {};
    virtual ~TwoStepLinearModel(){};


    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    virtual itk::LightObject::Pointer InternalClone() const;

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const;
    virtual DerivedParameterMapType ComputeDerivedParameters(const mitk::ModelBase::ParametersType&
        parameters) const;

    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values);
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const;

  private:

    //No copy constructor allowed
    TwoStepLinearModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}


#endif
