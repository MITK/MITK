/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MODEL_FACTORY_BASE_H
#define __MODEL_FACTORY_BASE_H

#include <itkObject.h>

#include "mitkModelBase.h"
#include "mitkModelTraitsInterface.h"
#include "mitkModelParameterizerBase.h"
#include "mitkModelFitInfo.h"
#include "mitkConstraintCheckerBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /**Base class for model factories.
   * Default implementation just passes the model properties through from an instance created with add model.
   * To use the the base class, derive and at least implement the abstract member functions.
   */
  class MITKMODELFIT_EXPORT ModelFactoryBase : public itk::Object, public ModelTraitsInterface
  {
  public:
    /*typedef ModelFactoryBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;
*/
    mitkClassMacroItkParent(ModelFactoryBase, itk::Object);

    typedef ModelBase ModelBaseType;
    typedef ModelBaseType::Pointer ModelBasePointer;

    typedef ModelTraitsInterface::ParameterNameType ParameterNameType;
    typedef ModelTraitsInterface::ParameterNamesType ParameterNamesType;
    typedef ModelTraitsInterface::ParametersSizeType ParametersSizeType;

    typedef ModelTraitsInterface::ParamterScaleMapType ParamterScaleMapType;
    typedef ModelTraitsInterface::ParamterUnitMapType ParamterUnitMapType;
    typedef ModelTraitsInterface::FunctionStringType FunctionStringType;
    typedef ModelTraitsInterface::ModellClassIDType ModellClassIDType;

    typedef ModelTraitsInterface::DerivedParameterNamesType DerivedParameterNamesType;
    typedef ModelTraitsInterface::DerivedParametersSizeType DerivedParametersSizeType;
    typedef ModelTraitsInterface::DerivedParamterScaleMapType DerivedParamterScaleMapType;
    typedef ModelTraitsInterface::DerivedParamterUnitMapType DerivedParamterUnitMapType;

    virtual ModelBasePointer CreateModel() const = 0;

    /** Created a model parameterizer set up according to the passed model fit info.
     @pre fit must point to a valid instance.*/
    ModelParameterizerBase::Pointer CreateParameterizer(const modelFit::ModelFitInfo* fit) const;

    /** Create the default constraints that should/can be used for fitting if nothing else
     * is specified by the user.
     * @return Pointer to the constraint checker for default constraints. May return a NULL pointer
     * to indicated that the Model has no constraints by default. */
    virtual ConstraintCheckerBase::Pointer CreateDefaultConstraints() const = 0;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const = 0;

   protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const = 0;

    ModelFactoryBase();
    ~ModelFactoryBase() override;

  private:

    //No copy constructor allowed
    ModelFactoryBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __MODEL_FACTORY_BASE_H
