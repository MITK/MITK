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

#ifndef __MITK_GENERIC_PARAM_MODEL_H_
#define __MITK_GENERIC_PARAM_MODEL_H_

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

    virtual std::string GetModelDisplayName() const override;

    virtual std::string GetModelType() const override;

    virtual FunctionStringType GetFunctionString() const override;
    itkSetStringMacro(FunctionString);

    /**@pre The Number of paremeters must be between 1 and 10.*/
    itkSetClampMacro(NumberOfParameters, ParametersSizeType, 1, 10);

    virtual std::string GetXName() const override;

    virtual ParameterNamesType GetParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfParameters() const override;

    virtual ParameterNamesType GetStaticParameterNames() const override;

    virtual ParametersSizeType GetNumberOfStaticParameters() const override;

  protected:
    GenericParamModel();
    virtual ~GenericParamModel() {};

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    virtual itk::LightObject::Pointer InternalClone() const;

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const;

    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values);
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const;

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