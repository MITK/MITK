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

#ifndef __MITK_TEST_MODEL_H_
#define __MITK_TEST_MODEL_H_

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /**Simple (linear) test model that is used to check functionality of default implementations in factories and stuff.*/
  class MITKMODELFIT_EXPORT TestModel : public mitk::ModelBase
  {

  public:
    typedef TestModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(TestModel, ModelBase);

    virtual ParamterScaleMapType GetParameterScales() const override;

    virtual ParamterUnitMapType GetParameterUnits() const override;

    virtual std::string GetModelDisplayName() const override;

    virtual std::string GetModelType() const override;

    virtual FunctionStringType GetFunctionString() const override;

    virtual std::string GetXName() const override;

    virtual std::string GetXAxisName() const override;

    virtual std::string GetXAxisUnit() const override;

    virtual std::string GetYAxisName() const override;

    virtual std::string GetYAxisUnit() const override;

    virtual ParameterNamesType GetParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfParameters() const override;

    virtual ParameterNamesType GetStaticParameterNames() const override;

    virtual ParametersSizeType GetNumberOfStaticParameters() const override;

    virtual ParameterNamesType GetDerivedParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfDerivedParameters() const override;

  protected:
    TestModel() {};
    virtual ~TestModel() {};

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    virtual itk::LightObject::Pointer InternalClone() const;

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;
    virtual DerivedParameterMapType ComputeDerivedParameters(const mitk::ModelBase::ParametersType&
        parameters) const override;

    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values);
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;

  private:

    //No copy constructor allowed
    TestModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
