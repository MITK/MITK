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

public slots:
  //void AddRow();
  void RemoveSelectedRows();
  void OnStartAddingZone();
  void OnResetZones();

protected slots:
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection & deselected);
  void OnZoneSizeSliderValueChanged(int value);

  void OnRowInsertion(const QModelIndex& parent, int start, int end);

public:
  explicit QmitkUSZoneManagementWidget(QWidget *parent = 0);
  ~QmitkUSZoneManagementWidget();

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
  mitk::DataStorage::SetOfObjects::ConstPointer GetZoneNodes();

protected:
    QmitkUSZonesDataModel*                      m_ZonesDataModel;

    itk::SmartPointer<mitk::USZonesInteractor>  m_Interactor;
    mitk::DataStorage::Pointer                  m_DataStorage;
    mitk::DataNode::Pointer                     m_BaseNode;

    int                                         m_SelectedRow;

private:
  Ui::QmitkUSZoneManagementWidget* ui;
};

#endif // QMITKUSZONEMANAGEMENTWIDGET_H
