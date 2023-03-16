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

#include "mitkModelBase.h"
#include "MitkPharmacokineticsExports.h"

namespace mitk
{

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

    static const std::string MODEL_DISPLAY_NAME;

    static const std::string NAME_PARAMETER_A;
    static const std::string NAME_PARAMETER_kep;
    static const std::string NAME_PARAMETER_kel;
    static const std::string NAME_PARAMETER_tlag;
    static const std::string NAME_STATIC_PARAMETER_Tau;
    static const std::string NAME_STATIC_PARAMETER_S0;

    static const std::string UNIT_PARAMETER_A;
    static const std::string UNIT_PARAMETER_kep;
    static const std::string UNIT_PARAMETER_kel;
    static const std::string UNIT_PARAMETER_tlag;
    static const std::string UNIT_STATIC_PARAMETER_Tau;
    static const std::string UNIT_STATIC_PARAMETER_S0;

    static const unsigned int POSITION_PARAMETER_A;
    static const unsigned int POSITION_PARAMETER_kep;
    static const unsigned int POSITION_PARAMETER_kel;

    //tlag in minutes
    static const unsigned int POSITION_PARAMETER_tlag;

    static const unsigned int NUMBER_OF_PARAMETERS;

    itkSetMacro(Tau, double);
    itkGetConstReferenceMacro(Tau, double);

    itkSetMacro(S0, double);
    itkGetConstReferenceMacro(S0, double);

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    std::string GetXAxisName() const override;

    std::string GetXAxisUnit() const override;

    std::string GetYAxisName() const override;

    std::string GetYAxisUnit() const override;

    ParameterNamesType GetParameterNames() const override;

    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;


    ParameterNamesType GetStaticParameterNames() const override;
    ParametersSizeType GetNumberOfStaticParameters() const override;
    ParamterUnitMapType GetStaticParameterUnits() const override;

  protected:
    DescriptivePharmacokineticBrixModel();
    ~DescriptivePharmacokineticBrixModel() override;

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;


    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

  private:
    /**injection time Tau in minutes [min]*/
    double m_Tau;

    /**Value of the first time step, thus base value to scale the signal.
     * Default is 1.*/
    double m_S0;

    //No copy constructor allowed
    DescriptivePharmacokineticBrixModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented


  };
}


#endif
