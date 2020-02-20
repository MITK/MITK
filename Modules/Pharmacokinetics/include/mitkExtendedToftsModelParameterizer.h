/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKEXTENDEDTOFTSMODELPARAMETRIZER_H
#define MITKEXTENDEDTOFTSMODELPARAMETRIZER_H

#include "mitkAIFBasedModelParameterizerBase.h"
#include "mitkExtendedToftsModel.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT ExtendedToftsModelParameterizer : public
    mitk::AIFBasedModelParameterizerBase<mitk::ExtendedToftsModel>
  {
  public:
    typedef ExtendedToftsModelParameterizer Self;
    typedef mitk::AIFBasedModelParameterizerBase<mitk::ExtendedToftsModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ExtendedToftsModelParameterizer,
                 mitk::AIFBasedModelParameterizerBase<mitk::ExtendedToftsModel>);
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
    ExtendedToftsModelParameterizer();

    ~ExtendedToftsModelParameterizer() override;

  private:

    //No copy constructor allowed
    ExtendedToftsModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}
#endif
