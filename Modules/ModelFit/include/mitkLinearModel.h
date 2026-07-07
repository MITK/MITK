/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLinearModel_h
#define mitkLinearModel_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class LinearModel
   * \brief Model representing a simple linear function: y(x) = b * x + y0.
   *
   * This model implements a linear relationship between the independent variable x
   * and the dependent variable y. It has two parameters (slope b and intercept y0)
   * and one derived parameter (x-intercept = -y0/b).
   *
   * \sa LinearModelFactory, LinearModelParameterizer, ModelBase
   */
  class MITKMODELFIT_EXPORT LinearModel : public mitk::ModelBase
  {

  public:
    typedef LinearModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(LinearModel, ModelBase);

    /** \brief Name string for the y-intercept parameter ("y-intercept"). */
    static const std::string NAME_PARAMETER_y0;
    /** \brief Name string for the slope parameter ("slope"). */
    static const std::string NAME_PARAMETER_b;

    /** \brief Total number of model parameters (2). */
    static const unsigned int NUMBER_OF_PARAMETERS;

    /** \brief Unit of the y-intercept parameter. */
    static const std::string UNIT_PARAMETER_y0;
    /** \brief Unit of the slope parameter. */
    static const std::string UNIT_PARAMETER_b;

    /** \brief Index position of y-intercept in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_y0;
    /** \brief Index position of slope in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_b;

    /** \brief Name string for the derived x-intercept parameter. */
    static const std::string NAME_DERIVED_PARAMETER_x_intercept;

    /** \brief Total number of derived parameters (1). */
    static const unsigned int NUMBER_OF_DERIVED_PARAMETERS;

    /** \brief Unit of the derived x-intercept parameter. */
    static const std::string UNIT_DERIVED_PARAMETER_x_intercept;

    /** \brief Total number of static parameters (0). */
    static const unsigned int NUMBER_OF_STATIC_PARAMETERS;

    /** \brief Display name of this model ("Linear Model"). */
    static const std::string MODEL_DISPLAY_NAME;

    /** \brief Type category of this model. */
    static const std::string MODEL_TYPE;

    /** \brief Function string representation of the model equation. */
    static const std::string FUNCTION_STRING;

    /** \brief Name of the independent variable. */
    static const std::string X_NAME;

    /** \brief Display name for the x-axis. */
    static const std::string X_AXIS_NAME;

    /** \brief Unit for the x-axis. */
    static const std::string X_AXIS_UNIT;

    /** \brief Display name for the y-axis. */
    static const std::string Y_AXIS_NAME;

    /** \brief Unit for the y-axis. */
    static const std::string Y_AXIS_UNIT;

    /** \brief Returns the names of all model parameters.
     * \return Vector containing "y-intercept" and "slope". */
    ParameterNamesType GetParameterNames() const override;

    /** \brief Returns the number of model parameters (2).
     * \return 2 */
    ParametersSizeType  GetNumberOfParameters() const override;

    /** \brief Returns the unit map for model parameters.
     * \return Map of parameter names to their unit strings. */
    ParamterUnitMapType GetParameterUnits() const override;

    /** \brief Returns the names of derived parameters.
     * \return Vector containing "x-intercept". */
    ParameterNamesType GetDerivedParameterNames() const override;

    /** \brief Returns the number of derived parameters (1).
     * \return 1 */
    ParametersSizeType  GetNumberOfDerivedParameters() const override;

    /** \brief Returns the unit map for derived parameters.
     * \return Map of derived parameter names to their unit strings. */
    ParamterUnitMapType GetDerivedParameterUnits() const override;

    /** \brief Returns the names of static parameters (empty for this model).
     * \return Empty vector. */
    ParameterNamesType GetStaticParameterNames() const override;

    /** \brief Returns the number of static parameters (0).
     * \return 0 */
    ParametersSizeType GetNumberOfStaticParameters() const override;

    /** \brief Returns the display name "Linear Model".
     * \return The model display name. */
    std::string GetModelDisplayName() const override;

    /** \brief Returns the model type string.
     * \return The model type. */
    std::string GetModelType() const override;

    /** \brief Returns the function string representing the model equation.
     * \return The function string. */
    FunctionStringType GetFunctionString() const override;

    /** \brief Returns the name of the independent variable.
     * \return The x variable name. */
    std::string GetXName() const override;

    /** \brief Returns the x-axis display name.
     * \return The x-axis name. */
    std::string GetXAxisName() const override;

    /** \brief Returns the x-axis unit.
     * \return The x-axis unit string. */
    std::string GetXAxisUnit() const override;

    /** \brief Returns the y-axis display name.
     * \return The y-axis name. */
    std::string GetYAxisName() const override;

    /** \brief Returns the y-axis unit.
     * \return The y-axis unit string. */
    std::string GetYAxisUnit() const override;


  protected:
    LinearModel() {};
    ~LinearModel() override {};

    LinearModel(const LinearModel& source);

    mitkCloneMacro(LinearModel);

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;
    DerivedParameterMapType ComputeDerivedParameters(const mitk::ModelBase::ParametersType&
        parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:

    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
