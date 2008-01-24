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
