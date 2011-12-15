/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  safeApp.setApplicationName("CoreApp");
  safeApp.setOrganizationName("DKFZ");

  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  Poco::Path basePath(argv[0]);
  basePath.setFileName("");
  
  Poco::Path provFile(basePath);
  provFile.setFileName("CoreApp.provisioning");

  Poco::Util::MapConfiguration* coreConfig(new Poco::Util::MapConfiguration());
  coreConfig->setString(berry::Platform::ARG_PROVISIONING, provFile.toString());
  coreConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.application");
  return berry::Starter::Run(argc, argv, coreConfig);
}
