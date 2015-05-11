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

#include "QmitkUSNavigationFreezeButton.h"

QmitkUSNavigationFreezeButton::QmitkUSNavigationFreezeButton(QWidget* parent) :
  QPushButton(parent),
  m_OutputIndex(-1)
{
  this->setText("Freeze Imaging");
  this->setIcon(QIcon(":/USNavigation/system-lock-screen.png"));
  this->setCheckable(true);

  connect(this, SIGNAL(clicked(bool)), this, SLOT(OnButtonClicked(bool)));
}

QmitkUSNavigationFreezeButton::~QmitkUSNavigationFreezeButton()
{
}

void QmitkUSNavigationFreezeButton::SetCombinedModality(mitk::USCombinedModality::Pointer combinedModality, int outputIndex)
{
  m_CombinedModality = combinedModality;
  m_OutputIndex = outputIndex;
}

void QmitkUSNavigationFreezeButton::Freeze()
{
  if ( ! this->isChecked() )
  {
    this->setChecked(true);
    this->OnButtonClicked(true);
  }
}

void QmitkUSNavigationFreezeButton::Unfreeze()
{
  if ( this->isChecked() )
  {
    this->setChecked(false);
    this->OnButtonClicked(false);
  }
}

void QmitkUSNavigationFreezeButton::OnButtonClicked(bool checked)
{
  // cannot do anything without a combined modality
  if ( m_CombinedModality.IsNull() )
  {
    MITK_WARN("QmitkUSNavigationFreezeButton")
      << "Cannot freeze the device as the device is null.";
    this->setChecked(false);
    return;
  }

  // freeze the imaging and the tracking
  m_CombinedModality->SetIsFreezed(checked);

  // cannot do anything more without a navigation data source
  mitk::NavigationDataSource::Pointer navigationDataSource = m_CombinedModality->GetNavigationDataSource();
  if ( navigationDataSource.IsNull() )
  {
    MITK_WARN("QmitkUSNavigationFreezeButton")
      << "Cannot freeze the device as the NavigationDataSource is null.";
    this->setChecked(false);
    return;
  }

  if ( m_OutputIndex >= 0 )
  {
    // make sure that the navigation data is up to date
    navigationDataSource->Update();

    // unfreeze if the navigation data got invalid during the time between
    // the button click and the actual freeze
    if ( checked &&
      ! navigationDataSource->GetOutput(m_OutputIndex)->IsDataValid() )
    {
      MITK_WARN("QmitkUSNavigationStepZoneMarking")("QmitkUSNavigationStepTumourSelection")
        << "Unfreezing device as the last tracking data of the reference sensor wasn't valid.";

      m_CombinedModality->SetIsFreezed(false);
      this->setChecked(false);
      return;
    }
  }

  emit SignalFreezed(checked);
}
