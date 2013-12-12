#ifndef QMITKUSZONEMANAGEMENTWIDGET_H
#define QMITKUSZONEMANAGEMENTWIDGET_H

#include <QWidget>
#include "MitkUSNavigationUIExports.h"
#include "mitkDataStorage.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class USZonesInteractor;
}

namespace Ui {
class QmitkUSZoneManagementWidget;
}

class QmitkUSZonesDataModel;
class QItemSelection;
class QModelIndex;

class MitkUSNavigationUI_EXPORT QmitkUSZoneManagementWidget : public QWidget
{
  Q_OBJECT

signals:
  void ZoneAdded();

public slots:
  void RemoveSelectedRows();
  void OnStartAddingZone();
  void OnAbortAddingZone();
  void OnResetZones();

protected slots:
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection & deselected);
  void OnZoneSizeSliderValueChanged(int value);

  void OnRowInsertion(const QModelIndex& parent, int start, int end);
  void OnDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

public:
  explicit QmitkUSZoneManagementWidget(QWidget* parent = 0);
  ~QmitkUSZoneManagementWidget();

  void SetStateMachineFilename(const std::string& filename);
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage, const char* baseNodeName = "Zones");
  mitk::DataStorage::SetOfObjects::ConstPointer GetZoneNodes();

protected:
  QmitkUSZonesDataModel*                      m_ZonesDataModel;

  itk::SmartPointer<mitk::USZonesInteractor>  m_Interactor;
  mitk::DataStorage::Pointer                  m_DataStorage;
  mitk::DataNode::Pointer                     m_BaseNode;

  std::string                                 m_StateMachineFileName;

private:
  Ui::QmitkUSZoneManagementWidget* ui;

  unsigned int m_CurMaxNumOfZones;
};

#endif // QMITKUSZONEMANAGEMENTWIDGET_H
