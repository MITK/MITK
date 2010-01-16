/*=========================================================================
 
 Program:   BlueBerry Platform
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

  for (std::size_t i = 0; i < G_QT_ARGC; ++i)
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
}

}
