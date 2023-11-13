/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExpDecayOffsetModel_h
#define mitkExpDecayOffsetModel_h

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** @class ExpDecayOffsetModel
   * @brief Implementation of a general exponential decay model with offset,
   * following the function: y(x) = y-intercept * exp(-rate*x) + baseline.
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

    static const std::string NAME_PARAMETER_y0;
    static const std::string NAME_PARAMETER_k;
    static const std::string NAME_PARAMETER_y_bl;

    static const unsigned int NUMBER_OF_PARAMETERS;

    static const std::string UNIT_PARAMETER_y0;
    static const std::string UNIT_PARAMETER_k;
    static const std::string UNIT_PARAMETER_y_bl;

    static const unsigned int POSITION_PARAMETER_y0;
    static const unsigned int POSITION_PARAMETER_k;
    static const unsigned int POSITION_PARAMETER_y_bl;

    static const unsigned int NUMBER_OF_STATIC_PARAMETERS;

    static const std::string MODEL_DISPLAY_NAME;

    static const std::string MODEL_TYPE;

    static const std::string FUNCTION_STRING;

    static const std::string X_NAME;

    static const std::string X_AXIS_NAME;

    static const std::string X_AXIS_UNIT;

    static const std::string Y_AXIS_NAME;

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

    //No copy constructor allowed
    ExpDecayOffsetModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
