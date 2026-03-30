/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptivePharmacokineticBrixModel_h
#define mitkDescriptivePharmacokineticBrixModel_h

#include <iostream>

#include <mitkModelBase.h>
#include <MitkPharmacokineticsExports.h>

namespace mitk
{

  /** \class DescriptivePharmacokineticBrixModel
   * \brief Descriptive pharmacokinetic model for MR perfusion according to Brix et al.
   *
   * This model describes the MR signal enhancement after contrast agent injection using
   * a phenomenological approach. The model function is defined as:
   * \code
   *   S(t) = S0 * {1 + (A/tau) * [ (kep*exp(-kel*(t-BAT)) / (kel*(kep-kel))) * (exp(kel*tx) - 1)
   *                                - (exp(-kep*(t-BAT)) / (kep-kel)) * (exp(kep*tx) - 1) ]}
   * \endcode
   * where tx = min(t - BAT, tau) for t > BAT.
   *
   * Model parameters (fitted): A (amplitude), kep (exchange rate constant), kel (elimination
   * rate constant), BAT (bolus arrival time).
   * Static parameters: tau (injection time), S0 (pre-contrast baseline signal).
   *
   * Model type: "Perfusion.MR"
   *
   * \sa DescriptivePharmacokineticBrixModelFactory, DescriptivePharmacokineticBrixModelParameterizer,
   *     DescriptivePharmacokineticBrixModelValueBasedParameterizer
   */
  class MITKPHARMACOKINETICS_EXPORT DescriptivePharmacokineticBrixModel : public ModelBase
  {

  public:
    typedef DescriptivePharmacokineticBrixModel Self;
    typedef ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(DescriptivePharmacokineticBrixModel, ModelBase);

    /** \brief Display name: "Descriptive Pharmacokinetic Brix Model". */
    static const std::string MODEL_DISPLAY_NAME;

    /** \brief Name of parameter A (amplitude, unitless). */
    static const std::string NAME_PARAMETER_A;
    /** \brief Name of parameter kep (exchange rate constant, 1/min). */
    static const std::string NAME_PARAMETER_kep;
    /** \brief Name of parameter kel (elimination rate constant, 1/min). */
    static const std::string NAME_PARAMETER_kel;
    /** \brief Name of parameter BAT (bolus arrival time, min). */
    static const std::string NAME_PARAMETER_BAT;

    /** \brief Name of static parameter tau (injection time, min). */
    static const std::string NAME_STATIC_PARAMETER_tau;
    /** \brief Name of static parameter S0 (pre-contrast signal, a.u.). */
    static const std::string NAME_STATIC_PARAMETER_s0;

    /** \brief Unit for parameter A ("a.u."). */
    static const std::string UNIT_PARAMETER_A;
    /** \brief Unit for parameter kep ("1/min"). */
    static const std::string UNIT_PARAMETER_kep;
    /** \brief Unit for parameter kel ("1/min"). */
    static const std::string UNIT_PARAMETER_kel;
    /** \brief Unit for parameter BAT ("min"). */
    static const std::string UNIT_PARAMETER_BAT;

    /** \brief Unit for static parameter tau ("min"). */
    static const std::string UNIT_STATIC_PARAMETER_tau;
    /** \brief Unit for static parameter S0 ("a.u."). */
    static const std::string UNIT_STATIC_PARAMETER_s0;

    /** \brief Index position of parameter A in the parameter array. */
    static const unsigned int POSITION_PARAMETER_A;
    /** \brief Index position of parameter kep in the parameter array. */
    static const unsigned int POSITION_PARAMETER_kep;
    /** \brief Index position of parameter kel in the parameter array. */
    static const unsigned int POSITION_PARAMETER_kel;
    /** \brief Index position of parameter BAT in the parameter array. */
    static const unsigned int POSITION_PARAMETER_BAT;

    /** \brief Total number of fitted parameters (4). */
    static const unsigned int NUMBER_OF_PARAMETERS;

    /** \brief Total number of static parameters (2: tau, S0). */
    static const unsigned int NUMBER_OF_STATIC_PARAMETERS;

    /** \brief Model type identifier ("Perfusion.MR"). */
    static const std::string MODEL_TYPE;

    /** \brief X axis display name ("Time"). */
    static const std::string X_AXIS_NAME;
    /** \brief X axis unit ("s"). */
    static const std::string X_AXIS_UNIT;
    /** \brief Y axis display name ("Signal"). */
    static const std::string Y_AXIS_NAME;
    /** \brief Y axis unit ("a.u."). */
    static const std::string Y_AXIS_UNIT;

    /** \brief Sets the injection time tau in minutes.
     *  \param[in] _arg The injection time value. */
    itkSetMacro(Tau, double);
    /** \brief Returns the injection time tau in minutes. */
    itkGetConstReferenceMacro(Tau, double);

    /** \brief Sets the pre-contrast baseline signal S0.
     *  \param[in] _arg The baseline signal value. Default is 1. */
    itkSetMacro(S0, double);
    /** \brief Returns the pre-contrast baseline signal S0. */
    itkGetConstReferenceMacro(S0, double);

    /** \brief Returns the model display name.
     *  \return "Descriptive Pharmacokinetic Brix Model". */
    std::string GetModelDisplayName() const override;

    /** \brief Returns the model type.
     *  \return "Perfusion.MR". */
    std::string GetModelType() const override;

    /** \brief Returns the X axis name.
     *  \return "Time". */
    std::string GetXAxisName() const override;

    /** \brief Returns the X axis unit.
     *  \return "s". */
    std::string GetXAxisUnit() const override;

    /** \brief Returns the Y axis name.
     *  \return "Signal". */
    std::string GetYAxisName() const override;

    /** \brief Returns the Y axis unit.
     *  \return "a.u.". */
    std::string GetYAxisUnit() const override;

    /** \brief Returns the names of all fitted parameters (A, kep, kel, BAT).
     *  \return Vector of parameter name strings. */
    ParameterNamesType GetParameterNames() const override;

    /** \brief Returns the number of fitted parameters (4).
     *  \return Number of parameters. */
    ParametersSizeType  GetNumberOfParameters() const override;

    /** \brief Returns the unit map for all fitted parameters.
     *  \return Map of parameter names to unit strings. */
    ParamterUnitMapType GetParameterUnits() const override;

    /** \brief Returns the names of all static parameters (tau, S0).
     *  \return Vector of static parameter name strings. */
    ParameterNamesType GetStaticParameterNames() const override;
    /** \brief Returns the number of static parameters (2).
     *  \return Number of static parameters. */
    ParametersSizeType GetNumberOfStaticParameters() const override;
    /** \brief Returns the unit map for all static parameters.
     *  \return Map of static parameter names to unit strings. */
    ParamterUnitMapType GetStaticParameterUnits() const override;

  protected:
    DescriptivePharmacokineticBrixModel();
    DescriptivePharmacokineticBrixModel(const DescriptivePharmacokineticBrixModel &other);
    ~DescriptivePharmacokineticBrixModel() override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
      const StaticParameterValuesType& values) override;

    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

    mitkCloneMacro(Self);

  private:
    /**injection time Tau in minutes [min]*/
    double m_Tau;

    /**Value of the first time step, thus base value to scale the signal.
     * Default is 1.*/
    double m_S0;

    void operator=(const Self&);  //purposely not implemented


  };
}


#endif
