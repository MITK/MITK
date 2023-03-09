/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericParamModel_h
#define mitkGenericParamModel_h

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** Model that can parse a user specified function string and uses it as model function
  that is represented by the model instance.
  The parser used to interpret the string can handle simple mathematical formulas (e.g. "3.5 + a * x * sin(x) - 1 / 2").
  The parser is able to recognize:
  - sums, differences, products and divisions (a + b, 4 - 3, 2 * x, 9 / 3)
  - algebraic signs ( +5, -5)
  - exponentiation ( 2 ^ 4 )
  - parentheses (3 * (4 + 2))
  - following unary functions: abs, exp, sin, cos, tan, sind (sine in degrees), cosd (cosine in degrees), tand (tangent in degrees)
  - variables (x, a, b, ... j)

  Remark: The variable "x" is reserved. It is the signal position / timepoint.
  Remark: The current version supports up to 10 model parameter.
  Don't use it for a model parameter that should be deduced by fitting (these are a..j).*/
  class MITKMODELFIT_EXPORT GenericParamModel : public mitk::ModelBase
  {

  public:
    typedef GenericParamModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(GenericParamModel, ModelBase);

    static const std::string NAME_STATIC_PARAMETER_number;

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    FunctionStringType GetFunctionString() const override;
    itkSetStringMacro(FunctionString);

    /**@pre The Number of paremeters must be between 1 and 10.*/
    itkSetClampMacro(NumberOfParameters, ParametersSizeType, 1, 10);

    std::string GetXName() const override;

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParameterNamesType GetStaticParameterNames() const override;

    ParametersSizeType GetNumberOfStaticParameters() const override;

  protected:
    GenericParamModel();
    ~GenericParamModel() override {};

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:
    /**Function string that should be parsed when computing the model function.*/
    FunctionStringType m_FunctionString;

    /**Number of parameters the model should offer / the function string contains.*/
    ParametersSizeType m_NumberOfParameters;

    //No copy constructor allowed
    GenericParamModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
