/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYQTLOGVIEW_H
#define BERRYQTLOGVIEW_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include "ui_berryQtLogView.h"

#include "berryQtPlatformLogModel.h"

namespace berry {

class QtLogView : public QWidget
{
    Q_OBJECT

public:
    QtLogView(QWidget *parent = 0);
    ~QtLogView();
    QtPlatformLogModel *model;
    QSortFilterProxyModel *filterModel;

private:
    Ui::QtLogViewClass ui;

    void showEvent ( QShowEvent * event );

protected slots:
    void slotFilterChange( const QString& );
    void slotRowAdded( const QModelIndex & , int , int  );
    void slotScrollDown( );
    void on_ShowAdvancedFields_clicked( bool checked = false );
    void on_ShowCategory_clicked( bool checked = false );
    void on_SaveToClipboard_clicked();

};

}

#endif // BERRYQTLOGVIEW_H
