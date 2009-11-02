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


#ifndef CHERRYQTINTROPART_H_
#define CHERRYQTINTROPART_H_

#include <cherryIntroPart.h>

#include "cherryUiQtDll.h"

#include <QWidget>

namespace cherry
{

class CHERRY_UI_QT QtIntroPart : public IntroPart
{

public:

  cherryObjectMacro(QtIntroPart);

  void CreatePartControl(void* parent);

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;

};

}

#endif /* CHERRYQTINTROPART_H_ */
