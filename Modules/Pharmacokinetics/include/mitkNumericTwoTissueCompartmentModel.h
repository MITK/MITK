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

#ifndef MITKNUMERICTWOTISSUECOMPARTMENTMODEL_H
#define MITKNUMERICTWOTISSUECOMPARTMENTMODEL_H

#include "mitkAIFBasedModelBase.h"
#include "MitkPharmacokineticsExports.h"


namespace mitk
{
  class MITKPHARMACOKINETICS_EXPORT NumericTwoTissueCompartmentModel : public AIFBasedModelBase
  {

  public:
    typedef NumericTwoTissueCompartmentModel Self;
    typedef AIFBasedModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Run-time type information (and related methods). */
    itkTypeMacro(NumericTwoTissueCompartmentModel, ModelBase)

    typedef std::vector<double> state_type;


    /** Model Specifications */
    static const std::string MODEL_DISPLAY_NAME;

    static const std::string NAME_PARAMETER_K1;
    static const std::string NAME_PARAMETER_k2;
    static const std::string NAME_PARAMETER_k3;
    static const std::string NAME_PARAMETER_k4;
    static const std::string NAME_PARAMETER_VB;

    static const std::string UNIT_PARAMETER_K1;
    static const std::string UNIT_PARAMETER_k2;
    static const std::string UNIT_PARAMETER_k3;
    static const std::string UNIT_PARAMETER_k4;
    static const std::string UNIT_PARAMETER_VB;

    static const unsigned int POSITION_PARAMETER_K1;
    static const unsigned int POSITION_PARAMETER_k2;
    static const unsigned int POSITION_PARAMETER_k3;
    static const unsigned int POSITION_PARAMETER_k4;
    static const unsigned int POSITION_PARAMETER_VB;

    static const unsigned int NUMBER_OF_PARAMETERS;

    virtual std::string GetModelDisplayName() const override;

    virtual std::string GetModelType() const override;

    virtual ParameterNamesType GetParameterNames() const override;
    virtual ParametersSizeType  GetNumberOfParameters() const override;

    virtual ParamterUnitMapType GetParameterUnits() const override;

  protected:
    NumericTwoTissueCompartmentModel();
    virtual ~NumericTwoTissueCompartmentModel();

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    virtual itk::LightObject::Pointer InternalClone() const;

    virtual ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    virtual void PrintSelf(std::ostream& os, ::itk::Indent indent) const;

  private:


    //No copy constructor allowed
    NumericTwoTissueCompartmentModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif // MITKNUMERICTWOTISSUECOMPARTMENTMODEL_H
