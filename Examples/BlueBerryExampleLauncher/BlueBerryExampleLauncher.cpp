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

#include <mitkBaseApplication.h>

#include "BlueBerryExampleLauncherDialog.h"

#include <QFileInfo>
#include <QVariant>

int main(int argc, char** argv)
{
  mitk::BaseApplication app(argc, argv);
  app.setApplicationName("BlueBerryExampleLauncher");
  app.setOrganizationName("DKFZ");
  app.initializeQt();

  BlueBerryExampleLauncherDialog demoDialog;
  QString selectedConfiguration = demoDialog.getDemoConfiguration();

  if (selectedConfiguration.isEmpty()) return EXIT_SUCCESS;

  app.setProvisioningFilePath(selectedConfiguration);

  // We create the application id relying on a convention:
  // org.mitk.example.<configuration-name>
  QString appId = "org.mitk.example.";
  QStringList appIdTokens = QFileInfo(selectedConfiguration).baseName().toLower().split('_', QString::SkipEmptyParts);
  appId += appIdTokens.size() > 1 ? appIdTokens.at(1) : appIdTokens.at(0);

  // Special cases
  if (appId == "org.mitk.example.exampleplugins")
  {
    appId = "org.mitk.qt.extapplication";
  }

  app.setProperty(mitk::BaseApplication::PROP_APPLICATION, appId);

  return app.run();
}
