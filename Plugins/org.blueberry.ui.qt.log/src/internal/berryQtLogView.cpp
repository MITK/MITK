/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtLogView.h"

#include "berryQtLogPlugin.h"

#include <berryPlatform.h>
#include <berryPlatformUI.h>

#include <QHeaderView>

#include <QTimer>
#include <QClipboard>

#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace berry {

QtLogView::QtLogView(QWidget *parent)
    : QWidget(parent)
{
  auto* prefService = berry::Platform::GetPreferencesService();
  auto* prefs = prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log");


  prefs->PutBool("ShowAdvancedFields", false);
  prefs->PutBool("ShowCategory", true);
  bool showAdvancedFields = false;

  ui.setupUi(this);

  model = QtLogPlugin::GetInstance()->GetLogModel();
  model->SetShowAdvancedFiels( showAdvancedFields );

  filterModel = new QSortFilterProxyModel(this);
  filterModel->setSourceModel(model);
  filterModel->setFilterKeyColumn(-1);

#ifdef __APPLE__
  QFont fnt = ui.tableView->font();
  fnt.setPointSize(11);
  ui.tableView->setFont(fnt);
#endif
  ui.tableView->setModel(filterModel);
  ui.tableView->verticalHeader()->setVisible(false);
  ui.tableView->horizontalHeader()->setStretchLastSection(true);

  connect( ui.filterContent, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotFilterChange( const QString& ) ) );
  connect( filterModel, SIGNAL( rowsInserted ( const QModelIndex &, int, int ) ), this, SLOT( slotRowAdded( const QModelIndex &, int , int  ) ) );
  connect( ui.SaveToClipboard, SIGNAL( clicked()),this, SLOT(on_SaveToClipboard_clicked()));

  ui.ShowAdvancedFields->setChecked( showAdvancedFields );
  ui.filterContent->setClearButtonEnabled(true);
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
  QRegularExpression regExp(QRegularExpression::escape(q), QRegularExpression::CaseInsensitiveOption);
  filterModel->setFilterRegularExpression(regExp);
}


void QtLogView::slotRowAdded ( const QModelIndex &  /*parent*/, int /*start*/, int /*end*/ )
{
  ui.tableView->setVisible(false);
  ui.tableView->resizeRowsToContents();

  //only resize columns when first entry is added
  static bool first = true;
  if(first)
    {
    ui.tableView->resizeColumnsToContents();
    first = false;
    }
  ui.tableView->setVisible(true);

  QTimer::singleShot(0,this,SLOT( slotScrollDown() ) );
}

void QtLogView::showEvent( QShowEvent * /*event*/ )
{
    ui.tableView->setVisible(false);
  ui.tableView->resizeColumnsToContents();
  ui.tableView->resizeRowsToContents();
  ui.tableView->setVisible(true);
}

void QtLogView::on_ShowAdvancedFields_clicked( bool checked )
{
  ui.tableView->setVisible(false);
  QtLogPlugin::GetInstance()->GetLogModel()->SetShowAdvancedFiels( checked );
  ui.tableView->resizeColumnsToContents();
  ui.tableView->setVisible(true);

  auto* prefService = berry::Platform::GetPreferencesService();
  auto* prefs = prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log");

  prefs->PutBool("ShowAdvancedFields", checked);
  prefs->Flush();
}

void QtLogView::on_ShowCategory_clicked( bool checked )
{
  ui.tableView->setVisible(false);
  QtLogPlugin::GetInstance()->GetLogModel()->SetShowCategory( checked );
  ui.tableView->resizeColumnsToContents();
  ui.tableView->setVisible(true);

  auto* prefService = berry::Platform::GetPreferencesService();
  auto* prefs = prefService->GetSystemPreferences()->Node("org_blueberry_ui_qt_log");

  prefs->PutBool("ShowCategory", checked);
  prefs->Flush();
}

void QtLogView::on_SaveToClipboard_clicked()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString loggingMessagesAsText = QString("");
  for (int i=0; i<ui.tableView->model()->rowCount(); i++)
    {
    for (int j=0; j<ui.tableView->model()->columnCount(); j++)
      {
      QModelIndex index = ui.tableView->model()->index(i, j);
      loggingMessagesAsText += ui.tableView->model()->data(index, Qt::DisplayRole).toString() + " ";
      }
    loggingMessagesAsText += "\n";
    }

  clipboard->setText(loggingMessagesAsText);
}

}
