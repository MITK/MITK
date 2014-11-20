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

#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QTimer>
#include <QTime>

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
  qSafeApp.setApplicationName("MitkDiffusion");
  qSafeApp.setOrganizationName("DKFZ");

  bool showSplashScreen(true);

  QPixmap pixmap( ":/splash/splashscreen.png" );
  QSplashScreen splash( pixmap );
  splash.setMask( pixmap.mask() );
  splash.setWindowFlags( Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );

  if (showSplashScreen)
  {
    splash.show();

    qSafeApp.sendPostedEvents();
    qSafeApp.processEvents();
    qSafeApp.flush();

    QTimer::singleShot(4000, &splash, SLOT(close()) );
  }

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
  provFile.setFileName("MitkDiffusion.provisioning");

  Poco::Util::MapConfiguration* diffConfig(new Poco::Util::MapConfiguration());
  if (!storageDir.isEmpty())
  {
    diffConfig->setString(berry::Platform::ARG_STORAGE_DIR, storageDir.toStdString());
  }

  diffConfig->setString(berry::Platform::ARG_PROVISIONING, provFile.toString());
  diffConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.diffusionimagingapp");

  QStringList preloadLibs;

  // Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  preloadLibs << "liborg_mitk_gui_qt_ext";

  QMap<QString, QString> preloadLibVersion;

#ifdef Q_OS_MAC
  const QString libSuffix = ".dylib";
#elif defined(Q_OS_UNIX)
  const QString libSuffix = ".so";
#elif defined(Q_OS_WIN)
  const QString libSuffix = ".dll";
#else
  const QString libSuffix;
#endif

  for (QStringList::Iterator preloadLibIter = preloadLibs.begin(),
       iterEnd = preloadLibs.end(); preloadLibIter != iterEnd; ++preloadLibIter)
  {
    QString& preloadLib = *preloadLibIter;
    // In case the application is started from an install directory
    QString tempLibraryPath = QCoreApplication::applicationDirPath() + "/plugins/" + preloadLib + libSuffix;
    QFile preloadLibrary (tempLibraryPath);
#ifdef Q_OS_MAC
    if (!preloadLibrary.exists())
    {
      // In case the application is started from a build tree
      QString relPath = "/../../../plugins/" + preloadLib + libSuffix;
      tempLibraryPath = QCoreApplication::applicationDirPath() + relPath;
      preloadLibrary.setFileName(tempLibraryPath);
    }
#endif
    if(preloadLibrary.exists())
    {
      preloadLib = tempLibraryPath;
    }
    // Else fall back to the QLibrary search logic
  }

  QString preloadConfig;
  Q_FOREACH(const QString& preloadLib, preloadLibs)
  {
    preloadConfig += preloadLib + preloadLibVersion[preloadLib] + ",";
  }
  preloadConfig.chop(1);

  diffConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY, preloadConfig.toStdString());

  // Seed the random number generator, once at startup.
  QTime time = QTime::currentTime();
  qsrand((uint)time.msec());

  return berry::Starter::Run(argc, argv, diffConfig);
}
