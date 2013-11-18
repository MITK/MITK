#ifndef QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H
#define QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H

#include <QWidget>

#include "MitkUSNavigationUIExports.h"
#include "mitkUSCombinedModality.h"

namespace Ui {
class QmitkUSCombinedModalityCreationWidget;
}

class MitkUSNavigationUI_EXPORT QmitkUSCombinedModalityCreationWidget : public QWidget
{
  Q_OBJECT

signals:
  void SignalAborted();
  void SignalCreated(mitk::USCombinedModality::Pointer combinedModality);
  void SignalCreated();

protected slots:
  void OnCreation();
  void OnAbortion();

  void OnSelectedUltrasoundOrTrackingDevice();

public:
  explicit QmitkUSCombinedModalityCreationWidget(QWidget *parent = 0);
  ~QmitkUSCombinedModalityCreationWidget();

private:
  Ui::QmitkUSCombinedModalityCreationWidget *ui;
};

#endif // QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H
