#ifndef QMITK_THRESHOLDACTION_H
#define QMITK_THRESHOLDACTION_H

#include <berryISelectionListener.h>
#include "berryQtViewPart.h"

#include "mitkIContextMenuAction.h"

#include "mitkDataStorage.h"
#include "mitkToolManager.h"

#include "org_mitk_gui_qt_segmentation_Export.h"

#include <QObject>
#include <QDialog>

class MITK_QT_SEGMENTATION QmitkThresholdAction: public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:

  QmitkThresholdAction();
  QmitkThresholdAction(const QmitkThresholdAction& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkThresholdAction();

  //interface methods
  void Run( const std::vector<mitk::DataNode*>& selectedNodes );
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart* functionality);

  // for receiving messages
  void OnThresholdingToolManagerToolModified();

protected:

  typedef std::vector<mitk::DataNode*> NodeList;

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ThresholdingToolManager;

  QDialog* m_ThresholdingDialog;

protected slots:

  // class internal slot
  void ThresholdingDone(int);
};

#endif // QMITK_THRESHOLDACTION_H
