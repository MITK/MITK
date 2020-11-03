/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
