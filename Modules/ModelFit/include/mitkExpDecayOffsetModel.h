/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExpDecayOffsetModel_h
#define mitkExpDecayOffsetModel_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class ExpDecayOffsetModel
   * \brief Exponential decay model with baseline offset: y(x) = y0 * exp(-k*x) + y_bl.
   *
   * This model implements an exponential decay function with an additional baseline
   * offset parameter. It has three parameters: y-intercept (y0), rate constant (k),
   * and baseline (y_bl).
   *
   * \sa ExpDecayOffsetModelFactory, ExpDecayOffsetModelParameterizer, ModelBase
   */
  class MITKMODELFIT_EXPORT ExpDecayOffsetModel : public mitk::ModelBase
  {

  public:
    typedef ExpDecayOffsetModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(ExpDecayOffsetModel, ModelBase);

    /** \brief Name string for the y-intercept parameter. */
    static const std::string NAME_PARAMETER_y0;
    /** \brief Name string for the rate constant parameter. */
    static const std::string NAME_PARAMETER_k;
    /** \brief Name string for the baseline parameter. */
    static const std::string NAME_PARAMETER_y_bl;

    /** \brief Total number of model parameters (3). */
    static const unsigned int NUMBER_OF_PARAMETERS;

    /** \brief Unit of the y-intercept parameter. */
    static const std::string UNIT_PARAMETER_y0;
    /** \brief Unit of the rate constant parameter. */
    static const std::string UNIT_PARAMETER_k;
    /** \brief Unit of the baseline parameter. */
    static const std::string UNIT_PARAMETER_y_bl;

    /** \brief Index position of y-intercept in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_y0;
    /** \brief Index position of rate constant in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_k;
    /** \brief Index position of baseline in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_y_bl;

    /** \brief Total number of static parameters (0). */
    static const unsigned int NUMBER_OF_STATIC_PARAMETERS;

    /** \brief Display name of this model. */
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

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;

    ParameterNamesType GetStaticParameterNames() const override;

    ParametersSizeType GetNumberOfStaticParameters() const override;

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    FunctionStringType GetFunctionString() const override;

    std::string GetXName() const override;

    std::string GetXAxisName() const override;

    std::string GetXAxisUnit() const override;

    std::string GetYAxisName() const override;

    std::string GetYAxisUnit() const override;

  protected:
    ExpDecayOffsetModel() {};
    ~ExpDecayOffsetModel() override {};

    ExpDecayOffsetModel(const ExpDecayOffsetModel& source);

    mitkCloneMacro(ExpDecayOffsetModel);

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:

    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
