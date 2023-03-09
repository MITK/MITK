/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNumericTwoCompartmentExchangeModelParameterizer_h
#define mitkNumericTwoCompartmentExchangeModelParameterizer_h

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
    StaticParameterMapType GetGlobalStaticParameters() const override;


    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    double m_ODEINTStepSize;

    NumericTwoCompartmentExchangeModelParameterizer();

    ~NumericTwoCompartmentExchangeModelParameterizer() override;

  private:

    //No copy constructor allowed
    NumericTwoCompartmentExchangeModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}
#endif
