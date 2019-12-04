/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __I_MODEL_FIT_PROVIDER_H
#define __I_MODEL_FIT_PROVIDER_H

#include <mitkServiceInterface.h>
#include <mitkModelBase.h>

#include "MitkModelFitExports.h"

namespace mitk
{
  class ModelFactoryBase;
  namespace modelFit
  {
    class ModelFitInfo;
  }
}

namespace itk
{
  template <class T>
  class SmartPointer;
}

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   *
   * \brief The common interface for all model providers for model fitting.
   *
   * Implementations of this interface must be registered as a service
   * to make themselves available via the service registry.
   *
   * It is recommended to derive new implementations from ModelFitProviderBase or
   * from AbstractFileIO (if both reader and writer is implemented),
   * which provide correct service registration semantics.
   *
   * \sa ModelFitProviderBase
   */
  struct MITKMODELFIT_EXPORT IModelFitProvider
  {
    virtual ~IModelFitProvider();

    /**
     * \brief returns a factory instance for the model represented by the provider.
     */
    virtual itk::SmartPointer<ModelFactoryBase> GenerateFactory() const = 0;

    /**
     * @param fitInfo Pointer to a fit info instance for the respective model.
     *
     * This methods returns the values of the model variable that was used by the fit.
     * Normally it is a time grid directly extracted from the input image time geometry.
     * But depending on the model and fit it could be generated out of other properties.
     * This method is a.o. used when the fit should be plotted correctly.
     */
    virtual ModelBase::TimeGridType GetVariableGrid(const modelFit::ModelFitInfo* fitInfo) const = 0;

    /**
    * @brief Service property name for a description.
    *
    * The property value must be of type \c std::string.
    *
    * @return The property name.
    */
    static std::string PROP_DESCRIPTION();

    /**
    * @brief Service property name for the model ID handled by the provider.
    *
    * The property value must be of type \c std::string.
    *
    * @return The property name.
    */
    static std::string PROP_MODEL_CLASS_ID();

    /**
    * @brief Service property name for the model type of the model handled by the provider.
    *
    * The property value must be of type \c std::string.
    *
    * @return The property name.
    */
    static std::string PROP_MODEL_TYPE();
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IModelFitProvider, "org.mitk.IModelFitProvider")

#endif /* __I_MODEL_FIT_PROVIDER_H */
