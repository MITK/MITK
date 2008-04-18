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

#ifndef CHERRYQTLOGVIEW_H
#define CHERRYQTLOGVIEW_H

#include <QtGui/QWidget>

#include <org.opencherry.osgi/cherryDll.h>

#include "ui_cherryQtLogView.h"

namespace cherry {

class CHERRY_API QtLogView : public QWidget
{

public:
    QtLogView(QWidget *parent = 0);
    ~QtLogView();

private:
    Ui::QtLogViewClass ui;
};

}

#endif // CHERRYQTLOGVIEW_H
