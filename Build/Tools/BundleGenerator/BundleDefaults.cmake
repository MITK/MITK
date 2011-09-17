SET(BUNDLE_NAMESPACE "")
SET(DEFAULT_REQUIRED_BUNDLES "org.mitk.core.services")
SET(DEFAULT_REQUIRED_BUNDLES_FOR_GUI "org.mitk.gui.qt.common")
SET(DEFAULT_PLUGIN_VERSION "0.1")
SET(DEFAULT_PLUGIN_VENDOR "DKFZ, Medical and Biological Informatics")
SET(DEFAULT_VIEW_BASEID "org.mitk.views.")
SET(DEFAULT_VIEW_CLASS_BEGIN "Qmitk")
SET(DEFAULT_VIEW_BASE_CLASS "QmitkFunctionality")
SET(DEFAULT_VIEW_BASE_CLASS_H "QmitkFunctionality.h")
SET(PROJECT_STATIC_VAR "MITK_STATIC")
SET(DOXYGEN_INGROUP "MITKPlugins")
SET(DEFAULT_CREATE_PLUGIN_MACRO "MACRO_CREATE_MITK_PLUGIN()")
SET(DEFAULT_CREATE_GUI_PLUGIN_MACRO "MACRO_CREATE_MITK_PLUGIN(QmitkExt)")
SET(PLUGIN_TEMPLATE "${PROJECT_SOURCE_DIR}/../../../CMake/QBundleTemplate" CACHE PATH "Path to the plugin templates" FORCE)
SET(PLUGIN_COPYRIGHT "/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/")
