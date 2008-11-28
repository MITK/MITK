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

#ifndef CHERRYQTEDITORPART_H_
#define CHERRYQTEDITORPART_H_

#include <cherryEditorPart.h>
#include <QWidget>

#include "cherryUiQtDll.h"

namespace cherry
{

class CHERRY_UI_QT QtEditorPart : public EditorPart
{
public:
  
  cherryClassMacro(QtEditorPart);
  
  void CreatePartControl(void* parent);
  
protected:
  
  virtual void CreateQtPartControl(QWidget* parent) = 0;
};

}
#endif /*CHERRYQTEDITORPART_H_*/
