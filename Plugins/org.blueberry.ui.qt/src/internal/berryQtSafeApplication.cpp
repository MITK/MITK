/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtSafeApplication.h"

#include <berryPlatform.h>

#include <QMessageBox>
#include <cstring>

namespace berry
{

int G_QT_ARGC = 0;
char** G_QT_ARGV = 0;

QtSafeApplication::QtSafeApplication(int& argc, char** argv) :
  QApplication(argc, argv)
{
}

QApplication* QtSafeApplication::CreateInstance()
{
  if (qApp)
    return qApp;

  // TODO extract the proper Qt command line arguments
  std::string name = Platform::GetConfiguration().getString(
      "application.name");
  G_QT_ARGC = 1;
  G_QT_ARGV = new char*[1];
  G_QT_ARGV[0] = new char[name.length() + 1];
  std::strcpy(G_QT_ARGV[0], name.c_str());

  return new QtSafeApplication(G_QT_ARGC, G_QT_ARGV);
}

QtSafeApplication::~QtSafeApplication()
{
  if (G_QT_ARGC == 0) return;

  for (int i = 0; i < G_QT_ARGC; ++i)
    delete[] G_QT_ARGV[i];

  delete[] G_QT_ARGV;
}

bool QtSafeApplication::notify(QObject* receiver, QEvent* event)
{
  QString msg;
  try
  {
    return QApplication::notify(receiver, event);
  } catch (Poco::Exception& e)
  {
    msg = QString::fromStdString(e.displayText());
  } catch (std::exception& e)
  {
    msg = e.what();
  } catch (...)
  {
    msg = "Unknown exception";
  }

  QString
      text(
          "An error occurred. You should save all data and quit the program to prevent possible data loss.\nSee the error log for details.\n\n");
  text += msg;
  QMessageBox::critical(0, "Error", text);
  return false;
}

}
