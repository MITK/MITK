/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYQTSAFEAPPLICATION_H_
#define CHERRYQTSAFEAPPLICATION_H_

#include <QApplication>

namespace cherry {

class QtSafeApplication : public QApplication
{

private:

  QtSafeApplication(int& argc, char** argv);

public:

  static QApplication* CreateInstance();

  ~QtSafeApplication();

  /**
   * Reimplement notify to catch unhandled exceptions and open an error message.
   *
   * @param receiver
   * @param event
   * @return
   */
  bool notify(QObject* receiver, QEvent* event);

};

}

#endif /* CHERRYQTSAFEAPPLICATION_H_ */
