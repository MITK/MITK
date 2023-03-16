/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkThreeStepLinearModelParameterizer_h
#define mitkThreeStepLinearModelParameterizer_h

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkThreeStepLinearModel.h"

namespace mitk
{

class MITKPHARMACOKINETICS_EXPORT ThreeStepLinearModelParameterizer : public
  mitk::ConcreteModelParameterizerBase<mitk::ThreeStepLinearModel>
{
public:
  typedef ThreeStepLinearModelParameterizer Self;
  typedef mitk::ConcreteModelParameterizerBase<mitk::ThreeStepLinearModel> Superclass;
  typedef itk::SmartPointer< Self >                            Pointer;
  typedef itk::SmartPointer< const Self >                      ConstPointer;

  itkTypeMacro(ThreeStepLinearModelParameterizer, ConcreteModelParameterizerBase);
  itkFactorylessNewMacro(Self);

  typedef Superclass::ModelBaseType ModelBaseType;
  typedef Superclass::ModelBasePointer ModelBasePointer;

  typedef Superclass::ModelType ModelType;
  typedef Superclass::ModelPointer ModelPointer;

  typedef Superclass::StaticParameterValueType StaticParameterValueType;
  typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
  typedef Superclass::StaticParameterMapType StaticParameterMapType;

  typedef Superclass::IndexType IndexType;

  /** This function returns the default parameterization (e.g. initial parametrization for fitting)
   defined by the model developer for  for the given model.*/
  ParametersType GetDefaultInitialParameterization() const override;

protected:
  ThreeStepLinearModelParameterizer(){};

  ~ThreeStepLinearModelParameterizer() override{};

private:

  //No copy constructor allowed
  ThreeStepLinearModelParameterizer(const Self& source);
  void operator=(const Self&);  //purposely not implemented
};}

#endif
