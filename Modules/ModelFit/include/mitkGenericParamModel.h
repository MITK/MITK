/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericParamModel_h
#define mitkGenericParamModel_h

#include <mutex>

#include <mitkFormulaParser.h>
#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class GenericParamModel
   * \brief Model that uses a user-specified formula string as its model function.
   *
   * The parser used to interpret the string can handle simple mathematical formulas
   * (e.g. "3.5 + a * x * sin(x) - 1 / 2"). It recognizes:
   * - sums, differences, products and divisions (a + b, 4 - 3, 2 * x, 9 / 3)
   * - algebraic signs (+5, -5)
   * - exponentiation (2 ^ 4)
   * - parentheses (3 * (4 + 2))
   * - unary functions: abs, exp, sin, cos, tan, sind, cosd, tand
   * - variables (x, a, b, ... j)
   *
   * \note The variable "x" is reserved for the signal position / time point.
   * \note The current version supports up to 10 model parameters (a through j).
   *       Do not use "x" as a model parameter; it is the independent variable.
   *
   * \sa GenericParamModelFactory, GenericParamModelParameterizer, FormulaParser
   */
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

    /** \pre The number of parameters must be between 1 and 10. */
    itkSetClampMacro(NumberOfParameters, ParametersSizeType, 1, 10);

    std::string GetXName() const override;

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;

    ParameterNamesType GetStaticParameterNames() const override;

    ParametersSizeType GetNumberOfStaticParameters() const override;

  protected:
    GenericParamModel();
    ~GenericParamModel() override {};

    GenericParamModel(const GenericParamModel& source);

    mitkCloneMacro(GenericParamModel);

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:
    /**Function string that should be parsed when computing the model function.*/
    FunctionStringType m_FunctionString;

    /**Number of parameters the model should offer / the function string contains.*/
    ParametersSizeType m_NumberOfParameters;

    /**Variable look-up table and parser reused across evaluations, so the function
     * string is compiled only once per instance. Guarded by m_FormulaParserMutex to
     * keep concurrent evaluations on the same instance safe. Deliberately not copied
     * by the copy constructor: each instance's parser must be bound to its own map.*/
    mutable FormulaParser::VariableMapType m_VariableMap;
    mutable FormulaParser m_FormulaParser{ &m_VariableMap };
    mutable std::mutex m_FormulaParserMutex;

    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
