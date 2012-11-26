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

#include <application/berryStarter.h>
#include <Poco/Util/MapConfiguration.h>

#include <QMessageBox>
#include <QApplication>

#include "BlueBerryExampleLauncherDialog.h"

class QSafeApplication : public QApplication
{

public:

  QSafeApplication(int& argc, char** argv)
    : QApplication(argc, argv)
  {}

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject* receiver, QEvent* event)
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

    QString text("An error occurred. You should save all data and quit the program "
                 "to prevent possible data loss.\nSee the error log for details.\n\n");
    text += msg;
    QMessageBox::critical(0, "Error", text);
    return false;
  }

};

int main(int argc, char** argv)
{
  QSafeApplication safeApp(argc, argv);
  safeApp.setApplicationName("BlueBerryExampleLauncher");
  safeApp.setOrganizationName("DKFZ");

  BlueBerryExampleLauncherDialog demoDialog;
  QString selectedConfiguration = demoDialog.getDemoConfiguration();

  if (selectedConfiguration.isEmpty()) return EXIT_SUCCESS;

  Poco::Util::MapConfiguration* coreConfig(new Poco::Util::MapConfiguration());
  coreConfig->setString(berry::Platform::ARG_PROVISIONING, selectedConfiguration.toStdString());
//  coreConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.coreapplication");

  return berry::Starter::Run(argc, argv, coreConfig);
}
