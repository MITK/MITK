/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __AIFBASED_MODEL_PARAMETERIZER_BASE_H
#define __AIFBASED_MODEL_PARAMETERIZER_BASE_H

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkAIFParametrizerHelper.h"
#include "mitkAIFBasedModelBase.h"

namespace mitk
{
  /** Base class for model parameterizers for Models using an Aterial Input Function
  */
  template <class TAIFBasedModel>
  class MITKPHARMACOKINETICS_EXPORT AIFBasedModelParameterizerBase : public ConcreteModelParameterizerBase
    <TAIFBasedModel>
  {
  public:
    typedef AIFBasedModelParameterizerBase<TAIFBasedModel> Self;
    typedef ConcreteModelParameterizerBase<TAIFBasedModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(AIFBasedModelParameterizerBase, ConcreteModelParameterizerBase);

    typedef typename Superclass::ModelBaseType ModelBaseType;
    typedef typename Superclass::ModelBasePointer ModelBasePointer;

    typedef typename Superclass::ModelType ModelType;
    typedef typename Superclass::ModelPointer ModelPointer;

    typedef typename Superclass::StaticParameterValueType StaticParameterValueType;
    typedef typename Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef typename Superclass::StaticParameterMapType StaticParameterMapType;


    typedef typename Superclass::IndexType IndexType;

    itkSetMacro(AIF, mitk::AIFBasedModelBase::AterialInputFunctionType);
    itkGetConstReferenceMacro(AIF, mitk::AIFBasedModelBase::AterialInputFunctionType);

    itkSetMacro(AIFTimeGrid, mitk::ModelBase::TimeGridType);
    itkGetConstReferenceMacro(AIFTimeGrid, mitk::ModelBase::TimeGridType);


    /** Returns the global static parameters for the model.
    * @remark this default implementation assumes only AIF and its timegrid as static parameters.
    * Reimplement in derived classes to change this behavior.*/
    StaticParameterMapType GetGlobalStaticParameters() const override
    {
      StaticParameterMapType result;
      StaticParameterValuesType valuesAIF = mitk::convertArrayToParameter(this->m_AIF);
      StaticParameterValuesType valuesAIFGrid = mitk::convertArrayToParameter(this->m_AIFTimeGrid);

      result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIF, valuesAIF));
      result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIFTimeGrid, valuesAIFGrid));

      return result;
    };


  protected:

    AIFBasedModelParameterizerBase()
    {};

    ~AIFBasedModelParameterizerBase() override
    {};


    mitk::AIFBasedModelBase::AterialInputFunctionType m_AIF;
    mitk::ModelBase::TimeGridType m_AIFTimeGrid;


  private:

    //No copy constructor allowed
    AIFBasedModelParameterizerBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };

}
#endif // __AIFBASED_MODEL_PARAMETERIZER_BASE_H
