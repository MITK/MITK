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

#include "MitkModelFitExports.h"
#include "mitkModelTraitsInterface.h"

namespace mitk
{
  /**@class ModelBase
   * @brief Base class for (dynamic) models.
   * A model can be used to calculate its signal given the discrete time grid of the signal
   * and the parameters of the model.\n
   * A model has 3 types of parameters:\n
   * - parameters
   * - static parameters
   * - derived parameters
   * .
   * "Parameters" and "static parameters" are used to compute the signal of the model.
   * "Parameters" are the ones that will be changed for/by model fitting.
   * "Static parameters" are used to configure the model for fitting but are itself not
   * part of the fitting scope (compare itk::Transform parameters and static parameters).
   * "Derived parameters" are model specific parameters computed from "Parameters" e.g. (DerivedParam1 = Param1/Param2).
   * It may be implemented if e.g. for practical usage not the fitted parameters are needed but
   * derivation of them.
   * @remark: If you implement your own model calls regard const correctness and do not change
   * or undermine the constness of this base class. It is important because in case of fitting
   * models are used in a multi threaded environment and must be thread safe. Thus the getter and
   * computation functions are implemented as const and thread safe methods.*/
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
    /** Type defining the time grid used be models.
     * @remark the model time grid has a resolution in sec and not like the time geometry which uses ms.*/
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

    /**Default implementation returns a scale of 1.0 for every defined parameter.*/
    ParamterScaleMapType GetParameterScales() const override;

    /**Default implementation returns no unit string ("") for every defined parameter.*/
    ParamterUnitMapType GetParameterUnits() const override;

    /**Default implementation returns a scale of 1.0 for every defined derived parameter.*/
    DerivedParamterScaleMapType GetDerivedParameterScales() const override;

    /**Default implementation returns no unit string ("") for every defined derived parameter.*/
    DerivedParamterUnitMapType GetDerivedParameterUnits() const override;

    /**Default implementation returns GetClassID as display name.*/
    std::string GetModelDisplayName() const override;

    /**Default implementation returns "Unkown" as model type.*/
    std::string GetModelType() const override;

    /**Default implementation returns an empty functions string.*/
    FunctionStringType GetFunctionString() const override;

    /**Default implementation the class name of the concrete instance as ID.*/
    ModellClassIDType GetClassID() const override;

    /**Default implementation returns an empty string.*/
    std::string GetXName() const override;

    /**Default implementation returns an empty string.*/
    std::string GetXAxisName() const override;

    /**Default implementation returns an empty string.*/
    std::string GetXAxisUnit() const override;

    /**Default implementation returns an empty string.*/
    std::string GetYAxisName() const override;

    /**Default implementation returns an empty string.*/
    std::string GetYAxisUnit() const override;

    /** Returns the names of static parameters that will be used when using
     * the model to compute the signal (but are not defined via GetSignal()).*/
    virtual ParameterNamesType GetStaticParameterNames() const = 0;
    /** Returns the number of static parameters that will be used when using
     * the model to compute the signal (but are not defined via GetSignal()).*/
    virtual ParametersSizeType GetNumberOfStaticParameters() const = 0;

    /**Default implementation returns no unit string ("") for every defined parameter.*/
    virtual ParamterUnitMapType GetStaticParameterUnits() const;


    /** Returns the names of derived parameters that can/will be computed by the model
     * given specific model parameters.
     * @remark Default implementation has no derived parameters*/
    DerivedParameterNamesType GetDerivedParameterNames() const override;
    /** Returns the number of derived parameters that can/will be computed by the model
    * given specific model parameters.
    * @remark Default implementation has no derived parameters*/
    DerivedParametersSizeType GetNumberOfDerivedParameters() const override;

    /** Generic interface method that can be used to set the static parameters of the model
     * before it is used.
     * It checks the validity of the passed map and uses SetStaticParameter to set the values.
     * @param parameters The map with the static parameters and their values.
     * @param allParameters If true an exception will be thrown if the keys of passed parameters do
     * not equal the return of GetStaticParameterNames. Thus if true, one must set all static
     * parameters of the model.
     * @pre Parameters must only contain keys that exist in GetStaticParameterNames()
     * @pre If allParameters == true, parameters must define all keys of GetStaticParameterNames()*/
    void SetStaticParameters(const StaticParameterMapType& parameters, bool allParameters = true);

    /** Generic interface method that can be used to retrieve the static parameters of the model;
     * e.g. in order to serialize the model settings.
     * It calls GetStaticParameter for every name defined in GetStaticParameterNames().*/
    StaticParameterMapType GetStaticParameters() const;

    /** Generic interface method that computes all derived parameters implemented for the given models.
     * To changed the derived parameter computation. ComputeDerivedParameters must be (re)implemented.
     * @pre parameters must have the right size.
     * @param parameters The parameters of the model for which the derived parameters should be computed.
     * It calls GetStaticParameter for every name defined in GetStaticParameterNames().
     * @remark Default implementation has no derived parameters*/
    DerivedParameterMapType GetDerivedParameters(const ParametersType& parameters) const;

    /** Sets the time grid of the model. It indicates the time points correlated with the signal the modell should
     produce.
     @remark The resolution of the time grid is in seconds. (Not in ms like the mitk::TimeGeometry)*/
    virtual void SetTimeGrid(const TimeGridType& grid);
    /** Gets the time grid of the model. It indicates the time points correlated with the signal the modell should
     produce.
     @remark The resolution of the time grid is in seconds. (Not in ms like the mitk::TimeGeometry)*/
    itkGetConstReferenceMacro(TimeGrid, TimeGridType);

    ModelResultType GetSignal(const ParametersType& parameters) const;

  protected:

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const = 0;

    /** Member is called by GetSignal() before ComputeModelfunction(). It indicates if model is in a valid state and
     * ready to compute the signal. The default implementation checks nothing and always returns true.
     * Reimplement to realize special behavior for derived classes.
     * @param [out] error Set internally to indicate the error reason if method returns false. Is used by GetSignal() for the
     * exception comment.
     * @return Returns true if the model is valid and can compute a signal. Otherwise it returns false.*/
    virtual bool ValidateModel(std::string& error) const;

    /** Helper function called by GetDerivedParameters(). Implement in derived classes to realize
    * the concrete computation of derived parameters.
    * @remark Default implementation has no derived parameters*/
    virtual DerivedParameterMapType ComputeDerivedParameters(const ParametersType& parameters) const;

    /** Helper function called by SetStaticParameters(). Implement in derived classes to realize
    * the concrete setting of static parameters.*/
    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) = 0;
    /** Helper function called by GetStaticParameters(). Implement in derived classes to realize
    * the concrete retrieval of static parameters.*/
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const = 0;

    ModelBase();
    ~ModelBase() override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

    //timeGrid in seconds

    TimeGridType m_TimeGrid;

  private:

    //No copy constructor allowed
    ModelBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
