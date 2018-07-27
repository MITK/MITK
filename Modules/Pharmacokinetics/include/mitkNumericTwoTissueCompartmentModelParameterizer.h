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

#ifndef MITKNUMERICTWOTISSUECOMPARTMENTMODELPARAMETERIZER_H
#define MITKNUMERICTWOTISSUECOMPARTMENTMODELPARAMETERIZER_H

#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkNumericTwoTissueCompartmentModel.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT NumericTwoTissueCompartmentModelParameterizer : public
    mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoTissueCompartmentModel>
  {
  public:
    typedef NumericTwoTissueCompartmentModelParameterizer Self;
    typedef mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoTissueCompartmentModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(NumericTwoTissueCompartmentModelParameterizer,
                 mitk::AIFBasedModelParameterizerBase<mitk::NumericTwoTissueCompartmentModel>);
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
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:
    NumericTwoTissueCompartmentModelParameterizer();

    virtual ~NumericTwoTissueCompartmentModelParameterizer();

  private:

    //No copy constructor allowed
    NumericTwoTissueCompartmentModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}
#endif
