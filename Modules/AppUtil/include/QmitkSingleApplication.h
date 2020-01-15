/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSINGLEAPPLICATION_H
#define QMITKSINGLEAPPLICATION_H

#include <MitkAppUtilExports.h>

#include <qtsingleapplication.h>

#include "QmitkSafeNotify.h"

class MITKAPPUTIL_EXPORT QmitkSingleApplication : public QtSingleApplication
{
public:
  QmitkSingleApplication(int &argc, char **argv, bool safeMode = true);

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject *receiver, QEvent *event) override;

  void setSafeMode(bool safeMode);

  bool getSafeMode() const;

private:
  bool m_SafeMode;
};

#endif // QMITKSINGLEAPPLICATION_H
