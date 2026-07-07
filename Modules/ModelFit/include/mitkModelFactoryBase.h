/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFactoryBase_h
#define mitkModelFactoryBase_h

#include <itkObject.h>

#include <mitkModelBase.h>
#include <mitkModelTraitsInterface.h>
#include <mitkModelParameterizerBase.h>
#include <mitkModelFitInfo.h>
#include <mitkConstraintCheckerBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{
  /**
   * \class ModelFactoryBase
   * \brief Abstract base class for model factories in the model fitting framework.
   *
   * A model factory is responsible for creating model instances, parameterizers,
   * and default constraints. It also exposes the model traits through the
   * ModelTraitsInterface. Derive from this class and implement the abstract
   * member functions to create a factory for a specific model type.
   *
   * \sa ConcreteModelFactoryBase, ModelBase, ModelParameterizerBase
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

    /**
     * \brief Creates and returns a new instance of the model.
     * \return Smart pointer to the newly created model instance.
     */
    virtual ModelBasePointer CreateModel() const = 0;

    /**
     * \brief Creates a model parameterizer configured according to the given model fit info.
     * \param[in] fit Pointer to the model fit info describing the fit configuration.
     * \return Smart pointer to the configured parameterizer.
     * \pre fit must point to a valid instance.
     */
    ModelParameterizerBase::Pointer CreateParameterizer(const modelFit::ModelFitInfo* fit) const;

    /**
     * \brief Creates default constraints for the model fitting process.
     *
     * These constraints should/can be used for fitting if the user does not specify custom ones.
     *
     * \return Pointer to the constraint checker for default constraints. May return a NULL pointer
     * to indicate that the model has no constraints by default.
     */
    virtual ConstraintCheckerBase::Pointer CreateDefaultConstraints() const = 0;

    /**
     * \brief Returns the default initial parameterization defined by the model developer.
     * \return The default initial parameter values for fitting.
     */
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

#endif
