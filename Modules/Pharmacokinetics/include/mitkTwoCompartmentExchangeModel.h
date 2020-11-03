/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKTWOCOMPARTMENTEXCHANGEMODEL_H
#define MITKTWOCOMPARTMENTEXCHANGEMODEL_H

#include "mitkAIFBasedModelBase.h"
#include "MitkPharmacokineticsExports.h"


namespace mitk
{
  /** @class TwoCompartmentExchangeModel
   * @brief Implementation of the analystical model function of the Physiological Pharmacokinetic Brix model, using an Aterial Input Function
   * The Model calculates the Concentration-Time-Curve as a convolution of the Aterial Input funciton CA(t) and a tissue specific
   * residue function R(t). The Residue funktion consists of two parts: The Residue funktion Qp(t) of the Blood Plasma p and the residue funktion Qi(t) of the
   * interstitial volume I.
   *       Ctotal(t) = vp * Cp(t) + fi * Ci(t) = [vp * Qp(t) + fi * Qi(t)] conv CA(t)
   *                 = Qtotal(t) conv CA(t)
   * where vp=Vp/VT and fi=Vi/VT are the portion of Plasma/interstitial volume Vp/VI of the total volume VT respectively.
   * The Residuefunctions are described by
   * Qp(t) = F/Vp * PS/Vp * 1/(l2 - l1) *[ µ2 exp(l1*t) - µ1 exp(l2*t)]* sig(t)
   * Qi(t) = F/Vp * PS/Vi * 1/(l1 - l2) *[ exp(l1*t) - exp(l2*t)]* sig(t)
   *       = F/Vp * PS/Vp * vp/fi * 1/(l1 - l2) *[ exp(l1*t) - exp(l2*t)]* sig(t)
   * with
   * l1/2 = -1/2 (PS/Vp * vp/fi + PS/Vp + F/Vp) +/- sqrt((PS/Vp * vp/fi + PS/Vp + F/Vp)² - 4* F/Vp * PS/Vp * vp/fi)
   * µ1/2 = F/Vp * Vp/PS + 1 + Vp/PS* l1/2
   *
   * The parameters PS/Vp, F/Vp,  vp and fi are subject to the fitting routine*/

  class MITKPHARMACOKINETICS_EXPORT TwoCompartmentExchangeModel : public AIFBasedModelBase
  {

  public:
    typedef TwoCompartmentExchangeModel Self;
    typedef AIFBasedModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(TwoCompartmentExchangeModel, ModelBase);

    /** Model Specifications */
    static const std::string MODEL_DISPLAY_NAME;

    static const std::string NAME_PARAMETER_F;
    static const std::string NAME_PARAMETER_PS;
    static const std::string NAME_PARAMETER_ve;
    static const std::string NAME_PARAMETER_vp;

    static const unsigned int POSITION_PARAMETER_F;
    static const unsigned int POSITION_PARAMETER_PS;
    static const unsigned int POSITION_PARAMETER_ve;
    static const unsigned int POSITION_PARAMETER_vp;

    static const std::string UNIT_PARAMETER_F;
    static const std::string UNIT_PARAMETER_PS;
    static const std::string UNIT_PARAMETER_ve;
    static const std::string UNIT_PARAMETER_vp;

    static const unsigned int NUMBER_OF_PARAMETERS;

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    ParameterNamesType GetParameterNames() const override;
    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;


  protected:
    TwoCompartmentExchangeModel();
    ~TwoCompartmentExchangeModel() override;

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

  private:


    //No copy constructor allowed
    TwoCompartmentExchangeModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented




  };
}

#endif // MITKTWOCOMPARTMENTEXCHANGEMODEL_H
