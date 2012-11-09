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
  qSafeApp.setApplicationName("mitkDiffusion");
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
  provFile.setFileName("mitkDiffusion.provisioning");

  Poco::Util::MapConfiguration* diffConfig(new Poco::Util::MapConfiguration());
  if (!storageDir.isEmpty())
  {
    diffConfig->setString(berry::Platform::ARG_STORAGE_DIR, storageDir.toStdString());
  }

  diffConfig->setString(berry::Platform::ARG_PROVISIONING, provFile.toString());
  diffConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.diffusionimagingapp");

  // Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  diffConfig->setString(berry::Platform::ARG_PRELOAD_LIBRARY, "liborg_mitk_gui_qt_ext");

  return berry::Starter::Run(argc, argv, diffConfig);
}
