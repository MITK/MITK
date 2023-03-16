/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkContourModelToImageWidget.h"
#include <ui_QmitkContourModelToImageWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkImage.h>

#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkLabelSetImage.h>
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
  mitk::LabelSetImage::Pointer FillContourModelSetIntoImage(mitk::Image *image, mitk::ContourModelSet *contourSet, mitk::TimePointType timePoint);

  Ui::QmitkContourModelToImageWidgetControls m_Controls;
  QFutureWatcher<mitk::LabelSetImage::Pointer> m_Watcher;
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

mitk::LabelSetImage::Pointer QmitkContourModelToImageWidgetPrivate::FillContourModelSetIntoImage(mitk::Image* image, mitk::ContourModelSet* contourSet, mitk::TimePointType timePoint)
{
  // Use mitk::ContourModelSetToImageFilter to fill the ContourModelSet into the image
  mitk::ContourModelSetToImageFilter::Pointer contourFiller = mitk::ContourModelSetToImageFilter::New();
  auto timeStep = image->GetTimeGeometry()->TimePointToTimeStep(timePoint);
  contourFiller->SetTimeStep(timeStep);
  contourFiller->SetImage(image);
  contourFiller->SetInput(contourSet);
  contourFiller->MakeOutputBinaryOn();

  try
  {
    contourFiller->Update();
  }
  catch (const std::exception & e)
  {
    MITK_ERROR << "Error while converting contour model. "<< e.what();
  }
  catch (...)
  {
    MITK_ERROR << "Unknown error while converting contour model.";
  }

  if (nullptr == contourFiller->GetOutput())
  {
    MITK_ERROR<<"Could not write the selected contours into the image!";
  }

  auto result = mitk::LabelSetImage::New();
  result->InitializeByLabeledImage(contourFiller->GetOutput());

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
  auto result = d->m_Watcher.result();
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

    auto dataStorage = dataSelectionWidget->GetDataStorage();
    if (dataStorage.IsNull())
    {
      std::string exception = "Cannot add result to the data storage. Data storage invalid.";
      MITK_ERROR << "Error filling contours into an image: " << exception;
      QMessageBox::information(nullptr, "Error filling contours into an image", QString::fromStdString(exception));
    }

    dataStorage->Add(filled, imageNode);
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

  const auto timePoint = this->GetTimeNavigationController()->GetSelectedTimePoint();
  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    MITK_ERROR << "Error writing contours into image! Currently selected time point is not supported by selected image data.";
    return;
  }

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
  QFuture< mitk::LabelSetImage::Pointer > future = QtConcurrent::run(d, &QmitkContourModelToImageWidgetPrivate::FillContourModelSetIntoImage, image, contourSet, timePoint);
  d->m_Watcher.setFuture(future);
}

QmitkContourModelToImageWidget::QmitkContourModelToImageWidget(mitk::DataStorage* dataStorage,
                                                               mitk::SliceNavigationController* timeNavigationController,
                                                               QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent),
    d_ptr(new QmitkContourModelToImageWidgetPrivate())
{
  Q_D(QmitkContourModelToImageWidget);

  // Set up UI
  d->m_Controls.setupUi(this);
  d->m_Controls.dataSelectionWidget->SetDataStorage(dataStorage);
  d->m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ImageAndSegmentationPredicate);
  d->m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ContourModelPredicate);
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
