/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitFunctorPolicy_h
#define mitkModelFitFunctorPolicy_h

#include "itkIndex.h"
#include "mitkModelFitFunctorBase.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ModelFitFunctorPolicy
  {
  public:
    typedef ModelFitFunctorBase::InputPixelArrayType InputPixelArrayType;
    typedef ModelFitFunctorBase::InputPixelArrayType OutputPixelArrayType;

    typedef mitk::ModelParameterizerBase ParameterizerType;
    typedef ParameterizerType::ConstPointer   ParameterizerConstPointer;

    typedef ModelFitFunctorBase               FunctorType;
    typedef ModelFitFunctorBase::ConstPointer FunctorConstPointer;

    typedef itk::Index<3> IndexType;

    ModelFitFunctorPolicy()
    {};

    ~ModelFitFunctorPolicy() {};

    unsigned int GetNumberOfOutputs() const
    {
      unsigned int result = 0;

      if (m_Functor.IsNotNull() && m_ModelParameterizer.IsNotNull())
      {
        ParameterizerType::ModelBasePointer tempModel = m_ModelParameterizer->GenerateParameterizedModel();
        result =  m_Functor->GetNumberOfOutputs(tempModel);
      }

      return result;
    }

    void SetModelFitFunctor(const mitk::ModelFitFunctorBase* functor)
    {
      if (!functor)
      {
        itkGenericExceptionMacro( << "Error. Functor is Null.");
      }

      m_Functor = functor;
    }

    void SetModelParameterizer(const ParameterizerType* parameterizer)
    {
      if (!parameterizer)
      {
        itkGenericExceptionMacro( << "Error. Parameterizer is Null.");
      }

      m_ModelParameterizer = parameterizer;
    }

    bool operator!=(const ModelFitFunctorPolicy& other) const
    {
      return !(*this == other);
    }

    bool operator==(const ModelFitFunctorPolicy& other) const
    {
      return (this->m_Functor == other.m_Functor) &&
             (this->m_ModelParameterizer == other.m_ModelParameterizer);
    }

    inline OutputPixelArrayType operator()(const InputPixelArrayType& value,
                                           const IndexType& currentIndex) const
    {
      if (!m_Functor)
      {
        itkGenericExceptionMacro( << "Error. Cannot process operator(). Functor is Null.");
      }

      if (!m_ModelParameterizer)
      {
        itkGenericExceptionMacro( << "Error. Cannot process operator(). Parameterizer is Null.");
      }

      ParameterizerType::ModelBasePointer parameterizedModel =
        m_ModelParameterizer->GenerateParameterizedModel(currentIndex);
      ParameterizerType::ParametersType initialParams = m_ModelParameterizer->GetInitialParameterization(
            currentIndex);
      OutputPixelArrayType result = m_Functor->Compute(value, parameterizedModel, initialParams);

      return result;
    }

  private:

    FunctorConstPointer m_Functor;
    ParameterizerConstPointer m_ModelParameterizer;
  };

}


#endif
