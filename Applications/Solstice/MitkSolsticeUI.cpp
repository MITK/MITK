/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
