/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageTreeInspector_h
#define QmitkDataStorageTreeInspector_h

#include <MitkQtWidgetsExports.h>

#include <QmitkAbstractDataStorageInspector.h>
#include <QmitkSimpleTextOverlayWidget.h>
#include <QSortFilterProxyModel>
#include <memory>

namespace Ui
{
  class QmitkDataStorageTreeInspector;
}

/*
* @brief This is an inspector that offers a simple tree view on a data storage.
* Something like the "data manager plugin", but in simple/light (with less functionality)
* It uses the QmitkDataStorageSimpleTreeModel.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageTreeInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:
  QmitkDataStorageTreeInspector(QWidget* parent = nullptr);
  ~QmitkDataStorageTreeInspector() override;

  QAbstractItemView* GetView() override;
  const QAbstractItemView* GetView() const override;

  void SetSelectionMode(SelectionMode mode) override;
  SelectionMode GetSelectionMode() const override;

protected:
  void Initialize() override;
  void OnModelReset();

  QmitkAbstractDataStorageModel* m_StorageModel;
  std::unique_ptr<Ui::QmitkDataStorageTreeInspector> m_Controls;
  QmitkSimpleTextOverlayWidget* m_Overlay;
};

#endif
