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

#ifndef __CONCRETE_MODEL_PARAMETERIZER_BASE_H
#define __CONCRETE_MODEL_PARAMETERIZER_BASE_H

#include "mitkModelParameterizerBase.h"

namespace mitk
{
  template <class TModel>
  class ConcreteModelParameterizerBase : public ModelParameterizerBase
  {
  public:
    typedef ConcreteModelParameterizerBase<TModel> Self;
    typedef ModelParameterizerBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ConcreteModelParameterizerBase, ModelParameterizerBase);
    itkFactorylessNewMacro(Self);

    typedef typename Superclass::ModelBaseType ModelBaseType;
    typedef typename Superclass::ModelBasePointer ModelBasePointer;

    typedef TModel ModelType;
    typedef typename ModelType::Pointer ModelPointer;

    typedef typename Superclass::StaticParameterValueType StaticParameterValueType;
    typedef typename Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef typename Superclass::StaticParameterMapType StaticParameterMapType;

    typedef Superclass::IndexType IndexType;

    /* Returns the global static parameters for the model.
     * @remark this default implementation assumes no global static parameters exist.
     * Thus an empty map is returned.*/
    virtual StaticParameterMapType GetGlobalStaticParameters() const
    {
      StaticParameterMapType result;
      return result;
    };

    /* Returns the local static parameters for the model at the given index.
     * @remark this default implementation assumes no local static parameters exist.
     * Thus an empty map is returned.*/
    virtual StaticParameterMapType GetLocalStaticParameters(const IndexType& /*currentPosition*/) const
    {
      StaticParameterMapType result;
      return result;
    };

    /* Returns an newly generated instance of the concrete model.
     * It is parameterized by the static parameters (returns of GetGlobalParameter() and
     * GetLocalParameter()).
     */
    virtual ModelBasePointer GenerateParameterizedModel(const IndexType& currentPosition) const override
    {
      ModelPointer newModel = ModelType::New();
      StaticParameterMapType params = this->GetGlobalStaticParameters();
      StaticParameterMapType locals = this->GetLocalStaticParameters(currentPosition);

      params.insert(locals.begin(), locals.end());
      newModel->SetTimeGrid(m_DefaultTimeGrid);
      newModel->SetStaticParameters(params);

      return newModel.GetPointer();
    };

    virtual ModelBasePointer GenerateParameterizedModel() const override
    {
      ModelPointer newModel = ModelType::New();
      StaticParameterMapType params = this->GetGlobalStaticParameters();

      newModel->SetTimeGrid(m_DefaultTimeGrid);
      newModel->SetStaticParameters(params, false);

      return newModel.GetPointer();
    };

    /* Returns a parameterization filled with zeros.
     * @remark this default implementation assumes no special initial parameterization is suggested.
     * Thus an zero filled vector is returned.*/
    virtual ParametersType GetDefaultInitialParameterization() const
    {
      ModelPointer newModel = ModelType::New();
      ParametersType params;
      params.SetSize(newModel->GetNumberOfParameters());
      params.Fill(0.0);

      return params;
    };

    virtual ParameterNamesType GetParameterNames() const  override
    {
      return GenerateParameterizedModel()->GetParameterNames();
    };

    virtual ParametersSizeType GetNumberOfParameters() const override
    {
      return GenerateParameterizedModel()->GetNumberOfParameters();
    };

    virtual ParamterScaleMapType GetParameterScales() const override
    {
      return GenerateParameterizedModel()->GetParameterScales();
    };

    virtual ParamterUnitMapType GetParameterUnits() const override
    {
      return GenerateParameterizedModel()->GetParameterUnits();
    };

    virtual ParameterNamesType GetDerivedParameterNames() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterNames();
    };

    virtual ParametersSizeType GetNumberOfDerivedParameters() const override
    {
      return GenerateParameterizedModel()->GetNumberOfDerivedParameters();
    };

    virtual DerivedParamterScaleMapType GetDerivedParameterScales() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterScales();
    };

    virtual DerivedParamterUnitMapType GetDerivedParameterUnits() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterUnits();
    };

    virtual std::string GetModelDisplayName() const override
    {
      return GenerateParameterizedModel()->GetModelDisplayName();
    };

    virtual std::string GetModelType() const override
    {
      return GenerateParameterizedModel()->GetModelType();
    };

    virtual FunctionStringType GetFunctionString() const override
    {
      return GenerateParameterizedModel()->GetFunctionString();
    };

    virtual ModellClassIDType GetClassID() const override
    {
      return GenerateParameterizedModel()->GetClassID();
    };

    virtual std::string GetXName() const override
    {
      return GenerateParameterizedModel()->GetXName();
    };

    virtual std::string GetXAxisName() const override
    {
      return GenerateParameterizedModel()->GetXAxisName();
    };

    virtual std::string GetXAxisUnit() const override
    {
      return GenerateParameterizedModel()->GetXAxisUnit();
    };

    virtual std::string GetYAxisName() const override
    {
      return GenerateParameterizedModel()->GetYAxisName();
    };

    virtual std::string GetYAxisUnit() const override
    {
      return GenerateParameterizedModel()->GetYAxisUnit();
    }

  protected:
    ConcreteModelParameterizerBase()
    {
    };

    virtual ~ConcreteModelParameterizerBase()
    {
    };

  private:

    //No copy constructor allowed
    ConcreteModelParameterizerBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __CONCRETE_MODEL_PARAMETERIZER_BASE_H
