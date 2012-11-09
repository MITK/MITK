/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryStarter.h"

#include <QApplication>

int main(int argc, char** argv)
{
  // Create a QCoreApplication instance first
  QApplication app(argc, argv);
  app.setApplicationName("solstice_ui");
  app.setOrganizationName("DKFZ");

  return berry::Starter::Run(argc, argv, 0);
}
