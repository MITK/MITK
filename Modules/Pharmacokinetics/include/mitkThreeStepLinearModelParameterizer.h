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

#ifndef MITKTHREESTEPLINEARMODELPARAMETERIZER_H
#define MITKTHREESTEPLINEARMODELPARAMETERIZER_H

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
  virtual ParametersType GetDefaultInitialParameterization() const;

protected:
  ThreeStepLinearModelParameterizer(){};

  virtual ~ThreeStepLinearModelParameterizer(){};

private:

  //No copy constructor allowed
  ThreeStepLinearModelParameterizer(const Self& source);
  void operator=(const Self&);  //purposely not implemented
};}

#endif // MITKTHREESTEPLINEARMODELPARAMETERIZER_H
