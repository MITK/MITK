/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    StaticParameterMapType GetGlobalStaticParameters() const override
    {
      StaticParameterMapType result;
      return result;
    };

    /* Returns the local static parameters for the model at the given index.
     * @remark this default implementation assumes no local static parameters exist.
     * Thus an empty map is returned.*/
    StaticParameterMapType GetLocalStaticParameters(const IndexType& /*currentPosition*/) const override
    {
      StaticParameterMapType result;
      return result;
    };

    /* Returns an newly generated instance of the concrete model.
     * It is parameterized by the static parameters (returns of GetGlobalParameter() and
     * GetLocalParameter()).
     */
    ModelBasePointer GenerateParameterizedModel(const IndexType& currentPosition) const override
    {
      ModelPointer newModel = ModelType::New();
      StaticParameterMapType params = this->GetGlobalStaticParameters();
      StaticParameterMapType locals = this->GetLocalStaticParameters(currentPosition);

      params.insert(locals.begin(), locals.end());
      newModel->SetTimeGrid(m_DefaultTimeGrid);
      newModel->SetStaticParameters(params);

      return newModel.GetPointer();
    };

    ModelBasePointer GenerateParameterizedModel() const override
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
    ParametersType GetDefaultInitialParameterization() const override
    {
      ModelPointer newModel = ModelType::New();
      ParametersType params;
      params.SetSize(newModel->GetNumberOfParameters());
      params.Fill(0.0);

      return params;
    };

    ParameterNamesType GetParameterNames() const  override
    {
      return GenerateParameterizedModel()->GetParameterNames();
    };

    ParametersSizeType GetNumberOfParameters() const override
    {
      return GenerateParameterizedModel()->GetNumberOfParameters();
    };

    ParamterScaleMapType GetParameterScales() const override
    {
      return GenerateParameterizedModel()->GetParameterScales();
    };

    ParamterUnitMapType GetParameterUnits() const override
    {
      return GenerateParameterizedModel()->GetParameterUnits();
    };

    ParameterNamesType GetDerivedParameterNames() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterNames();
    };

    ParametersSizeType GetNumberOfDerivedParameters() const override
    {
      return GenerateParameterizedModel()->GetNumberOfDerivedParameters();
    };

    DerivedParamterScaleMapType GetDerivedParameterScales() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterScales();
    };

    DerivedParamterUnitMapType GetDerivedParameterUnits() const override
    {
      return GenerateParameterizedModel()->GetDerivedParameterUnits();
    };

    std::string GetModelDisplayName() const override
    {
      return GenerateParameterizedModel()->GetModelDisplayName();
    };

    std::string GetModelType() const override
    {
      return GenerateParameterizedModel()->GetModelType();
    };

    FunctionStringType GetFunctionString() const override
    {
      return GenerateParameterizedModel()->GetFunctionString();
    };

    ModellClassIDType GetClassID() const override
    {
      return GenerateParameterizedModel()->GetClassID();
    };

    std::string GetXName() const override
    {
      return GenerateParameterizedModel()->GetXName();
    };

    std::string GetXAxisName() const override
    {
      return GenerateParameterizedModel()->GetXAxisName();
    };

    std::string GetXAxisUnit() const override
    {
      return GenerateParameterizedModel()->GetXAxisUnit();
    };

    std::string GetYAxisName() const override
    {
      return GenerateParameterizedModel()->GetYAxisName();
    };

    std::string GetYAxisUnit() const override
    {
      return GenerateParameterizedModel()->GetYAxisUnit();
    }

  protected:
    ConcreteModelParameterizerBase()
    {
    };

    ~ConcreteModelParameterizerBase() override
    {
    };

  private:

    //No copy constructor allowed
    ConcreteModelParameterizerBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __CONCRETE_MODEL_PARAMETERIZER_BASE_H
