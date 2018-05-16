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
  static const QString PROP_NO_REGISTRY_FLUSHING; // = "BlueBerry.noRegistryFlushing";
  static const QString PROP_NO_LAZY_REGISTRY_CACHE_LOADING; // = "BlueBerry.noLazyRegistryCacheLoading";
  static const QString PROP_CHECK_CONFIG; // = "osgi.checkConfiguration";
  static const QString PROP_NO_REGISTRY_CACHE; // = "BlueBerry.noRegistryCache";
  static const QString PROP_DEFAULT_REGISTRY; // = "BlueBerry.createRegistry";
  static const QString PROP_REGISTRY_nullptr_USER_TOKEN; // = "BlueBerry.registry.nulltoken";
  static const QString PROP_REGISTRY_MULTI_LANGUAGE; // = "BlueBerry.registry.MultiLanguage";

  /**
   * Specific error code supplied to the Status objects
   */
  static const int PLUGIN_ERROR; // = 1;
};

}

#endif // BERRYREGISTRYCONSTANTS_H
