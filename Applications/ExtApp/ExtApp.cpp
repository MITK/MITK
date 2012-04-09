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

#include <QApplication>
#include <QMessageBox>
#include <QtSingleApplication>

class QtSafeApplication : public QtSingleApplication
{

public:

  QtSafeApplication(int& argc, char** argv) : QtSingleApplication(argc, argv)
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
    }
    catch (Poco::Exception& e)
    {
      msg = QString::fromStdString(e.displayText());
    }
    catch (std::exception& e)
    {
      msg = e.what();
    }
    catch (...)
    {
      msg = "Unknown exception";
    }

    QString text("An error occurred. You should save all data and quit the program to "
                 "prevent possible data loss.\nSee the error log for details.\n\n");
    text += msg;

    QMessageBox::critical(0, "Error", text);
    return false;
  }

};


int main(int argc, char** argv)
{
  // Create a QApplication instance first
  QtSafeApplication qSafeApp(argc, argv);
  qSafeApp.setApplicationName("ExtApp");
  qSafeApp.setOrganizationName("DKFZ");

  // This function checks if an instance is already running
  // and either sends a message to it (containing the command
  // line arguments) or checks if a new instance was forced by
  // providing the BlueBerry.newInstance command line argument.
  // In the latter case, a path to a temporary directory for
  // the new application's storage directory is returned.
  QString storageDir = handleNewAppInstance(&qSafeApp, argc, argv, "BlueBerry.newInstance");

  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  Poco::Path basePath(argv[0]);
  basePath.setFileName("");
  
  Poco::Path provFile(basePath);
  provFile.setFileName("ExtApp.provisioning");

  Poco::Path extPath(basePath);
  extPath.pushDirectory("ExtBundles");

  std::string pluginDirs = extPath.toString();

  Poco::Util::MapConfiguration* extConfig(new Poco::Util::MapConfiguration());
  if (!storageDir.isEmpty())
  {
    extConfig->setString(berry::Platform::ARG_STORAGE_DIR, storageDir.toStdString());
  }
  extConfig->setString(berry::Platform::ARG_PLUGIN_DIRS, pluginDirs);
  extConfig->setString(berry::Platform::ARG_PROVISIONING, provFile.toString());
  extConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.extapplication");

  // Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  extConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY, "liborg_mitk_gui_qt_ext");

  return berry::Starter::Run(argc, argv, extConfig);
}
