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

#include <QApplication>
#include <QMessageBox>
#include <QtSingleApplication>
#include <QtGlobal>
#include <QTime>
#include <QDesktopServices>

#include <usModuleSettings.h>

#include <mitkCommon.h>
#include <mitkException.h>

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
    catch (mitk::Exception& e)
    {
      msg = QString("MITK Exception:\n\n")
            + QString("Description: ")
            + QString(e.GetDescription()) + QString("\n\n")
            + QString("Filename: ") + QString(e.GetFile()) + QString("\n\n")
            + QString("Line: ") + QString::number(e.GetLine());
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
    MITK_ERROR << "An error occurred: " << msg.toStdString();

    QMessageBox msgBox;
    msgBox.setText("An error occurred. You should save all data and quit the program to prevent possible data loss.");
    msgBox.setDetailedText(msg);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.addButton(trUtf8("Exit immediately"), QMessageBox::YesRole);
    msgBox.addButton(trUtf8("Ignore"), QMessageBox::NoRole);

    int ret = msgBox.exec();

    switch(ret)
      {
      case 0:
        MITK_ERROR << "The program was closed.";
        this->closeAllWindows();
        break;
      case 1:
        MITK_ERROR << "The error was ignored by the user. The program may be in a corrupt state and don't behave like expected!";
        break;
      }

    return false;
  }

};


int main(int argc, char** argv)
{
  // Create a QApplication instance first
  QtSafeApplication qSafeApp(argc, argv);
  qSafeApp.setApplicationName("MITK Workbench");
  qSafeApp.setOrganizationName("DKFZ");

  // This function checks if an instance is already running
  // and either sends a message to it (containing the command
  // line arguments) or checks if a new instance was forced by
  // providing the BlueBerry.newInstance command line argument.
  // In the latter case, a path to a temporary directory for
  // the new application's storage directory is returned.
  QString storageDir = handleNewAppInstance(&qSafeApp, argc, argv, "BlueBerry.newInstance");

  if (storageDir.isEmpty())
  {
    // This is a new instance and no other instance is already running. We specify
    // the storage directory here (this is the same code as in berryInternalPlatform.cpp
    // so that we can re-use the location for the persistent data location of the
    // the CppMicroServices library.

    // Append a hash value of the absolute path of the executable to the data location.
    // This allows to start the same application from different build or install trees.
    storageDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + '_';
    storageDir += QString::number(qHash(QCoreApplication::applicationDirPath())) + "/";
  }
  us::ModuleSettings::SetStoragePath((storageDir + "us/").toStdString());

  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  Poco::Path basePath(argv[0]);
  basePath.setFileName("");

  Poco::Path provFile(basePath);
  provFile.setFileName("MitkWorkbench.provisioning");

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

#ifdef Q_OS_WIN
#define CTK_LIB_PREFIX
#else
#define CTK_LIB_PREFIX "lib"
#endif


  // Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.

#ifdef Q_OS_MAC

  QString applicationPath = QCoreApplication::applicationDirPath();
  applicationPath.append("/plugins/liborg_mitk_gui_qt_ext.dylib");

  QFile preloadLibrary (applicationPath);

  if (preloadLibrary.exists())
  {
    std::cout << "APP_PATH: "<<applicationPath.toStdString();
    applicationPath.append(",");
  }
  extConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY, "/plugins/liborg_mitk_gui_qt_ext.dylib," CTK_LIB_PREFIX "CTKDICOMCore:0.1");
#else
  extConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY, "liborg_mitk_gui_qt_ext," CTK_LIB_PREFIX "CTKDICOMCore:0.1");
#endif

  // Seed the random number generator, once at startup.
  QTime time = QTime::currentTime();
  qsrand((uint)time.msec());

  // Run the workbench.
  return berry::Starter::Run(argc, argv, extConfig);
}
