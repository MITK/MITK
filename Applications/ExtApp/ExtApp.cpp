/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <org.blueberry.osgi/src/application/berryStarter.h>
#include <Poco/Util/MapConfiguration.h>
#include <mitkCoreExtObjectFactory.h>

int main(int argc, char** argv)
{
  
  RegisterCoreExtObjectFactory();
  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  Poco::Path basePath(argv[0]);
  basePath.setFileName("");
  
  Poco::Path BlueBerryPath(basePath);
  BlueBerryPath.pushDirectory("BlueBerry");

  Poco::Path corePath(basePath);
  corePath.pushDirectory("CoreBundles");

  Poco::Path extPath(basePath);
  extPath.pushDirectory("ExtBundles");

  std::string pluginDirs = BlueBerryPath.toString() + ";" + corePath.toString() + ";" + extPath.toString();

  Poco::Util::MapConfiguration* extConfig(new Poco::Util::MapConfiguration());
  extConfig->setString(berry::Platform::ARG_PLUGIN_DIRS, pluginDirs);
  extConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.extapplication");
  return berry::Starter::Run(argc, argv, extConfig);
}
