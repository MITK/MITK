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

#ifndef DESCRIPTIVEPHARMACOKINETICBRIXMODEL_H
#define DESCRIPTIVEPHARMACOKINETICBRIXMODEL_H

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

    virtual std::string GetModelDisplayName() const override;

    virtual std::string GetModelType() const override;

    virtual std::string GetXAxisName() const override;

    virtual std::string GetXAxisUnit() const override;

    virtual std::string GetYAxisName() const override;

    virtual std::string GetYAxisUnit() const override;

    virtual ParameterNamesType GetParameterNames() const override;

    virtual ParametersSizeType  GetNumberOfParameters() const override;

    virtual ParamterUnitMapType GetParameterUnits() const override;


    virtual ParameterNamesType GetStaticParameterNames() const override;
    virtual ParametersSizeType GetNumberOfStaticParameters() const override;
    virtual ParamterUnitMapType GetStaticParameterUnits() const override;

  protected:
    DescriptivePharmacokineticBrixModel();
    virtual ~DescriptivePharmacokineticBrixModel();

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    virtual itk::LightObject::Pointer InternalClone() const;

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    virtual void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;
    virtual StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;


    virtual void PrintSelf(std::ostream& os, ::itk::Indent indent) const;

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


#endif //DESCRIPTIVEPHARMACOKINETICBRIXMODEL_H
