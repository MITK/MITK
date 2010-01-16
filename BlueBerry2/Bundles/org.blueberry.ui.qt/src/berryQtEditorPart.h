/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifndef BERRYQTEDITORPART_H_
#define BERRYQTEDITORPART_H_

#include <berryEditorPart.h>
#include <QWidget>

#include "berryUiQtDll.h"

namespace berry
{

class BERRY_UI_QT QtEditorPart : public EditorPart
{
public:
  
  osgiObjectMacro(QtEditorPart);
  
  void CreatePartControl(void* parent);
  
protected:
  
  virtual void CreateQtPartControl(QWidget* parent) = 0;
};

}
#endif /*BERRYQTEDITORPART_H_*/
