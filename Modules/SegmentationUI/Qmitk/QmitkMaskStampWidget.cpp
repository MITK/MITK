/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMaskStampWidget.h"

#include <mitkLabelSetImage.h>
#include <mitkRenderingManager.h>
#include <mitkToolManagerProvider.h>

#include <QMessageBox>

QmitkMaskStampWidget::QmitkMaskStampWidget(QWidget *parent, const char * /*name*/)
  : QWidget(parent), m_ToolManager(nullptr), m_DataStorage(nullptr)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);
  m_ToolManager->ActivateTool(-1);

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary =
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isMask = mitk::NodePredicateAnd::New(isBinary, isImage);

  mitk::NodePredicateAnd::Pointer maskPredicate = mitk::NodePredicateAnd::New();
  maskPredicate->AddPredicate(isMask);
  maskPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  m_Controls.m_cbMaskNodeSelector->SetPredicate(maskPredicate);

  connect(m_Controls.m_pbStamp, SIGNAL(clicked()), this, SLOT(OnStamp()));
  connect(m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)));
  m_Controls.m_InformationWidget->hide();
}

QmitkMaskStampWidget::~QmitkMaskStampWidget()
{
}

void QmitkMaskStampWidget::SetDataStorage(mitk::DataStorage *storage)
{
  m_DataStorage = storage;
  m_Controls.m_cbMaskNodeSelector->SetDataStorage(m_DataStorage);
}

void QmitkMaskStampWidget::OnStamp()
{
  mitk::DataNode *maskNode = m_Controls.m_cbMaskNodeSelector->GetSelectedNode();

  if (!maskNode)
  {
    QMessageBox::information(this, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  mitk::Image *mask = dynamic_cast<mitk::Image *>(maskNode->GetData());
  if (!mask)
  {
    QMessageBox::information(this, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

  if (!workingNode)
  {
    QMessageBox::information(this, "Mask Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());

  if (!workingImage)
  {
    QMessageBox::information(this, "Mask Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  try
  {
    workingImage->MaskStamp(mask, m_Controls.m_chkOverwrite->isChecked());
  }
  catch (mitk::Exception &e)
  {
    QApplication::restoreOverrideCursor();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Mask Stamp", "Could not stamp the selected mask.\n See error log for details.\n");
    return;
  }

  QApplication::restoreOverrideCursor();

  maskNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMaskStampWidget::OnShowInformation(bool on)
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}
