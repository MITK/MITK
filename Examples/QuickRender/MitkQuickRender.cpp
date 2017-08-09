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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVariant>
#include <mitkBaseApplication.h>

int main(int argc, char **argv)
{
  mitk::BaseApplication myApp(argc, argv);
  myApp.setApplicationName("MITK QuickRender");
  myApp.setOrganizationName("DKFZ");

  QStringList preloadLibs;
  preloadLibs << "liborg_mitk_example_gui_qmlapplication";
  myApp.setPreloadLibraries(preloadLibs);
  myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "org.mitk.example.gui.qmlapplication");

  QByteArray on("1");

  qputenv("QML_BAD_GUI_RENDER_LOOP", on);

  return myApp.run();
}
