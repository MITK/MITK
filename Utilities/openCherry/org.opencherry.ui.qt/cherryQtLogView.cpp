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

#include "cherryQtLogView.h"

#include "cherryQtUIPlugin.h"

#include <QHeaderView>

namespace cherry {

QtLogView::QtLogView(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	ui.tableView->setModel(QtUIPlugin::GetInstance()->GetLogModel());
	
	ui.tableView->verticalHeader()->setVisible(false);
}

QtLogView::~QtLogView()
{

}

}
