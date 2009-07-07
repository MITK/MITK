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
	
	model = QtUIPlugin::GetInstance()->GetLogModel();
	
  filterModel = new QSortFilterProxyModel(this);
  filterModel->setSourceModel(model);
	filterModel->setFilterKeyColumn(-1);

	ui.tableView->setModel(filterModel);

	ui.tableView->verticalHeader()->setVisible(false);
	           
	connect( ui.filterContent, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilterChange( const QString& ) ) );
	connect( filterModel, SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ), this, SLOT( slotRowAdded( const QModelIndex &, int , int  ) ) );
           
	ui.tableView->resizeColumnsToContents();
	ui.tableView->resizeRowsToContents();
}

QtLogView::~QtLogView()
{
}

void QtLogView::slotFilterChange( const QString& q )
{
  filterModel->setFilterRegExp(QRegExp(q, Qt::CaseInsensitive, QRegExp::FixedString));
}


void QtLogView::slotRowAdded ( const QModelIndex & parent, int start, int end )
{
  for(int r=start;r<=end;r++)
  {
    ui.tableView->resizeRowToContents(r);
  }
  
  ui.tableView->scrollToBottom();
}


}
