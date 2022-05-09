/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSurfaceStampWidget.h"

#include <mitkLabelSetImage.h>
#include <mitkRenderingManager.h>
#include <mitkSurface.h>
#include <mitkToolManagerProvider.h>

#include <QMessageBox>

QmitkSurfaceStampWidget::QmitkSurfaceStampWidget(QWidget *parent, const char * /*name*/)
  : QWidget(parent), m_ToolManager(nullptr), m_DataStorage(nullptr)
{
  m_Controls.setupUi(this);
  m_Controls.m_InformationWidget->hide();

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  m_ToolManager->ActivateTool(-1);

  mitk::NodePredicateAnd::Pointer m_SurfacePredicate = mitk::NodePredicateAnd::New();
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateDataType::New("Surface"));
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  m_Controls.m_cbSurfaceNodeSelector->SetPredicate(m_SurfacePredicate);

  connect(m_Controls.m_pbStamp, SIGNAL(clicked()), this, SLOT(OnStamp()));
  connect(m_Controls.m_cbShowInformation, SIGNAL(toggled(bool)), this, SLOT(OnShowInformation(bool)));
  m_Controls.m_InformationWidget->hide();
}

QmitkSurfaceStampWidget::~QmitkSurfaceStampWidget()
{
}

void QmitkSurfaceStampWidget::SetDataStorage(mitk::DataStorage *storage)
{
  m_DataStorage = storage;
  m_Controls.m_cbSurfaceNodeSelector->SetDataStorage(m_DataStorage);
}

void QmitkSurfaceStampWidget::OnStamp()
{
  mitk::DataNode *surfaceNode = m_Controls.m_cbSurfaceNodeSelector->GetSelectedNode();

  if (!surfaceNode)
  {
    QMessageBox::information(this, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);
  m_ToolManager->ActivateTool(-1);

  mitk::Surface *surface = dynamic_cast<mitk::Surface *>(surfaceNode->GetData());
  if (!surface)
  {
    QMessageBox::information(this, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

  if (!workingNode)
  {
    QMessageBox::information(
      this, "Surface Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());

  if (!workingImage)
  {
    QMessageBox::information(
      this, "Surface Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  try
  {
    //    workingImage->SurfaceStamp( surface, m_Controls.m_chkOverwrite->isChecked() );
  }
  catch (mitk::Exception &e)
  {
    QApplication::restoreOverrideCursor();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Surface Stamp", "Could not stamp the selected surface.\n See error log for details.\n");
    return;
  }

  QApplication::restoreOverrideCursor();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceStampWidget::OnShowInformation(bool on)
{
  if (on)
    m_Controls.m_InformationWidget->show();
  else
    m_Controls.m_InformationWidget->hide();
}
