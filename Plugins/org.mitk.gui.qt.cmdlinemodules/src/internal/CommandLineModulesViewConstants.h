/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef CommandLineModulesViewConstants_h
#define CommandLineModulesViewConstants_h

#include <QString>
#include <string>

/**
 * \class CommandLineModulesViewConstants
 * \brief Structure to define a namespace for constants used privately within this view.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
struct CommandLineModulesViewConstants
{
  /**
   * \brief The name of the preferences node containing the temporary directory.
   */
  static const std::string TEMPORARY_DIRECTORY_NODE_NAME;

  /**
   * \brief The name of the preferences node containing the output directory.
   */
  static const std::string OUTPUT_DIRECTORY_NODE_NAME;

  /**
   * \brief The name of the preferences node containing the list of directories to scan.
   */
  static const std::string MODULE_DIRECTORIES_NODE_NAME;

  /**
   * \brief The name of the preferences node containing the additional files to add to the module list.
   */
  static const std::string MODULE_FILES_NODE_NAME;

  /**
   * \brief The name of the preferences node containing whether we are producing debug output.
   */
  static const std::string DEBUG_OUTPUT_NODE_NAME;

  /**
   * \brief The name of the preferences node containing whether we are displaying advanced widgets.
   */
  static const std::string SHOW_ADVANCED_WIDGETS_NAME;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the application directory.
   */
  static const std::string LOAD_FROM_APPLICATION_DIR;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the "application directory/cli-modules".
   */
  static const std::string LOAD_FROM_APPLICATION_DIR_CLI_MODULES;


  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the users home directory.
   */
  static const std::string LOAD_FROM_HOME_DIR;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the users "home directory/cli-modules".
   */
  static const std::string LOAD_FROM_HOME_DIR_CLI_MODULES;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the applications current working directory.
   */
  static const std::string LOAD_FROM_CURRENT_DIR;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the applications "current working directory/cli-modules".
   */
  static const std::string LOAD_FROM_CURRENT_DIR_CLI_MODULES;

  /**
   * \brief The name of the preferences node containing a boolean describing whether
   * we are loading modules from the directory specified in CTK_MODULE_LOAD_PATH.
   */
  static const std::string LOAD_FROM_AUTO_LOAD_DIR;

  /**
   * \brief The name of the preferences node containing the validation mode.
   */
  static const std::string XML_VALIDATION_MODE;

  /**
   * \brief The name of the preferences node containing the timeout in seconds for XML retrieval.
   */
  static const std::string XML_TIMEOUT_SECS;

  /**
   * \brief The name of the preferences node containing the maximum number of concurrent processes.
   */
  static const std::string MAX_CONCURRENT;

  /**
   * \brief The View ID = org.mitk.gui.qt.cmdlinemodules, and should match that in plugin.xml.
   */
  static const std::string VIEW_ID;

};

#endif // CommandLineModulesViewConstants_h
