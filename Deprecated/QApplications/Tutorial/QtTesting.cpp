#include <qapplication.h>
#include <qtimer.h>

#include <stdlib.h>
#include <iostream>

int QtTesting()
{
  //QtTestingClass *qttestingclass = new QtTestingClass;
  std::cout << "Testing ... " << std::endl;
  QTimer *timer = new QTimer( qApp );
  QObject::connect( timer, SIGNAL(timeout()), qApp, SLOT(quit()) );
  timer->start( 2000, TRUE ); // 2 seconds single-shot timer
  qApp->exec();
  return EXIT_SUCCESS;
}
