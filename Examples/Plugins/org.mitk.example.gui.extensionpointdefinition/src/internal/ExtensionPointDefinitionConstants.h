/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef EXTENSIONPOINTDEFINITIONCONSTANTS_H_
#define EXTENSIONPOINTDEFINITIONCONSTANTS_H_

#include <QString>

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
  static const QString CHANGETEXT_XP_NAME;
  static const QString CHANGETEXT_CHILD_DESCRIPTION;
  static const QString CHANGETEXT_XMLATTRIBUTE_CLASS;
  static const QString CHANGETEXT_XMLATTRIBUTE_NAME;
  static const QString CHANGETEXT_XMLATTRIBUTE_ID;
};

#endif /*EXTENSIONPOINTDEFINITIONCONSTANTS_H_*/
