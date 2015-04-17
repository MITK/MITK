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

#include "QmitkContourModelToImageWidget.h"
#include "mitkImage.h"
#include "../../Common/QmitkDataSelectionWidget.h"

#include <mitkContourModelUtils.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkExtractSliceFilter.h>
#include <mitkProgressBar.h>
#include <mitkSegTool2D.h>
#include <mitkSliceNavigationController.h>
#include <mitkVtkImageOverwrite.h>

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <qmessagebox.h>

static const char* const HelpText = "Select a image and a contour(set)";

QmitkContourModelToImageWidget::QmitkContourModelToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImageAndSegmentationPredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ContourModelPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.btnProcess, SIGNAL(pressed()), this, SLOT(OnProcessPressed()));

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
          this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
      m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkContourModelToImageWidget::~QmitkContourModelToImageWidget()
{
}

void QmitkContourModelToImageWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull() )
  {
    this->EnableButtons(false);
    dataSelectionWidget->SetHelpText(HelpText);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkContourModelToImageWidget::SelectionControl(unsigned int index, const mitk::DataNode* /*selection*/)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  dataSelectionWidget->SetHelpText("");
  this->EnableButtons();
}

void QmitkContourModelToImageWidget::EnableButtons(bool enable)
{
  m_Controls.btnProcess->setEnabled(enable);
}

void QmitkContourModelToImageWidget::OnProcessPressed()
{
  QFutureWatcher<void> watcher;
  connect(&watcher, SIGNAL(finished()), this, SLOT(OnProcessingFinished()));

  QFuture<void> future = QtConcurrent::run(this, &QmitkContourModelToImageWidget::FillContourModelSetIntoImage);

  watcher.setFuture(future);
}

void QmitkContourModelToImageWidget::FillContourModelSetIntoImage()
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  mitk::DataNode::Pointer imageNode = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer contourNode = dataSelectionWidget->GetSelection(1);

  if(imageNode.IsNull() || contourNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain valid data";
    QMessageBox::information( this, "Contour To Image",
                              "Selection does not contain valid data, please select a binary image and a contour(set)", QMessageBox::Ok );
    m_Controls.btnProcess->setEnabled(false);
    return;
  }

  mitk::Image::Pointer image = static_cast<mitk::Image*>(imageNode->GetData());

  if (image.IsNull())
  {
    MITK_ERROR<<"Error writing contours into image! Invalid image data selected!";
    return;
  }

  unsigned int timeStep = this->GetTimeNavigationController()->GetTime()->GetPos();

  mitk::ContourModelSet::Pointer contourSet;
  mitk::ContourModel::Pointer contour = dynamic_cast<mitk::ContourModel*>(contourNode->GetData());
  if (contour.IsNotNull())
  {
    contourSet = mitk::ContourModelSet::New();
    contourSet->AddContourModel(contour);
  }
  else
  {
    contourSet = static_cast<mitk::ContourModelSet*>(contourNode->GetData());
    if (contourSet.IsNull())
    {
      MITK_ERROR<<"Error writing contours into binary image! Invalid contour data selected!";
      return;
    }
  }

  //Disable Buttons during calculation and initialize Progressbar
  this->EnableButtons(false);

  // Use mitk::ContourModelSetToImageFilter to fill the ContourModelSet into the image
  mitk::ContourModelSetToImageFilter::Pointer contourFiller = mitk::ContourModelSetToImageFilter::New();
  contourFiller->SetTimeStep(timeStep);
  contourFiller->SetImage(image);
  contourFiller->SetInput(contourSet);
  contourFiller->MakeOutputBinaryOn();
  contourFiller->Update();

  // Add result to data storage
  mitk::Image::Pointer result = contourFiller->GetOutput();
  if (result.IsNull())
  {
    MITK_ERROR<<"Could not write the selected contours into the image!";
    return;
  }

  result->DisconnectPipeline();
  mitk::DataNode::Pointer filled = mitk::DataNode::New();
  std::stringstream stream;
  stream << imageNode->GetName();
  stream << "_";
  stream << contourNode->GetName();
  filled->SetName(stream.str());
  filled->SetData(result);
  dataSelectionWidget->GetDataStorage()->Add(filled, imageNode);

  this->EnableButtons();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkContourModelToImageWidget::OnProcessingFinished()
{
  // Called when processing finished, nothing has to be done here
}
