/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYREGISTRYCONSTANTS_H
#define BERRYREGISTRYCONSTANTS_H

#include <QString>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

struct org_blueberry_core_runtime_EXPORT RegistryConstants
{

  /**
   * The unique identifier constant (value "<code>org.blueberry.core.runtime</code>")
   * of the Core Runtime (pseudo-) plug-in.
   */
  static const QString RUNTIME_NAME; // = "org.blueberry.core.runtime";

  // System options
  static const QString PROP_NO_REGISTRY_FLUSHING; // = "blueberry.noRegistryFlushing";
  static const QString PROP_NO_LAZY_CACHE_LOADING; // = "blueberry.noLazyRegistryCacheLoading";
  static const QString PROP_NO_REGISTRY_CACHE; // = "blueberry.noRegistryCache";
  static const QString PROP_DEFAULT_REGISTRY; // = "blueberry.createRegistry";
  static const QString PROP_REGISTRY_NULL_USER_TOKEN; // = "blueberry.registry.nulltoken";
  static const QString PROP_MULTI_LANGUAGE; // = "blueberry.registry.MultiLanguage";

  /**
   * Specific error code supplied to the Status objects
   */
  static const int PLUGIN_ERROR; // = 1;
};

}

#endif // BERRYREGISTRYCONSTANTS_H
