/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <QApplication>
#include <QTimer>

#include <iostream>
#include <stdlib.h>

int QtTesting(void)
{
  // QtTestingClass *qttestingclass = new QtTestingClass;
  std::cout << "Testing ... " << std::endl;
  auto timer = new QTimer(QApplication::instance());
  timer->setSingleShot(true);
  QObject::connect(timer, SIGNAL(timeout()), QApplication::instance(), SLOT(quit()));
  timer->start(2000); // 2 seconds single-shot timer
  QApplication::instance()->exec();
  return EXIT_SUCCESS;
}
