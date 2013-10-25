#include "QmitkUSCombinedModalityManagerWidget.h"
#include "ui_QmitkUSCombinedModalityManagerWidget.h"

#include "mitkUSDevice.h"
#include "mitkUSCombinedModality.h"
#include "mitkNavigationDataSource.h"

QmitkUSCombinedModalityManagerWidget::QmitkUSCombinedModalityManagerWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::QmitkUSCombinedModalityManagerWidget)
{
    ui->setupUi(this);

    ui->m_USDevices->Initialize<mitk::USDevice>(mitk::USDevice::US_PROPKEY_LABEL);
    ui->m_TrackingDevices->Initialize<mitk::NavigationDataSource>(mitk::NavigationDataSource::US_PROPKEY_DEVICENAME);
    ui->m_CombinedModalities->Initialize<mitk::USDevice>(mitk::USCombinedModality::US_PROPKEY_LABEL,
                                                         "(" + mitk::USDevice::US_PROPKEY_CLASS + "=" + mitk::USCombinedModality::DeviceClassIdentifier + ")");
}

QmitkUSCombinedModalityManagerWidget::~QmitkUSCombinedModalityManagerWidget()
{
    delete ui;
}

void QmitkUSCombinedModalityManagerWidget::OnCreateCombinedModalityButtonClicked()
{
    mitk::USDevice::Pointer usDevice = ui->m_USDevices->GetSelectedService<mitk::USDevice>();
    if (usDevice.IsNull())
    {
        MITK_WARN << "No US Device selected for creation of Combined Modality.";
        return;
    }

    mitk::NavigationDataSource::Pointer trackingDevice = ui->m_TrackingDevices->GetSelectedService<mitk::NavigationDataSource>();
    if (trackingDevice.IsNull())
    {
        MITK_WARN << "No Traccking Device selected for creation of Combined Modality.";
        return;
    }

    mitk::USCombinedModality::Pointer combinedModality = mitk::USCombinedModality::New("", "");

    combinedModality->SetUltrasoundDevice(usDevice);
    combinedModality->SetTrackingDevice(trackingDevice);
    combinedModality->Initialize();
}

void QmitkUSCombinedModalityManagerWidget::OnRemoveCombinedModalityButtonClicked()
{

}
