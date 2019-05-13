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
#include <QShortcut>

QmitkUSNavigationFreezeButton::QmitkUSNavigationFreezeButton(QWidget* parent) :
  QPushButton(parent),
  m_OutputIndex(-1),
  m_FreezeButtonToggle(true)
{
  this->setText("Freeze Imaging");
  this->setIcon(QIcon(":/USNavigation/system-lock-screen.png"));
  this->setCheckable(true);

  //set shortcuts
  QShortcut *shortcut = new QShortcut(QKeySequence("F12"), parent);
  connect(shortcut, SIGNAL(activated()), this, SLOT(OnFreezeButtonToggle()));

  connect(this, SIGNAL(clicked(bool)), this, SLOT(OnButtonClicked(bool)));
}

QmitkUSNavigationFreezeButton::~QmitkUSNavigationFreezeButton()
{
}

void QmitkUSNavigationFreezeButton::SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality, int outputIndex)
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
void QmitkUSNavigationFreezeButton::OnFreezeButtonToggle()
{
if(this->isVisible())
  {
  this->setChecked(m_FreezeButtonToggle);
  OnButtonClicked(m_FreezeButtonToggle);
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
    m_FreezeButtonToggle = true;
    return;
  }
  m_FreezeButtonToggle = !checked;

  // cannot do anything without a navigation data source
  mitk::NavigationDataSource::Pointer navigationDataSource = m_CombinedModality->GetNavigationDataSource();
  if ( navigationDataSource.IsNull() )
  {
    MITK_WARN("QmitkUSNavigationFreezeButton")
      << "Cannot freeze the device as the NavigationDataSource is null.";
    this->setChecked(false);
    return;
  }

  if (checked) //freezing
  {
  MITK_INFO << "Freezing";
  // freeze the imaging and the tracking
  m_CombinedModality->SetIsFreezed(true);

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
  emit SignalFreezed(true);
  }

  else //unfreezing
  {
  MITK_INFO << "Unfreezing";
  emit SignalFreezed(false);
  //m_CombinedModality->SetIsFreezed(false);//commented out to workaround bug: may only be unfreezed after critical structure was added
  }








}
