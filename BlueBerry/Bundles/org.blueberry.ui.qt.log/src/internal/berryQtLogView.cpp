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

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include "berryQtLogView.h"

#include "berryQtLogPlugin.h"

#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>

#include <QHeaderView>

#include <QTimer>

namespace berry {

QtLogView::QtLogView(QWidget *parent)
    : QWidget(parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log"))
        .Cast<berry::IBerryPreferences>();
  
  bool showAdvancedFields = 
      prefs->GetBool("ShowAdvancedFields", true) ;

  ui.setupUi(this);
  
  model = QtLogPlugin::GetInstance()->GetLogModel();
  model->SetShowAdvancedFiels( showAdvancedFields );
  
  filterModel = new QSortFilterProxyModel(this);
  filterModel->setSourceModel(model);
  filterModel->setFilterKeyColumn(-1);

  ui.tableView->setModel(filterModel);

  ui.tableView->verticalHeader()->setVisible(false);
  ui.tableView->horizontalHeader()->setStretchLastSection(true);
             
  connect( ui.filterContent, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilterChange( const QString& ) ) );
  connect( filterModel, SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ), this, SLOT( slotRowAdded( const QModelIndex &, int , int  ) ) );
  ui.ShowAdvancedFields->setChecked( showAdvancedFields );
           
}

QtLogView::~QtLogView()
{
}

void QtLogView::slotScrollDown( )
{
  ui.tableView->scrollToBottom();
}

void QtLogView::slotFilterChange( const QString& q )
{
  filterModel->setFilterRegExp(QRegExp(q, Qt::CaseInsensitive, QRegExp::FixedString));
}

void QtLogView::slotRowAdded ( const QModelIndex &  /*parent*/, int start, int end )
{
  static int first=false;

  if(!first)
  {
    first=true;
    ui.tableView->resizeColumnsToContents();
    ui.tableView->resizeRowsToContents();
  }
  else
    for(int r=start;r<=end;r++)
    {
      ui.tableView->resizeRowToContents(r);
    }

  QTimer::singleShot(0,this,SLOT( slotScrollDown() ) );
}

void QtLogView::on_ShowAdvancedFields_clicked( bool checked )
{
  QtLogPlugin::GetInstance()->GetLogModel()->SetShowAdvancedFiels( checked );  
  ui.tableView->resizeColumnsToContents();

  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log"))
        .Cast<berry::IBerryPreferences>();
  
  prefs->PutBool("ShowAdvancedFields", checked);
  prefs->Flush();
}

}
