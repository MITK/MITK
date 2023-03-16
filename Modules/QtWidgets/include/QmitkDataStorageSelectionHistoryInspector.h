/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageSelectionHistoryInspector_h
#define QmitkDataStorageSelectionHistoryInspector_h

#include <MitkQtWidgetsExports.h>

#include "QmitkAbstractDataStorageInspector.h"
#include <QSortFilterProxyModel>
#include "QmitkSimpleTextOverlayWidget.h"

#include "ui_QmitkDataStorageSelectionHistoryInspector.h"

/*
* @brief This is an inspector that offers a simple list view on the last selected nodes (in chronologic order) in a data storage.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageSelectionHistoryInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:
  QmitkDataStorageSelectionHistoryInspector(QWidget* parent = nullptr);

  QAbstractItemView* GetView() override;
  const QAbstractItemView* GetView() const override;

  void SetSelectionMode(SelectionMode mode) override;
  SelectionMode GetSelectionMode() const override;

  static void AddNodeToHistory(mitk::DataNode* node);
  static void ResetHistory();

  constexpr static const char* INSPECTOR_ID()
  {
    return "org.mitk.QmitkDataStorageSelectionHistoryInspector";
  };

protected:
  void Initialize() override;

  QmitkAbstractDataStorageModel* m_StorageModel;
  Ui_QmitkDataStorageSelectionHistoryInspector m_Controls;
  QmitkSimpleTextOverlayWidget* m_Overlay;
};

#endif
