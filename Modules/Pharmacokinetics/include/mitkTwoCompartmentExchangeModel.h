/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoCompartmentExchangeModel_h
#define mitkTwoCompartmentExchangeModel_h

#include <mitkAIFBasedModelBase.h>
#include <MitkPharmacokineticsExports.h>


namespace mitk
{
  /** \class TwoCompartmentExchangeModel
   * \brief Implementation of the two-compartment exchange model (2CXM) for MR perfusion pharmacokinetics.
   *
   * Calculates the concentration-time curve as a convolution of the arterial input function CA(t)
   * and a tissue-specific residue function R(t) composed of plasma and interstitial contributions:
   * \code
   *   Ctotal(t) = vp * Cp(t) + fi * Ci(t) = [vp * Qp(t) + fi * Qi(t)] conv CA(t)
   * \endcode
   * where vp = Vp/VT and fi = Vi/VT are the plasma and interstitial volume fractions respectively.
   *
   * The residue functions involve eigenvalues l1, l2 computed from F/Vp, PS/Vp, vp, and fi.
   * Fitted parameters: PS (permeability-surface area product per plasma volume), F/Vp (flow per
   * plasma volume), vp (plasma volume fraction), fi (interstitial volume fraction).
   *
   * \sa TwoTissueCompartmentModel, ExtendedToftsModel, AIFBasedModelBase
   */

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

    static const std::string MODEL_DISPLAY_NAME;

    static const std::string MODEL_TYPE;

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    ParameterNamesType GetParameterNames() const override;
    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;


  protected:
    TwoCompartmentExchangeModel();
    ~TwoCompartmentExchangeModel() override;

    TwoCompartmentExchangeModel(const TwoCompartmentExchangeModel& source);

    mitkCloneMacro(TwoCompartmentExchangeModel);

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

  private:

    void operator=(const Self&);  //purposely not implemented




  };
}

#endif
