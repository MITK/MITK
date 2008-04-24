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

#ifndef CHERRYQTPARTPANE_H_
#define CHERRYQTPARTPANE_H_

#include <org.opencherry.ui/cherryIViewPane.h>

#include <QtGui/QDockWidget>
#include <QtCore/QString>

namespace cherry {

class QtViewPane : public QDockWidget, public IViewPane
{
  Q_OBJECT
  
public:
  
  QtViewPane(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0 );
  
  void* GetControl();
  
  bool GetVisible();
  void SetVisible(bool visible);
  
  
};

}

#endif /*CHERRYQTPARTPANE_H_*/
