/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-07 13:46:44 +0200 (Di, 07 Sep 2010) $
Version:   $Revision: 25948 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKPYTHONVARIABLESTACKTREEWIDGET_H_
#define QMITKPYTHONVARIABLESTACKTREEWIDGET_H_

#include <QAbstractItemModel>
#include <QStringList>

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>
#include <QtGui/QTreeWidgetItem>

#include "QmitkPythonMediator.h"

class QmitkPythonVariableStackTreeWidget : public QAbstractTableModel, public QmitkPythonClient
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkPythonVariableStackTreeWidget(QWidget *parent = 0);
  virtual ~QmitkPythonVariableStackTreeWidget();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags( const QModelIndex& index ) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role) const;
  QStringList mimeTypes() const;

  void clear();
  void setVariableStack(std::vector<QStringList>);
  std::vector<QStringList> getVariableStack();

  QMimeData * mimeData(const QModelIndexList &) const;
  bool dropMimeData ( const QMimeData *, Qt::DropAction, int, int, const QModelIndex & );
  Qt::DropActions supportedDropActions() const;
  Qt::DropActions supportedDragActions() const;

  virtual void update();

protected:
    std::vector<QStringList> m_variableStack;
};

#endif
