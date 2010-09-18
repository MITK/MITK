#ifndef QMITK_TOOLROIDATASELECTIONBOX_H
#define QMITK_TOOLROIDATASELECTIONBOX_H

#include "QmitkExtExports.h"
#include "mitkToolManager.h"
#include "QmitkBoundingObjectWidget.h"
#include <QComboBox>

class QmitkExt_EXPORT QmitkToolRoiDataSelectionBox : public QWidget
{
  Q_OBJECT

public:

  QmitkToolRoiDataSelectionBox(QWidget* parent = 0, mitk::DataStorage* storage = 0, const char* name = 0);
  virtual ~QmitkToolRoiDataSelectionBox();

  mitk::DataStorage* GetDataStorage();
  void SetDataStorage(mitk::DataStorage& storage);

  mitk::ToolManager* GetToolManager();
  void SetToolManager(mitk::ToolManager& manager);

  void OnToolManagerRoiDataModified();

  mitk::ToolManager::DataVectorType GetSelection();

  void UpdateComboBoxData();

  void setEnabled(bool);

signals:
  void RoiDataSelected(const mitk::DataNode* node);
  protected slots:
    void OnRoiDataSelectionChanged(const QString& name);

protected:

  QmitkBoundingObjectWidget* m_boundingObjectWidget;
  QComboBox* m_segmentationComboBox;

  mitk::ToolManager::Pointer m_ToolManager;
  bool m_SelfCall;

  mitk::DataNode::Pointer m_lastSelection;
  QString m_lastSelectedName;
};
#endif