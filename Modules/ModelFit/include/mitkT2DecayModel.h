/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkT2DecayModel_h
#define mitkT2DecayModel_h

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** @class T2DecayModel
  * @brief Simple model of the MR T2 signal decay. This corresponds to an exponential decay in the form of:
  * f(t) = M0 * exp(-t/T2) with T2 being the transverse / spin-spin relaxation time. The derived parameter R2
  * is calculated from T2 by inversion.
  */
  class MITKMODELFIT_EXPORT T2DecayModel : public mitk::ModelBase
  {

  public:
    typedef T2DecayModel Self;
    typedef mitk::ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::ParameterNameType  ParameterNameType;
    typedef Superclass::ParametersSizeType ParametersSizeType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(T2DecayModel, ModelBase);

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    FunctionStringType GetFunctionString() const override;

    std::string GetXName() const override;

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParameterNamesType GetStaticParameterNames() const override;

    ParametersSizeType GetNumberOfStaticParameters() const override;

    DerivedParametersSizeType GetNumberOfDerivedParameters() const override;

    DerivedParameterNamesType GetDerivedParameterNames() const override;

    mitk::ModelBase::DerivedParameterMapType ComputeDerivedParameters(
      const mitk::ModelBase::ParametersType &parameters) const;

  protected:
    T2DecayModel() {};
    ~T2DecayModel() override {};

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const override;

  private:

    //No copy constructor allowed
    T2DecayModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif
