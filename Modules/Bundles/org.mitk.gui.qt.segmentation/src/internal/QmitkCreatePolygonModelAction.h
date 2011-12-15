#ifndef QMITKCREATEPOLYGONMODELACTION_H
#define QMITKCREATEPOLYGONMODELACTION_H

#include <org_mitk_gui_qt_segmentation_Export.h>

// Parent classes
#include <QObject>
#include <mitkIContextMenuAction.h>

// Data members
#include <mitkDataNode.h>

class QmitkStdMultiWidget;

class MITK_QT_SEGMENTATION QmitkCreatePolygonModelAction : public QObject, public mitk::IContextMenuAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkCreatePolygonModelAction();
  ~QmitkCreatePolygonModelAction();

  // IContextMenuAction
  void Run(const std::vector<mitk::DataNode *> &selectedNodes);
  void SetDataStorage(mitk::DataStorage *dataStorage);
  void SetStdMultiWidget(QmitkStdMultiWidget *stdMultiWidget);
  void SetSmoothed(bool smoothed);
  void SetDecimated(bool decimated);
  void SetFunctionality(berry::QtViewPart *functionality);

  void OnSurfaceCalculationDone();

private:
  QmitkCreatePolygonModelAction(const QmitkCreatePolygonModelAction &);
  QmitkCreatePolygonModelAction & operator=(const QmitkCreatePolygonModelAction &);

  mitk::DataStorage::Pointer m_DataStorage;
  QmitkStdMultiWidget *m_StdMultiWidget;
  bool m_IsSmoothed;
  bool m_IsDecimated;
};

#endif
