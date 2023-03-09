/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConcreteModelFactoryBase_h
#define mitkConcreteModelFactoryBase_h

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

    ModelBasePointer CreateModel() const  override
    {
      return CreateConcreteModel().GetPointer();
    };

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override
    {
      ConstraintCheckerBase::Pointer noConstraints;
      return noConstraints;
    };

    ParameterNamesType GetParameterNames() const  override
    {
      return m_Reference->GetParameterNames();
    };

    ParametersSizeType GetNumberOfParameters() const override
    {
      return m_Reference->GetNumberOfParameters();
    };

    ParamterScaleMapType GetParameterScales() const override
    {
      return m_Reference->GetParameterScales();
    };

    ParamterUnitMapType GetParameterUnits() const override
    {
      return m_Reference->GetParameterUnits();
    };

    ParameterNamesType GetDerivedParameterNames() const override
    {
      return m_Reference->GetDerivedParameterNames();
    };

    ParametersSizeType GetNumberOfDerivedParameters() const override
    {
      return m_Reference->GetNumberOfDerivedParameters();
    };

    DerivedParamterScaleMapType GetDerivedParameterScales() const override
    {
      return m_Reference->GetDerivedParameterScales();
    };

    DerivedParamterUnitMapType GetDerivedParameterUnits() const override
    {
      return m_Reference->GetDerivedParameterUnits();
    };

    std::string GetModelDisplayName() const override
    {
      return m_Reference->GetModelDisplayName();
    };

    std::string GetModelType() const override
    {
      return m_Reference->GetModelType();
    };

    FunctionStringType GetFunctionString() const override
    {
      return m_Reference->GetFunctionString();
    };

    ModellClassIDType GetClassID() const override
    {
      return m_Reference->GetClassID();
    };

    std::string GetXName() const override
    {
      return m_Reference->GetXName();
    };

    std::string GetXAxisName() const override
    {
      return m_Reference->GetXAxisName();
    };

    std::string GetXAxisUnit() const override
    {
      return m_Reference->GetXAxisUnit();
    };

    std::string GetYAxisName() const override
    {
      return m_Reference->GetYAxisName();
    };

    std::string GetYAxisUnit() const override
    {
      return m_Reference->GetYAxisUnit();
    }

  protected:
    ConcreteModelFactoryBase()
    {
      m_Reference = ModelType::New();
    };

    ~ConcreteModelFactoryBase() override
    {
    };

    ModelPointer m_Reference;

  private:

    //No copy constructor allowed
    ConcreteModelFactoryBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
