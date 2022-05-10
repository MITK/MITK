/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkStaticDynamicSegmentationDialog.h"

#include <mitkImageTimeSelector.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>

#include <QPushButton>

QmitkStaticDynamicSegmentationDialog::QmitkStaticDynamicSegmentationDialog(QWidget *parent)
  : QMessageBox(parent)
{
  this->setWindowModality(Qt::WindowModality::NonModal);
  this->setIcon(QMessageBox::Question);
  this->setWindowTitle(tr("Create a static or dynamic segmentation?"));
  this->setText(tr("The selected image has multiple time steps."));
  this->setInformativeText(tr("Do you want to create a static "
    "segmentation that is identical for all time steps or do you want to create a "
    "dynamic segmentation to segment individual time steps?"));

  QAbstractButton* staticButton = this->addButton(tr("Create static segmentation"), QMessageBox::AcceptRole);
  this->addButton(tr("Create dynamic segmentation"), QMessageBox::AcceptRole);

  // only the static button needs to be connected to take care of the special case, where
  // the time steps of the 4D image are combined into a single time step of the 3D segmentation.
  connect(staticButton, &QAbstractButton::clicked, this, &QmitkStaticDynamicSegmentationDialog::OnStaticButtonClicked);
}

void QmitkStaticDynamicSegmentationDialog::SetReferenceImage(const mitk::Image* referenceImage)
{
  m_ReferenceImage = referenceImage;
  m_SegmentationTemplate = referenceImage;

  const auto currentTimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  m_ImageTimeStep = 0;
  if (m_ReferenceImage->GetTimeGeometry()->IsValidTimePoint(currentTimePoint))
  {
    m_ImageTimeStep = m_ReferenceImage->GetTimeGeometry()->TimePointToTimeStep(currentTimePoint);
  }
}

mitk::Image::ConstPointer QmitkStaticDynamicSegmentationDialog::GetSegmentationTemplate() const
{
  return m_SegmentationTemplate;
}

void QmitkStaticDynamicSegmentationDialog::OnStaticButtonClicked(bool /*checked*/)
{
  auto selector = mitk::ImageTimeSelector::New();
  selector->SetInput(m_ReferenceImage);
  selector->SetTimeNr(m_ImageTimeStep);
  selector->Update();
  mitk::Image::Pointer newImage = selector->GetOutput();

  const auto referenceTimeGeometry = m_ReferenceImage->GetTimeGeometry();
  auto timeGeometry = mitk::ProportionalTimeGeometry::New();
  timeGeometry->SetFirstTimePoint(referenceTimeGeometry->GetMinimumTimePoint());
  timeGeometry->SetStepDuration(referenceTimeGeometry->GetMaximumTimePoint() - referenceTimeGeometry->GetMinimumTimePoint());
  timeGeometry->SetTimeStepGeometry(m_ReferenceImage->GetGeometry(m_ImageTimeStep), 0);

  newImage->SetTimeGeometry(timeGeometry);

  m_SegmentationTemplate = newImage;
}
