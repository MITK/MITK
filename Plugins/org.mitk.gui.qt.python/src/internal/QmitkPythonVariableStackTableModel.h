/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPythonVariableStackTableModel_h
#define QmitkPythonVariableStackTableModel_h

#include <QAbstractTableModel>
#include <QStringList>
#include <QVariant>
#include <QModelIndex>
#include <QModelIndex>
#include <Python.h>

///
/// implements a table model to show the variables of the Python "__main__" dictionary
/// furthermore implements dragging and dropping of datanodes (conversion from and to python)
///
class QmitkPythonVariableStackTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  QmitkPythonVariableStackTableModel(QObject *parent = 0);
  virtual ~QmitkPythonVariableStackTableModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags( const QModelIndex& index ) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role) const;
  QStringList mimeTypes() const;

  void clear();
  void setVariableStack(QList<QStringList>);
  QList<QStringList> getVariableStack();

  QMimeData * mimeData(const QModelIndexList &) const;
  bool dropMimeData ( const QMimeData *, Qt::DropAction, int, int, const QModelIndex & );
  Qt::DropActions supportedDropActions() const;
  Qt::DropActions supportedDragActions() const;

public slots:
  void Update();
protected:
  QList<QStringList> getAttributeList();
  PyObject* getPyObject(PyObject* object);
  PyObject* getPyObjectString(const QString& objectName);
private:
  QList<QStringList> m_VariableStack;
};

#endif // QmitkPythonVariableStackTableModel_h
