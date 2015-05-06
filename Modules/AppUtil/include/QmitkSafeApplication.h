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

#ifndef QMITKSAFEAPPLICATION_H
#define QMITKSAFEAPPLICATION_H

#include <MitkAppUtilExports.h>

#include <QMessageBox>
#include <QApplication>

class MITKAPPUTIL_EXPORT QmitkSafeApplication : public QApplication
{

public:

  QmitkSafeApplication(int& argc, char** argv);

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject* receiver, QEvent* event) override;

  void setSafeMode(bool safeMode);

  bool getSafeMode() const;

private:

  bool m_SafeMode;

};

#endif // QMITKSAFEAPPLICATION_H
