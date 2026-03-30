/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExponentialSaturationModel_h
#define mitkExponentialSaturationModel_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class ExponentialSaturationModel
   * \brief Exponential saturation model with bolus arrival time.
   *
   * This model has the form: if x < onset: y(x) = baseline, else:
   * y(x) = baseline + (y_final - baseline) * (1 - exp(-rate * (x - onset))).
   * Parameters are bolus arrival time (BAT/onset), baseline (y_bl), final value (y_fin),
   * and rate constant (k).
   *
   * \sa ExponentialSaturationModelFactory, ExponentialSaturationModelParameterizer, ModelBase
   */
  class MITKMODELFIT_EXPORT ExponentialSaturationModel : public mitk::ModelBase
  {

  public:
    typedef ExponentialSaturationModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(ExponentialSaturationModel, ModelBase);

    /** \brief Name string for the bolus arrival time (onset) parameter. */
    static const std::string NAME_PARAMETER_BAT;
    /** \brief Name string for the baseline parameter. */
    static const std::string NAME_PARAMETER_y_bl;
    /** \brief Name string for the final value parameter. */
    static const std::string NAME_PARAMETER_y_fin;
    /** \brief Name string for the rate constant parameter. */
    static const std::string NAME_PARAMETER_k;

    /** \brief Total number of model parameters (4). */
    static const unsigned int NUMBER_OF_PARAMETERS;

    /** \brief Unit of the bolus arrival time parameter. */
    static const std::string UNIT_PARAMETER_BAT;
    /** \brief Unit of the baseline parameter. */
    static const std::string UNIT_PARAMETER_y_bl;
    /** \brief Unit of the final value parameter. */
    static const std::string UNIT_PARAMETER_y_fin;
    /** \brief Unit of the rate constant parameter. */
    static const std::string UNIT_PARAMETER_k;

    /** \brief Index position of BAT in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_BAT;
    /** \brief Index position of baseline in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_y_bl;
    /** \brief Index position of final value in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_y_fin;
    /** \brief Index position of rate constant in the parameter vector. */
    static const unsigned int POSITION_PARAMETER_k;

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
    ExponentialSaturationModel() {};
    ~ExponentialSaturationModel() override {};

    ExponentialSaturationModel(const ExponentialSaturationModel& source);

    mitkCloneMacro(ExponentialSaturationModel);

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:

    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
