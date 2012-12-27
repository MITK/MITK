set(BUNDLE_NAMESPACE "")
set(DEFAULT_REQUIRED_BUNDLES "org.mitk.core.services")
set(DEFAULT_REQUIRED_BUNDLES_FOR_GUI "org.mitk.gui.qt.common")
set(DEFAULT_PLUGIN_VERSION "0.1")
set(DEFAULT_PLUGIN_VENDOR "DKFZ, Medical and Biological Informatics")
set(DEFAULT_VIEW_BASEID "org.mitk.views.")
set(DEFAULT_VIEW_CLASS_BEGIN "Qmitk")
set(DEFAULT_VIEW_BASE_CLASS "QmitkFunctionality")
set(DEFAULT_VIEW_BASE_CLASS_H "QmitkFunctionality.h")
set(PROJECT_STATIC_VAR "MITK_STATIC")
set(DOXYGEN_INGROUP "MITKPlugins")
set(DEFAULT_CREATE_PLUGIN_MACRO "MACRO_CREATE_MITK_PLUGIN()")
set(DEFAULT_CREATE_GUI_PLUGIN_MACRO "MACRO_CREATE_MITK_PLUGIN(QmitkExt)")
set(PLUGIN_TEMPLATE "${PROJECT_SOURCE_DIR}/../../../CMake/QBundleTemplate" CACHE PATH "Path to the plugin templates" FORCE)
set(PLUGIN_COPYRIGHT "/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/")
