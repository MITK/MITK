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
#include <QSortFilterProxyModel>

#include "../cherryUiQtDll.h"

#include "ui_cherryQtLogView.h"

#include "cherryQtPlatformLogModel.h"

namespace cherry {

class CHERRY_UI_QT QtLogView : public QWidget
{
    Q_OBJECT
    
public:
    QtLogView(QWidget *parent = 0);
    ~QtLogView();
	  QtPlatformLogModel *model;
	  QSortFilterProxyModel *filterModel;
	  
private:
    Ui::QtLogViewClass ui;
    
protected slots:
    void slotFilterChange( const QString& );
    void slotRowAdded( const QModelIndex & , int , int  );
    
};

}

#endif // CHERRYQTLOGVIEW_H
