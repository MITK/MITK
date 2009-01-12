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

#ifndef CHERRYQTVIEWPART_H_
#define CHERRYQTVIEWPART_H_

#include <cherryViewPart.h>

#include "cherryUiQtDll.h"

#include <QWidget>

namespace cherry
{

class CHERRY_UI_QT QtViewPart : public ViewPart
{
  
public:
  
  cherryClassMacro(QtViewPart);
  
  void CreatePartControl(void* parent);
  
protected:
  
  ///
  /// Called immediately before CreateQtPartControl().
  /// Actions that should be taken before creating the controls are executed here. 
  ///
  virtual void BeforeCreateQtPartControl(QWidget* parent);
  virtual void CreateQtPartControl(QWidget* parent) = 0;

  ///
  /// Called immediately after CreateQtPartControl().
  /// Actions that should be taken after creating the controls are executed here. 
  ///
  virtual void AfterCreateQtPartControl(QWidget* parent);

};

}

#endif /*CHERRYQTVIEWPART_H_*/
