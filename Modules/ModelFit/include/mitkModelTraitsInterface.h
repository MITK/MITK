/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelTraitsInterface_h
#define mitkModelTraitsInterface_h

#include "MitkModelFitExports.h"

#include <itkArray.h>
#include <itkArray2D.h>
#include <itkObject.h>

namespace mitk
{
  class MITKMODELFIT_EXPORT ModelTraitsInterface
  {
  public:


    typedef itk::Array<double> ModelResultType;
    using ParameterValueType = double;
    typedef itk::Array<ParameterValueType> ParametersType;

    typedef std::string ParameterNameType;
    typedef std::vector<ParameterNameType> ParameterNamesType;
    typedef ParametersType::SizeValueType ParametersSizeType;
    typedef ParameterNamesType DerivedParameterNamesType;
    typedef ParametersSizeType DerivedParametersSizeType;

    typedef std::map<ParameterNameType, double> ParamterScaleMapType;
    typedef std::map<ParameterNameType, std::string> ParamterUnitMapType;
    typedef std::map<ParameterNameType, double> DerivedParamterScaleMapType;
    typedef std::map<ParameterNameType, std::string> DerivedParamterUnitMapType;

    typedef std::string FunctionStringType;
    typedef std::string ModellClassIDType;

    /** Returns the names of parameters that must be defined when using
     * the model to compute the signal (GetSignal()).*/
    virtual ParameterNamesType GetParameterNames() const = 0;
    /** Returns the number of parameters that must be defined when using
     * the model to compute the signal (GetSignal()).*/
    virtual ParametersSizeType GetNumberOfParameters() const = 0;

    virtual ParamterScaleMapType GetParameterScales() const = 0;
    virtual ParamterUnitMapType GetParameterUnits() const = 0;

    /** Returns the names of derived parameters that must be defined when using
     * the model to compute them (GetDerivedParameters()).*/
    virtual DerivedParameterNamesType GetDerivedParameterNames() const = 0;
    /** Returns the number of derived parameters that must be defined when using
     * the model to compute them (GetDerivedParameters()).*/
    virtual DerivedParametersSizeType GetNumberOfDerivedParameters() const = 0;

    virtual DerivedParamterScaleMapType GetDerivedParameterScales() const = 0;
    virtual DerivedParamterUnitMapType GetDerivedParameterUnits() const = 0;

    virtual std::string GetModelDisplayName() const = 0;

    virtual std::string GetModelType() const = 0;

    virtual FunctionStringType GetFunctionString() const = 0;

    virtual ModellClassIDType GetClassID() const = 0;

    virtual std::string GetXName() const = 0;

    virtual std::string GetXAxisName() const = 0;
    virtual std::string GetXAxisUnit() const = 0;

    virtual std::string GetYAxisName() const = 0;
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
