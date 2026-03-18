/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseApplication.h>

int main(int argc, char* argv[])
{
  mitk::BaseApplication app(argc, argv);

  app.setSingleMode(true);
  app.setApplicationName("MITK Workbench");
  app.setOrganizationName("DKFZ");
  app.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.extapplication.workbench");

  return app.run();
}
