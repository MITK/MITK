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

#ifndef CHERRYQTOBJECTTABLEMODEL_H_
#define CHERRYQTOBJECTTABLEMODEL_H_

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QList>

#include <cherryIDebugObjectListener.h>

#include "cherryObjectItem.h"

namespace cherry
{

class QtObjectTableModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  QtObjectTableModel(QObject* parent = 0);
  ~QtObjectTableModel();

  QModelIndex index(int row, int column, const QModelIndex& parent =
      QModelIndex()) const;
  QModelIndex parent(const QModelIndex& index) const;

  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex& index, int) const;

  QVariant headerData(int section, Qt::Orientation orientation, int) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

  void ResetData();

protected:

  void ObjectCreated(const Object* obj);
  void ObjectDestroyed(const Object* obj);

  void SmartPointerCreated(unsigned int id, const Object* obj);
  void SmartPointerDestroyed(unsigned int id, const Object* obj);

private slots:

  void UpdatePendingData();

private:

  ObjectItem* FindObjectItem(const ObjectItem& item, int& index);

  QList<ObjectItem*> indexData;
  QList<ObjectItem*> pendingData;

  IDebugObjectListener::Pointer objectListener;
  friend class DebugObjectListener;

};

}

#endif /* CHERRYQTOBJECTTABLEMODEL_H_ */
