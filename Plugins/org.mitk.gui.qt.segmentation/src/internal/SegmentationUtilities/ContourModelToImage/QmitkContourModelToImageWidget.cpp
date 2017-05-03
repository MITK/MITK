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

#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkSliceNavigationController.h>

#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <qmessagebox.h>

static const char* const HelpText = "Select a image and a contour(set)";

class QmitkContourModelToImageWidgetPrivate
{
public:
  QmitkContourModelToImageWidgetPrivate();
  ~QmitkContourModelToImageWidgetPrivate();

  /** @brief Check if selections is valid. */
  void SelectionControl( unsigned int index, const mitk::DataNode* selection);

  /** @brief Enable buttons if data selction is valid. */
  void EnableButtons(bool enable = true);

  /** @brief Does the actual contour filling */
  mitk::Image::Pointer FillContourModelSetIntoImage(mitk::Image *image, mitk::ContourModelSet *contourSet, unsigned int timeStep);

  Ui::QmitkContourModelToImageWidgetControls m_Controls;
  QFutureWatcher<mitk::Image::Pointer> m_Watcher;
};

QmitkContourModelToImageWidgetPrivate::QmitkContourModelToImageWidgetPrivate()
{
}

QmitkContourModelToImageWidgetPrivate::~QmitkContourModelToImageWidgetPrivate()
{
}

void QmitkContourModelToImageWidgetPrivate::EnableButtons(bool enable)
{
  m_Controls.btnProcess->setEnabled(enable);
}

void QmitkContourModelToImageWidgetPrivate::SelectionControl(unsigned int index, const mitk::DataNode* /*selection*/)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  dataSelectionWidget->SetHelpText("");
  this->EnableButtons();
}

mitk::Image::Pointer QmitkContourModelToImageWidgetPrivate::FillContourModelSetIntoImage(mitk::Image* image, mitk::ContourModelSet* contourSet, unsigned int timeStep)
{
  // Use mitk::ContourModelSetToImageFilter to fill the ContourModelSet into the image
  mitk::ContourModelSetToImageFilter::Pointer contourFiller = mitk::ContourModelSetToImageFilter::New();
  contourFiller->SetTimeStep(timeStep);
  contourFiller->SetImage(image);
  contourFiller->SetInput(contourSet);
  contourFiller->MakeOutputBinaryOn();
  contourFiller->Update();

  mitk::Image::Pointer result = contourFiller->GetOutput();
  if (result.IsNull())
  {
    MITK_ERROR<<"Could not write the selected contours into the image!";
  }

  result->DisconnectPipeline();
  return result;
}

void QmitkContourModelToImageWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  Q_D(QmitkContourModelToImageWidget);
  QmitkDataSelectionWidget* dataSelectionWidget = d->m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull() )
  {
    d->EnableButtons(false);
    dataSelectionWidget->SetHelpText(HelpText);
  }
  else
  {
    d->SelectionControl(index, selection);
  }
}

void QmitkContourModelToImageWidget::OnProcessingFinished()
{
  // Called when processing finished
  // Adding the result to the data storage

  Q_D(QmitkContourModelToImageWidget);

  // Adding the result to the data storage
  mitk::Image::Pointer result = d->m_Watcher.result();
  if (result.IsNotNull())
  {
    QmitkDataSelectionWidget* dataSelectionWidget = d->m_Controls.dataSelectionWidget;
    mitk::DataNode::Pointer imageNode = dataSelectionWidget->GetSelection(0);
    mitk::DataNode::Pointer contourNode = dataSelectionWidget->GetSelection(1);

    mitk::DataNode::Pointer filled = mitk::DataNode::New();
    std::stringstream stream;
    stream << imageNode->GetName();
    stream << "_";
    stream << contourNode->GetName();
    filled->SetName(stream.str());
    filled->SetData(result);

    dataSelectionWidget->GetDataStorage()->Add(filled, imageNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    MITK_ERROR<<"Error filling contours into an image!";
  }

  d->EnableButtons();
}

void QmitkContourModelToImageWidget::OnProcessPressed()
{
  Q_D(QmitkContourModelToImageWidget);

  QmitkDataSelectionWidget* dataSelectionWidget = d->m_Controls.dataSelectionWidget;

  mitk::DataNode::Pointer imageNode = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer contourNode = dataSelectionWidget->GetSelection(1);

  // Check if data nodes are valid
  if(imageNode.IsNull() || contourNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain valid data";
    QMessageBox::information( this, "Contour To Image",
                              "Selection does not contain valid data, please select a binary image and a contour(set)",
                              QMessageBox::Ok );
    d->m_Controls.btnProcess->setEnabled(false);
    return;
  }

  mitk::Image::Pointer image = static_cast<mitk::Image*>(imageNode->GetData());

  // Check if the image is valid
  if (image.IsNull())
  {
    MITK_ERROR<<"Error writing contours into image! Invalid image data selected!";
    return;
  }

  unsigned int timeStep = this->GetTimeNavigationController()->GetTime()->GetPos();

  // Check if the selected contours are valid
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
  d->EnableButtons(false);

  // Start the computation in a background thread
  QFuture< mitk::Image::Pointer > future = QtConcurrent::run(d, &QmitkContourModelToImageWidgetPrivate::FillContourModelSetIntoImage, image, contourSet, timeStep);
  d->m_Watcher.setFuture(future);
}

QmitkContourModelToImageWidget::QmitkContourModelToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent),
    d_ptr(new QmitkContourModelToImageWidgetPrivate())
{
  Q_D(QmitkContourModelToImageWidget);

  // Set up UI
  d->m_Controls.setupUi(this);
  d->m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImageAndSegmentationPredicate);
  d->m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ContourModelPredicate);
  d->m_Controls.dataSelectionWidget->SetHelpText(HelpText);
  d->EnableButtons(false);

  // Create connections
  connect (d->m_Controls.btnProcess, SIGNAL(pressed()), this, SLOT(OnProcessPressed()));
  connect(d->m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
          this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
  connect(&d->m_Watcher, SIGNAL(finished()), this, SLOT(OnProcessingFinished()));

  if( d->m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
      d->m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    OnSelectionChanged(0, d->m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkContourModelToImageWidget::~QmitkContourModelToImageWidget()
{
}
