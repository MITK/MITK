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

#ifndef __CONCRETE_MODEL_FACTORY_BASE_H
#define __CONCRETE_MODEL_FACTORY_BASE_H

#include "mitkModelFactoryBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  template <class TModel>
  class ConcreteModelFactoryBase : public ModelFactoryBase
  {
  public:
/*    typedef ConcreteModelFactoryBase Self;
    typedef ModelFactoryBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;
*/
    mitkClassMacro(ConcreteModelFactoryBase, ModelFactoryBase);

    typedef ModelFactoryBase::ModelBaseType ModelBaseType;
    typedef ModelFactoryBase::ModelBasePointer ModelBasePointer;

    typedef TModel ModelType;
    typedef typename ModelType::Pointer ModelPointer;

    typedef ModelFactoryBase::ParameterNameType ParameterNameType;
    typedef ModelFactoryBase::ParameterNamesType ParameterNamesType;
    typedef ModelFactoryBase::ParametersSizeType ParametersSizeType;
    typedef ModelFactoryBase::ParamterScaleMapType ParamterScaleMapType;
    typedef ModelFactoryBase::ParamterUnitMapType ParamterUnitMapType;
    typedef ModelFactoryBase::DerivedParameterNamesType DerivedParameterNamesType;
    typedef ModelFactoryBase::DerivedParametersSizeType DerivedParametersSizeType;
    typedef ModelFactoryBase::DerivedParamterScaleMapType DerivedParamterScaleMapType;
    typedef ModelFactoryBase::DerivedParamterUnitMapType DerivedParamterUnitMapType;
    typedef ModelFactoryBase::FunctionStringType FunctionStringType;
    typedef ModelFactoryBase::ModellClassIDType ModellClassIDType;

    static ModelPointer CreateConcreteModel()
    {
      return ModelType::New();
    }

    virtual ModelBasePointer CreateModel() const  override
    {
      return CreateConcreteModel().GetPointer();
    };

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override
    {
      ConstraintCheckerBase::Pointer noConstraints;
      return noConstraints;
    };

    virtual ParameterNamesType GetParameterNames() const  override
    {
      return m_Reference->GetParameterNames();
    };

    virtual ParametersSizeType GetNumberOfParameters() const override
    {
      return m_Reference->GetNumberOfParameters();
    };

    virtual ParamterScaleMapType GetParameterScales() const override
    {
      return m_Reference->GetParameterScales();
    };

    virtual ParamterUnitMapType GetParameterUnits() const override
    {
      return m_Reference->GetParameterUnits();
    };

    virtual ParameterNamesType GetDerivedParameterNames() const override
    {
      return m_Reference->GetDerivedParameterNames();
    };

    virtual ParametersSizeType GetNumberOfDerivedParameters() const override
    {
      return m_Reference->GetNumberOfDerivedParameters();
    };

    virtual DerivedParamterScaleMapType GetDerivedParameterScales() const override
    {
      return m_Reference->GetDerivedParameterScales();
    };

    virtual DerivedParamterUnitMapType GetDerivedParameterUnits() const override
    {
      return m_Reference->GetDerivedParameterUnits();
    };

    virtual std::string GetModelDisplayName() const override
    {
      return m_Reference->GetModelDisplayName();
    };

    virtual std::string GetModelType() const override
    {
      return m_Reference->GetModelType();
    };

    virtual FunctionStringType GetFunctionString() const override
    {
      return m_Reference->GetFunctionString();
    };

    virtual ModellClassIDType GetClassID() const override
    {
      return m_Reference->GetClassID();
    };

    virtual std::string GetXName() const override
    {
      return m_Reference->GetXName();
    };

    virtual std::string GetXAxisName() const override
    {
      return m_Reference->GetXAxisName();
    };

    virtual std::string GetXAxisUnit() const override
    {
      return m_Reference->GetXAxisUnit();
    };

    virtual std::string GetYAxisName() const override
    {
      return m_Reference->GetYAxisName();
    };

    virtual std::string GetYAxisUnit() const override
    {
      return m_Reference->GetYAxisUnit();
    }

  protected:
    ConcreteModelFactoryBase()
    {
      m_Reference = ModelType::New();
    };

    virtual ~ConcreteModelFactoryBase()
    {
    };

    ModelPointer m_Reference;

  private:

    //No copy constructor allowed
    ConcreteModelFactoryBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __CONCRETE_MODEL_FACTORY_BASE_H
