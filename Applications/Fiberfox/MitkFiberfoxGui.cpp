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

#include <QVariant>

int main(int argc, char **argv)
{
  mitk::BaseApplication app(argc, argv);

  app.setSingleMode(false);
  app.setApplicationName("MITK Fiberfox");
  app.setOrganizationName("DKFZ");

  // Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  QStringList preloadLibs;
  preloadLibs << "liborg_mitk_gui_qt_ext";
  app.setPreloadLibraries(preloadLibs);

  app.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.extapplication.fiberfox");

  // Run Fiberfox.
  return app.run();
}
