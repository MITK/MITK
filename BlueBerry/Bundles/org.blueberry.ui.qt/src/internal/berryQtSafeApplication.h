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


#ifndef BERRYQTSAFEAPPLICATION_H_
#define BERRYQTSAFEAPPLICATION_H_

#include <QApplication>

namespace berry {

class QtSafeApplication : public QApplication
{

private:

  QtSafeApplication(int& argc, char** argv);

public:

  static QApplication* CreateInstance();

  ~QtSafeApplication();

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject* receiver, QEvent* event);

};

}

#endif /* BERRYQTSAFEAPPLICATION_H_ */
