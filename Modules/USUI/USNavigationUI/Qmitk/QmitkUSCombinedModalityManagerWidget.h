#ifndef QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H
#define QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H

#include <QWidget>

#include "MitkUSNavigationUIExports.h"
#include "mitkUSCombinedModality.h"

namespace Ui {
class QmitkUSCombinedModalityManagerWidget;
}

class MitkUSNavigationUI_EXPORT QmitkUSCombinedModalityManagerWidget : public QWidget
{
    Q_OBJECT

signals:
    /**
     * \brief Signals the selected combined modality.
     * The given object will be null if no combined modality is selected.
     */
    void SignalCombinedModalitySelected(mitk::USCombinedModality::Pointer);

protected slots:
    /**
     * \brief Triggered, when the user selects a device from either the list of USDevices or Tracking Devices
     */
    void OnSelectedUltrasoundOrTrackingDevice();

    /**
     * \brief Triggered, when the user selects a defice from the list of USCombinedModilites
     */
    void OnSelectedCombinedModality();

    void OnCreateCombinedModalityButtonClicked();
    void OnRemoveCombinedModalityButtonClicked();

public:
    explicit QmitkUSCombinedModalityManagerWidget(QWidget *parent = 0);
    ~QmitkUSCombinedModalityManagerWidget();

    itkGetMacro(SelectedCombinedModality, mitk::USCombinedModality::Pointer)

protected:
    mitk::USCombinedModality::Pointer m_SelectedCombinedModality;

private:
    Ui::QmitkUSCombinedModalityManagerWidget *ui;
};

#endif // QMITKUSCOMBINEDMODALITYMANAGERWIDGET_H
