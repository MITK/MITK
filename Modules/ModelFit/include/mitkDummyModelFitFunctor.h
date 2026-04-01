/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDummyModelFitFunctor_h
#define mitkDummyModelFitFunctor_h

#include <itkObject.h>

#include <mitkModelBase.h>
#include <mitkModelFitFunctorBase.h>
#include <mitkMVConstrainedCostFunctionDecorator.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class DummyModelFitFunctor
   * \brief A no-op model fit functor that returns initial parameters without optimization.
   *
   * This functor does not perform any actual fitting. It simply returns the initial
   * parameters as the "fitted" result. It is useful for testing the model fitting
   * pipeline infrastructure or for generating model signals from known parameters.
   *
   * \sa ModelFitFunctorBase, LevenbergMarquardtModelFitFunctor
   */
  class MITKMODELFIT_EXPORT DummyModelFitFunctor : public ModelFitFunctorBase
  {
  public:
    typedef DummyModelFitFunctor Self;
    typedef ModelFitFunctorBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(DummyModelFitFunctor, ModelFitFunctorBase);

    typedef Superclass::InputPixelArrayType InputPixelArrayType;
    typedef Superclass::OutputPixelArrayType OutputPixelArrayType;

    itkSetMacro(DerivativeStepLength, double);
    itkGetMacro(DerivativeStepLength, double);

    ParameterNamesType GetCriterionNames() const override;

  protected:

    typedef Superclass::ParametersType ParametersType;
    typedef Superclass::SignalType SignalType;

    DummyModelFitFunctor();

    ~DummyModelFitFunctor() override;

    ParametersType DoModelFit(const SignalType& value, const ModelBase* model,
                                      const ModelBase::ParametersType& initialParameters,
                                      DebugParameterMapType& debugParameters) const override;

    OutputPixelArrayType GetCriteria(const ModelBase* model, const ParametersType& parameters,
        const SignalType& sample) const override;

    /** Generator function that instantiates and parameterizes the cost function that should be used by the fit functor*/
    virtual MVModelFitCostFunction::Pointer GenerateCostFunction(const SignalType& value,
        const ModelBase* model) const;

    ParameterNamesType DefineDebugParameterNames() const override;

  private:
    double m_DerivativeStepLength;

  };

}


#endif
