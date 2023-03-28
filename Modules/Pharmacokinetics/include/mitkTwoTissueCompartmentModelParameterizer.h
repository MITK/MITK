/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoTissueCompartmentModelParameterizer_h
#define mitkTwoTissueCompartmentModelParameterizer_h

#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkTwoTissueCompartmentModel.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT TwoTissueCompartmentModelParameterizer : public
    mitk::AIFBasedModelParameterizerBase<mitk::TwoTissueCompartmentModel>
  {
  public:
    typedef TwoTissueCompartmentModelParameterizer Self;
    typedef mitk::AIFBasedModelParameterizerBase<mitk::TwoTissueCompartmentModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(TwoTissueCompartmentModelParameterizer,
                 mitk::AIFBasedModelParameterizerBase<mitk::TwoTissueCompartmentModel>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelBaseType ModelBaseType;
    typedef Superclass::ModelBasePointer ModelBasePointer;

    typedef Superclass::ModelType ModelType;
    typedef ModelType::Pointer ModelPointer;

    typedef Superclass::StaticParameterValueType StaticParameterValueType;
    typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef  Superclass::StaticParameterMapType StaticParameterMapType;

    typedef Superclass::IndexType IndexType;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    TwoTissueCompartmentModelParameterizer();

    ~TwoTissueCompartmentModelParameterizer() override;

  private:

    //No copy constructor allowed
    TwoTissueCompartmentModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}
#endif
