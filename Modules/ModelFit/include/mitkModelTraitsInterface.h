/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelTraitsInterface_h
#define mitkModelTraitsInterface_h

#include <MitkModelFitExports.h>

#include <itkArray.h>
#include <itkArray2D.h>
#include <itkObject.h>

namespace mitk
{
  /**
   * \class ModelTraitsInterface
   * \brief Pure virtual interface that defines the traits (metadata) of a model used for fitting.
   *
   * This interface provides a common contract for querying model properties such as
   * parameter names, units, scales, derived parameter information, axis labels,
   * and model identification strings. All model-related classes that need to expose
   * these traits (e.g. ModelBase, ModelFactoryBase, ModelParameterizerBase) implement
   * this interface.
   *
   * \sa ModelBase, ModelFactoryBase, ModelParameterizerBase
   */
  class MITKMODELFIT_EXPORT ModelTraitsInterface
  {
  public:

    /** \brief Type representing the model output signal as an array of doubles. */
    typedef itk::Array<double> ModelResultType;
    /** \brief Type of a single parameter value. */
    using ParameterValueType = double;
    /** \brief Type representing a vector of parameter values. */
    typedef itk::Array<ParameterValueType> ParametersType;

    /** \brief Type used for parameter names. */
    typedef std::string ParameterNameType;
    /** \brief Type for a list of parameter names. */
    typedef std::vector<ParameterNameType> ParameterNamesType;
    /** \brief Type for the count of parameters. */
    typedef ParametersType::SizeValueType ParametersSizeType;
    /** \brief Type for a list of derived parameter names. */
    typedef ParameterNamesType DerivedParameterNamesType;
    /** \brief Type for the count of derived parameters. */
    typedef ParametersSizeType DerivedParametersSizeType;

    /** \brief Map type associating parameter names to their scaling factors. */
    typedef std::map<ParameterNameType, double> ParamterScaleMapType;
    /** \brief Map type associating parameter names to their unit strings. */
    typedef std::map<ParameterNameType, std::string> ParamterUnitMapType;
    /** \brief Map type associating derived parameter names to their scaling factors. */
    typedef std::map<ParameterNameType, double> DerivedParamterScaleMapType;
    /** \brief Map type associating derived parameter names to their unit strings. */
    typedef std::map<ParameterNameType, std::string> DerivedParamterUnitMapType;

    /** \brief Type for the function string representation of the model. */
    typedef std::string FunctionStringType;
    /** \brief Type for the unique class identifier of the model. */
    typedef std::string ModellClassIDType;

    /**
     * \brief Returns the names of parameters that must be defined when using
     * the model to compute the signal (GetSignal()).
     * \return A vector of parameter name strings.
     */
    virtual ParameterNamesType GetParameterNames() const = 0;

    /**
     * \brief Returns the number of parameters that must be defined when using
     * the model to compute the signal (GetSignal()).
     * \return The number of model parameters.
     */
    virtual ParametersSizeType GetNumberOfParameters() const = 0;

    /**
     * \brief Returns the scaling factors for all model parameters.
     * \return A map from parameter name to its scale factor.
     */
    virtual ParamterScaleMapType GetParameterScales() const = 0;

    /**
     * \brief Returns the unit strings for all model parameters.
     * \return A map from parameter name to its unit string.
     */
    virtual ParamterUnitMapType GetParameterUnits() const = 0;

    /**
     * \brief Returns the names of derived parameters that can be computed by the model.
     * \return A vector of derived parameter name strings.
     */
    virtual DerivedParameterNamesType GetDerivedParameterNames() const = 0;

    /**
     * \brief Returns the number of derived parameters that can be computed by the model.
     * \return The number of derived parameters.
     */
    virtual DerivedParametersSizeType GetNumberOfDerivedParameters() const = 0;

    /**
     * \brief Returns the scaling factors for all derived parameters.
     * \return A map from derived parameter name to its scale factor.
     */
    virtual DerivedParamterScaleMapType GetDerivedParameterScales() const = 0;

    /**
     * \brief Returns the unit strings for all derived parameters.
     * \return A map from derived parameter name to its unit string.
     */
    virtual DerivedParamterUnitMapType GetDerivedParameterUnits() const = 0;

    /**
     * \brief Returns the human-readable display name of the model.
     * \return The model display name.
     */
    virtual std::string GetModelDisplayName() const = 0;

    /**
     * \brief Returns the type/category string of the model (e.g. "Linear", "Pharmacokinetic").
     * \return The model type string.
     */
    virtual std::string GetModelType() const = 0;

    /**
     * \brief Returns a parsable function string representing the model equation.
     * \return The function string (e.g. "a*x+b").
     */
    virtual FunctionStringType GetFunctionString() const = 0;

    /**
     * \brief Returns the unique class identifier for this model type.
     * \return The model class ID string.
     */
    virtual ModellClassIDType GetClassID() const = 0;

    /**
     * \brief Returns the name of the independent variable used by the model (e.g. "x").
     * \return The variable name string.
     */
    virtual std::string GetXName() const = 0;

    /**
     * \brief Returns the display name of the x-axis (e.g. "Time").
     * \return The x-axis name string.
     */
    virtual std::string GetXAxisName() const = 0;

    /**
     * \brief Returns the unit of the x-axis (e.g. "s", "ms").
     * \return The x-axis unit string.
     */
    virtual std::string GetXAxisUnit() const = 0;

    /**
     * \brief Returns the display name of the y-axis (e.g. "Signal Intensity").
     * \return The y-axis name string.
     */
    virtual std::string GetYAxisName() const = 0;

    /**
     * \brief Returns the unit of the y-axis.
     * \return The y-axis unit string.
     */
    virtual std::string GetYAxisUnit() const = 0;

  protected:
    ModelTraitsInterface() {};
    virtual ~ModelTraitsInterface() {};

  private:

    //No copy constructor allowed
    ModelTraitsInterface(const ModelTraitsInterface& source);
    void operator=(const ModelTraitsInterface&);  //purposely not implemented
  };
}

#endif
