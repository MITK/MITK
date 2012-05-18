/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <QApplication>
#include <QTimer>

#include <stdlib.h>
#include <iostream>

int QtTesting()
{
  //QtTestingClass *qttestingclass = new QtTestingClass;
  std::cout << "Testing ... " << std::endl;
  QTimer *timer = new QTimer( QApplication::instance() );
  timer->setSingleShot(true);
  QObject::connect( timer, SIGNAL(timeout()), QApplication::instance(), SLOT(quit()) );
  timer->start( 2000 ); // 2 seconds single-shot timer
  QApplication::instance()->exec();
  return EXIT_SUCCESS;
}
