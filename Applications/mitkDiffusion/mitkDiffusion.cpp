/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 13820 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <application/berryStarter.h>
#include <Poco/Util/MapConfiguration.h>

#include <QPixmap>
#include <QBitmap>
#include <QSplashScreen>
#include <QTimer>
#include <QApplication>

#include <Poco/Exception.h>
#include <QMessageBox>
#include <QApplication>

class mitkDiffusionImagingSafeQApplication : public QApplication
{

public:

  mitkDiffusionImagingSafeQApplication(int& argc, char** argv);

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject* receiver, QEvent* event);

};

mitkDiffusionImagingSafeQApplication::mitkDiffusionImagingSafeQApplication(int& argc, char** argv)
: QApplication(argc, argv)
{

}

bool mitkDiffusionImagingSafeQApplication::notify(QObject* receiver, QEvent* event)
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

int main(int argc, char** argv)
{

  mitkDiffusionImagingSafeQApplication app(argc, argv);
  app.setApplicationName("mitkDiffusion");
  app.setOrganizationName("DKFZ");
  bool showSplashScreen(true);

  QPixmap pixmap( ":/splash/splashscreen.png" );
  QSplashScreen splash( pixmap );
  splash.setMask( pixmap.mask() );
  splash.setWindowFlags( Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );

  if (showSplashScreen)
  {
    splash.show();

    app.sendPostedEvents();
    app.processEvents();
    app.flush();

    QTimer::singleShot(4000, &splash, SLOT(close()) );
  }

  // These paths replace the .ini file and are tailored for installation
  // packages created with CPack. If a .ini file is presented, it will
  // overwrite the settings in MapConfiguration
  Poco::Path basePath(argv[0]);
  basePath.setFileName("");

  Poco::Path provFile(basePath);
  provFile.setFileName("mitkDiffusion.provisioning");

  Poco::Util::MapConfiguration* sbConfig(new Poco::Util::MapConfiguration());
  sbConfig->setString(berry::Platform::ARG_PROVISIONING, provFile.toString());
  sbConfig->setString(berry::Platform::ARG_APPLICATION, "org.mitk.qt.diffusionimagingapp");
  return berry::Starter::Run(argc, argv, sbConfig);

}
