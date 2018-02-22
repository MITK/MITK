/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H
#define QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H

#include <QWidget>

#include "mitkUSCombinedModality.h"

namespace Ui {
class QmitkUSCombinedModalityCreationWidget;
}

/**
 * \brief Widget that enables the user to create a mitk::USCombinedModality of a mitk::NavigationDataSource and a mitk::USDevice.
 * A mitk::NavigationDataSource and a mitk::USDevice can be picked from two lists, showing
 * the corresponding micro service objects. The combined modality is registered as a micro
 * service as well during the creation process.
 */
class QmitkUSCombinedModalityCreationWidget : public QWidget
{
  Q_OBJECT

signals:
  /** \brief Emmited when the user clicks the "Cancel" button. */
  void SignalAborted();

  /** \brief Emmited when the user clicked the "Create" button and the creation is successfull. */
  void SignalCreated(mitk::USCombinedModality::Pointer combinedModality);

  /** \brief Emmited when the user clicked the "Create" button and the creation is successfull. */
  void SignalCreated();

protected slots:
  /** \brief Creates a combined modility of the selected mitk::NavigationDataSource and mitk::USDevice. **/
  void OnCreation();

  /** \brief Handles the enabled state of the "Create" button. **/
  void OnSelectedUltrasoundOrTrackingDevice();

public:
  explicit QmitkUSCombinedModalityCreationWidget(QWidget *parent = 0);
  ~QmitkUSCombinedModalityCreationWidget();

private:
  Ui::QmitkUSCombinedModalityCreationWidget *ui;

  itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> m_CombinedModality;
};

#endif // QMITKUSCOMBINEDMODALITYCREATIONWIDGET_H
