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

    QString
        text(
            "An error occurred. You should save all data and quit the program to prevent possible data loss.\nSee the error log for details.\n\n");
    text += msg;
    QMessageBox::critical(0, "Error", text);
    return false;
  }

};

int main(int argc, char** argv)
{
  QSafeApplication safeApp(argc, argv);
  safeApp.setApplicationName("MitkCoreApp");
  safeApp.setOrganizationName("DKFZ");

  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  QDir basePath(QCoreApplication::applicationDirPath());

  QString provFile = basePath.absoluteFilePath("MitkCoreApp.provisioning");

  Poco::Util::MapConfiguration* coreConfig(new Poco::Util::MapConfiguration());
  coreConfig->setString(berry::Platform::ARG_PROVISIONING.toStdString(), provFile.toStdString());
  coreConfig->setString(berry::Platform::ARG_APPLICATION.toStdString(), "org.mitk.qt.coreapplication");

  // Preload the org.mitk.gui.qt.common plug-in (and hence also Qmitk) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  coreConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY.toStdString(), "liborg_mitk_gui_qt_common");

  return berry::Starter::Run(argc, argv, coreConfig);
}
