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

#ifndef EXTENSIONPOINTDEFINITIONCONSTANTS_H_
#define EXTENSIONPOINTDEFINITIONCONSTANTS_H_

#include <string>

/**
 * The ExtensionPointDefinitionConstants contains a list of unique ids in the
 * following form: <br>
 * "org.mitk.mybundle.mytype.propername" <br>
 *
 * This ids have the purpose of connecting the plugin.xml of each bundle to the
 * appropriate classes.
 *
 * Additionally it includes names of tags, which are used in XML files <br>
 * regarding the BlueBerry example.
 */
struct ExtensionPointDefinitionConstants
{
  static const std::string CHANGETEXT_XP_NAME;
  static const std::string CHANGETEXT_CHILD_DESCRIPTION;
  static const std::string CHANGETEXT_XMLATTRIBUTE_CLASS;
  static const std::string CHANGETEXT_XMLATTRIBUTE_NAME;
  static const std::string CHANGETEXT_XMLATTRIBUTE_ID;
};

#endif /*EXTENSIONPOINTDEFINITIONCONSTANTS_H_*/
