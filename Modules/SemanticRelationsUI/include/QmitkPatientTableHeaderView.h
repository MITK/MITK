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

#ifndef QMITKPATIENTTABLEHEADERVIEW_H
#define QMITKPATIENTTABLEHEADERVIEW_H

// semantic relations ui module
#include "MitkSemanticRelationsUIExports.h"

// qt
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPointer>

/*
* @brief 
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableHeaderView : public QHeaderView
{
  Q_OBJECT

public:

  QmitkPatientTableHeaderView(QWidget* parent = nullptr);
  ~QmitkPatientTableHeaderView();

  enum HeaderDataModelRoles
  {
    HorizontalHeaderDataRole = Qt::UserRole
  };

  /**
  * @brief Set the model of the table view of this header view.
  *   This model returns a standard item model for the 'HorizontalHeaderDataRole'-role.
  *   The header model has been previously filled with header data.
  *   This function is overwritten from QHeaderView.
  */
  virtual void setModel(QAbstractItemModel* model) override;

protected:

  /**
  * @brief Paint each header using 'PaintHeader'.
  *   This function is overwritten from QHeaderView.
  */
  virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
  /**
  * @brief Get the section size by retrieving the text content.
  *   The section size is dependent on the child and parent headers.
  *   This function is overwritten from QHeaderView.
  */
  QSize sectionSizeFromContents(int logicalIndex) const override;

private:

  int PaintHeader(QPainter* painter, const QModelIndex& currentIndex, int logicalIndex, const QRect& sectionRect, int top) const;
  QSize HeaderSize(const QModelIndex& index) const;
  int CurrentHeaderLeft(const QModelIndex& currentIndex, const QModelIndex& headerIndex, int sectionIndex, int left) const;
  int CurrentHeaderWidth(const QModelIndex& currentIndex, const QModelIndex& headerIndex, int sectionIndex) const;

  QModelIndexList ParentIndexList(QModelIndex index) const;
  QModelIndex HeaderIndex(int sectionIndex) const;
  QModelIndex FindHeader(const QModelIndex& currentIndex, int sectionIndex, int& currentHeaderIndex) const;
  QModelIndexList ListHeader(const QModelIndex& currentIndex) const;

  QStandardItemModel* m_HeaderModel;

};

#endif // QMITKPATIENTTABLEHEADERVIEW_H
