/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSAFENOTIFY_H
#define QMITKSAFENOTIFY_H

#include <mitkException.h>
#include <mitkLogMacros.h>

#include <QMessageBox>

template <class A>
bool QmitkSafeNotify(A *app, QObject *receiver, QEvent *event)
{
  QString msg;
  try
  {
    return app->A::notify(receiver, event);
  }
  catch (mitk::Exception &e)
  {
    msg = QString("MITK Exception:\n\n") + QString("Description: ") + QString(e.GetDescription()) + QString("\n\n") +
          QString("Filename: ") + QString(e.GetFile()) + QString("\n\n") + QString("Line: ") +
          QString::number(e.GetLine());
  }
  catch (std::exception &e)
  {
    msg = e.what();
  }
  catch (...)
  {
    msg = "Unknown exception";
  }
  MITK_ERROR << "An error occurred: " << msg.toStdString();

  QMessageBox msgBox;
  msgBox.setText("An error occurred. You should save all data and quit the program to prevent possible data loss.");
  msgBox.setDetailedText(msg);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.addButton(app->trUtf8("Exit immediately"), QMessageBox::YesRole);
  msgBox.addButton(app->trUtf8("Ignore"), QMessageBox::NoRole);

  int ret = msgBox.exec();

  switch (ret)
  {
    case 0:
      MITK_ERROR << "The program was closed.";
      app->closeAllWindows();
      break;
    case 1:
      MITK_ERROR
        << "The error was ignored by the user. The program may be in a corrupt state and don't behave like expected!";
      break;
  }

  return false;
}

#endif // QMITKSAFENOTIFY_H
