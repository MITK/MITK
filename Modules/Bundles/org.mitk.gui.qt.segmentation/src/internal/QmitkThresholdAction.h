#ifndef QMITKTHRESHOLDACTION_H
#define QMITKTHRESHOLDACTION_H

#include <org_mitk_gui_qt_segmentation_Export.h>

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataStorage.h>
#include <mitkToolManager.h>

class QDialog;
class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkThresholdAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkThresholdAction();
  ~QmitkThresholdAction();

  // IContextMenuAction
  void Run(const std::vector<mitk::DataNode *> &selectedNodes);
  void SetDataStorage(mitk::DataStorage *dataStorage);
  void SetStdMultiWidget(QmitkStdMultiWidget *stdMultiWidget);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart *functionality);

  void OnThresholdingToolManagerToolModified();

private slots:
  void ThresholdingDone(int);

private:
  QmitkThresholdAction(const QmitkThresholdAction &);
  QmitkThresholdAction & operator=(const QmitkThresholdAction &);

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ThresholdingToolManager;
  QDialog *m_ThresholdingDialog;
};

#endif
