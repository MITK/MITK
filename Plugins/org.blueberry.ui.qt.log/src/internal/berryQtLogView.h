/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    QtLogView(QWidget *parent = nullptr);
    ~QtLogView() override;
    QtPlatformLogModel *model;
    QSortFilterProxyModel *filterModel;

private:
    Ui::QtLogViewClass ui;

    void showEvent ( QShowEvent * event ) override;

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
