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

class MitkUSNavigationUI_EXPORT QmitkUSZoneManagementWidget : public QWidget
{
  Q_OBJECT

public slots:
  void AddRow();
  void RemoveSelectedRows();

protected slots:
  void OnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

public:
  explicit QmitkUSZoneManagementWidget(QWidget *parent = 0);
  ~QmitkUSZoneManagementWidget();

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

protected:
    QmitkUSZonesDataModel* m_ZonesDataModel;

    itk::SmartPointer<mitk::USZonesInteractor> m_Interactor;

private:
  Ui::QmitkUSZoneManagementWidget* ui;
};

#endif // QMITKUSZONEMANAGEMENTWIDGET_H
