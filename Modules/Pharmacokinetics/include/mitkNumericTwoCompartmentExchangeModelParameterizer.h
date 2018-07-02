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

#ifndef MITKNUMERICTWOCOMPARTMENTEXCHANGEMODELPARAMETRIZER_H
#define MITKNUMERICTWOCOMPARTMENTEXCHANGEMODELPARAMETRIZER_H

#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkNumericTwoCompartmentExchangeModel.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT NumericTwoCompartmentExchangeModelParameterizer : public
    mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoCompartmentExchangeModel>
  {
  public:
    typedef NumericTwoCompartmentExchangeModelParameterizer Self;
    typedef mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoCompartmentExchangeModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(NumericTwoCompartmentExchangeModelParameterizer,
                 mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoCompartmentExchangeModel>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelBaseType ModelBaseType;
    typedef Superclass::ModelBasePointer ModelBasePointer;

    typedef Superclass::ModelType ModelType;
    typedef ModelType::Pointer ModelPointer;

    typedef Superclass::StaticParameterValueType StaticParameterValueType;
    typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef  Superclass::StaticParameterMapType StaticParameterMapType;

    typedef Superclass::IndexType IndexType;

    itkSetMacro(ODEINTStepSize, double);
    itkGetConstReferenceMacro(ODEINTStepSize, double);

    /** Returns the global static parameters for the model.
    * @remark this default implementation assumes only AIF and its timegrid as static parameters.
    * Reimplement in derived classes to change this behavior.*/
    virtual StaticParameterMapType GetGlobalStaticParameters() const;


    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:

    double m_ODEINTStepSize;

    NumericTwoCompartmentExchangeModelParameterizer();

    virtual ~NumericTwoCompartmentExchangeModelParameterizer();

  private:

    //No copy constructor allowed
    NumericTwoCompartmentExchangeModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}
#endif
