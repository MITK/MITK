/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelGenerator_h
#define mitkModelGenerator_h

#include <mitkModelBase.h>
#include <mitkModelFactoryBase.h>
#include <mitkModelParameterizerBase.h>

#include <mitkModelFitInfo.h>

#include <mitkIModelFitProvider.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class ModelGenerator
   * \brief Utility class for creating model factories and parameterizers from model class IDs.
   *
   * This class uses the micro services registry to look up IModelFitProvider instances
   * and create the corresponding model factories and parameterizers.
   *
   * \sa ModelFactoryBase, ModelParameterizerBase, IModelFitProvider
   */
  class MITKMODELFIT_EXPORT ModelGenerator
  {
  public:
    typedef ModelFactoryBase::ModellClassIDType ModelClassIDType;

    /**
     * \brief Returns a factory instance for the given model class ID.
     * \param[in] id The model class identifier.
     * \return Smart pointer to the factory, or nullptr if no matching factory is found.
     */
    static ModelFactoryBase::Pointer GetModelFactory(const ModelClassIDType& id);

    /**
     * \brief Generates a model parameterizer from a model fit info instance.
     * \param[in] fit The model fit info describing the fit configuration.
     * \return Smart pointer to the parameterizer, or nullptr if no matching factory is found.
     */
    static ModelParameterizerBase::Pointer GenerateModelParameterizer(const modelFit::ModelFitInfo&
        fit);

    /**
     * \brief Returns the IModelFitProvider service for the given model class ID.
     * \param[in] id The model class identifier.
     * \return Pointer to the provider service, or nullptr if not found.
     */
    static IModelFitProvider* GetProviderService(const ModelClassIDType& id);

  protected:
    ModelGenerator();
    virtual ~ModelGenerator();

  private:
    //No copy constructor allowed
    ModelGenerator(const ModelGenerator& source);
    void operator=(const ModelGenerator&);  //purposely not implemented
  };
}

#endif
