/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKTWOTISSUECOMPARTMENTMODEL_H
#define MITKTWOTISSUECOMPARTMENTMODEL_H

#include "mitkAIFBasedModelBase.h"
#include "MitkPharmacokineticsExports.h"


namespace mitk
{
  class MITKPHARMACOKINETICS_EXPORT TwoTissueCompartmentModel : public AIFBasedModelBase
  {

  public:
    typedef TwoTissueCompartmentModel Self;
    typedef AIFBasedModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(TwoTissueCompartmentModel, ModelBase);

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

    std::string GetModelDisplayName() const override;

    std::string GetModelType() const override;

    ParameterNamesType GetParameterNames() const override;
    ParametersSizeType  GetNumberOfParameters() const override;

    ParamterUnitMapType GetParameterUnits() const override;

  protected:
    TwoTissueCompartmentModel();
    ~TwoTissueCompartmentModel() override;

    /**
     * Actual implementation of the clone method. This method should be reimplemeted
     * in subclasses to clone the extra required parameters.
     */
    itk::LightObject::Pointer InternalClone() const override;

    ModelResultType ComputeModelfunction(const ParametersType& parameters) const override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

  private:


    //No copy constructor allowed
    TwoTissueCompartmentModel(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };
}

#endif // MITKTWOTISSUECOMPARTMENTMODEL_H
