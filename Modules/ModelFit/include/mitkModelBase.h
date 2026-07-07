/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelBase_h
#define mitkModelBase_h

#include <iostream>

#include <itkArray.h>
#include <itkArray2D.h>
#include <itkObject.h>

#include <MitkModelFitExports.h>
#include <mitkModelTraitsInterface.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * \class ModelBase
   * \brief Base class for (dynamic) models.
   *
   * A model can be used to calculate its signal given the discrete time grid of the signal
   * and the parameters of the model.
   *
   * A model has 3 types of parameters:
   * - parameters
   * - static parameters
   * - derived parameters
   *
   * "Parameters" and "static parameters" are used to compute the signal of the model.
   * "Parameters" are the ones that will be changed for/by model fitting.
   * "Static parameters" are used to configure the model for fitting but are itself not
   * part of the fitting scope (compare itk::Transform parameters and static parameters).
   * "Derived parameters" are model specific parameters computed from "Parameters" e.g.
   * (DerivedParam1 = Param1/Param2). It may be implemented if e.g. for practical usage
   * not the fitted parameters are needed but derivation of them.
   *
   * \remark If you implement your own model calls regard const correctness and do not change
   * or undermine the constness of this base class. It is important because in case of fitting
   * models are used in a multi threaded environment and must be thread safe. Thus the getter and
   * computation functions are implemented as const and thread safe methods.
   *
   * \sa ModelTraitsInterface, ModelFactoryBase, ModelParameterizerBase
   */
  class MITKMODELFIT_EXPORT ModelBase : public itk::Object, public ModelTraitsInterface
  {
  public:
    typedef ModelBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ModelBase, itk::Object);

    typedef ModelTraitsInterface::ModelResultType ModelResultType;
    typedef ModelTraitsInterface::ParameterValueType ParameterValueType;
    typedef ModelTraitsInterface::ParametersType ParametersType;
    /** \brief Type defining the time grid used by models.
     * \remark The model time grid has a resolution in seconds, not milliseconds like the time geometry. */
    typedef itk::Array<double> TimeGridType;
    typedef ModelTraitsInterface::ParameterNameType ParameterNameType;
    typedef ModelTraitsInterface::ParameterNamesType ParameterNamesType;
    typedef ModelTraitsInterface::ParametersSizeType ParametersSizeType;

    typedef ModelTraitsInterface::DerivedParameterNamesType DerivedParameterNamesType;
    typedef ModelTraitsInterface::DerivedParametersSizeType DerivedParametersSizeType;

    typedef double StaticParameterValueType;
    typedef std::vector<StaticParameterValueType> StaticParameterValuesType;
    typedef std::map<ParameterNameType, StaticParameterValuesType> StaticParameterMapType;

    typedef double DerivedParameterValueType;
    typedef std::map<ParameterNameType, DerivedParameterValueType> DerivedParameterMapType;

    /** \brief Default implementation returns a scale of 1.0 for every defined parameter. */
    ParamterScaleMapType GetParameterScales() const override;

    /** \brief Default implementation returns no unit string ("") for every defined parameter. */
    ParamterUnitMapType GetParameterUnits() const override;

    /** \brief Default implementation returns a scale of 1.0 for every defined derived parameter. */
    DerivedParamterScaleMapType GetDerivedParameterScales() const override;

    /** \brief Default implementation returns no unit string ("") for every defined derived parameter. */
    DerivedParamterUnitMapType GetDerivedParameterUnits() const override;

    /** \brief Default implementation returns GetClassID as display name. */
    std::string GetModelDisplayName() const override;

    /** \brief Default implementation returns "Unknown" as model type. */
    std::string GetModelType() const override;

    /** \brief Default implementation returns an empty function string. */
    FunctionStringType GetFunctionString() const override;

    /** \brief Default implementation returns the class name of the concrete instance as ID. */
    ModellClassIDType GetClassID() const override;

    /** \brief Default implementation returns an empty string. */
    std::string GetXName() const override;

    /** \brief Default implementation returns an empty string. */
    std::string GetXAxisName() const override;

    /** \brief Default implementation returns an empty string. */
    std::string GetXAxisUnit() const override;

    /** \brief Default implementation returns an empty string. */
    std::string GetYAxisName() const override;

    /** \brief Default implementation returns an empty string. */
    std::string GetYAxisUnit() const override;

    /**
     * \brief Returns the names of static parameters used when computing the signal.
     *
     * Static parameters are not part of the fitting scope but configure the model.
     * \return A vector of static parameter names.
     */
    virtual ParameterNamesType GetStaticParameterNames() const = 0;

    /**
     * \brief Returns the number of static parameters.
     * \return The count of static parameters.
     */
    virtual ParametersSizeType GetNumberOfStaticParameters() const = 0;

    /** \brief Default implementation returns no unit string ("") for every defined static parameter. */
    virtual ParamterUnitMapType GetStaticParameterUnits() const;


    /**
     * \brief Returns the names of derived parameters that can be computed by the model.
     * \remark Default implementation has no derived parameters.
     */
    DerivedParameterNamesType GetDerivedParameterNames() const override;

    /**
     * \brief Returns the number of derived parameters that can be computed by the model.
     * \remark Default implementation has no derived parameters.
     */
    DerivedParametersSizeType GetNumberOfDerivedParameters() const override;

    /**
     * \brief Sets the static parameters of the model before it is used.
     *
     * Checks the validity of the passed map and uses SetStaticParameter to set the values.
     *
     * \param parameters The map with the static parameters and their values.
     * \param allParameters If true, an exception will be thrown if the keys of passed parameters do
     * not equal the return of GetStaticParameterNames. Thus if true, one must set all static
     * parameters of the model.
     * \pre Parameters must only contain keys that exist in GetStaticParameterNames().
     * \pre If allParameters == true, parameters must define all keys of GetStaticParameterNames().
     */
    void SetStaticParameters(const StaticParameterMapType& parameters, bool allParameters = true);

    /**
     * \brief Retrieves the static parameters of the model (e.g. for serialization).
     *
     * Calls GetStaticParameter for every name defined in GetStaticParameterNames().
     * \return A map of static parameter names to their values.
     */
    StaticParameterMapType GetStaticParameters() const;

    /**
     * \brief Computes all derived parameters for the given model parameters.
     *
     * To change the derived parameter computation, ComputeDerivedParameters must be (re)implemented.
     *
     * \param parameters The parameters of the model for which the derived parameters should be computed.
     * \return A map of derived parameter names to their computed values.
     * \pre parameters must have the correct size.
     * \remark Default implementation has no derived parameters.
     */
    DerivedParameterMapType GetDerivedParameters(const ParametersType& parameters) const;

    /**
     * \brief Sets the time grid of the model.
     *
     * The time grid indicates the time points correlated with the signal the model should produce.
     * \remark The resolution of the time grid is in seconds (not in ms like mitk::TimeGeometry).
     */
    virtual void SetTimeGrid(const TimeGridType& grid);

    /**
     * \brief Gets the time grid of the model.
     * \remark The resolution of the time grid is in seconds (not in ms like mitk::TimeGeometry).
     */
    itkGetConstReferenceMacro(TimeGrid, TimeGridType);

    /**
     * \brief Computes and returns the model signal for the given parameters.
     * \param parameters The model parameters.
     * \return The computed model signal.
     */
    ModelResultType GetSignal(const ParametersType& parameters) const;

  protected:

    /**
     * \brief Computes the model function for the given parameters.
     *
     * Must be implemented by derived classes.
     * \param parameters The model parameters.
     * \return The computed model result.
     */
    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const = 0;

    /**
     * \brief Validates whether the model is in a valid state to compute the signal.
     *
     * Called by GetSignal() before ComputeModelfunction(). The default implementation
     * checks nothing and always returns true. Reimplement to realize special behavior
     * for derived classes.
     *
     * \param[out] error Set internally to indicate the error reason if method returns false.
     * \return True if the model is valid and can compute a signal, false otherwise.
     */
    virtual bool ValidateModel(std::string& error) const;

    /**
     * \brief Computes derived parameters from the given model parameters.
     *
     * Helper function called by GetDerivedParameters(). Implement in derived classes
     * to realize the concrete computation of derived parameters.
     * \remark Default implementation has no derived parameters.
     */
    virtual DerivedParameterMapType ComputeDerivedParameters(const ParametersType& parameters) const;

    /**
     * \brief Sets a single static parameter by name.
     *
     * Helper function called by SetStaticParameters(). Must be implemented in derived classes.
     */
    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) = 0;

    /**
     * \brief Retrieves a single static parameter value by name.
     *
     * Helper function called by GetStaticParameters(). Must be implemented in derived classes.
     */
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const = 0;

    ModelBase();
    ~ModelBase() override;

    ModelBase(const ModelBase& source);

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

    //timeGrid in seconds

    TimeGridType m_TimeGrid;

  private:

    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
