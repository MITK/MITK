/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageCropperView.h"

#include <mitkBoundingShapeCropper.h>
#include <mitkDisplayInteractor.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkInteractionConst.h>
#include <mitkITKImageImport.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateFunction.h>
#include <mitkRenderingManager.h>

#include <usModuleRegistry.h>

#include <QMessageBox>

const std::string QmitkImageCropperView::VIEW_ID = "org.mitk.views.qmitkimagecropper";

QmitkImageCropperView::QmitkImageCropperView(QObject *)
  : m_ParentWidget(nullptr)
  , m_BoundingShapeInteractor(nullptr)
  , m_CropOutsideValue(0)
{
  CreateBoundingShapeInteractor(false);
}

QmitkImageCropperView::~QmitkImageCropperView()
{
  //disable interactor
  if (m_BoundingShapeInteractor != nullptr)
  {
    m_BoundingShapeInteractor->SetDataNode(nullptr);
    m_BoundingShapeInteractor->EnableInteraction(false);
  }
}

void QmitkImageCropperView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  m_Controls.imageSelectionWidget->SetDataStorage(GetDataStorage());
  m_Controls.imageSelectionWidget->SetNodePredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_Controls.imageSelectionWidget->SetSelectionIsOptional(true);
  m_Controls.imageSelectionWidget->SetAutoSelectNewNodes(true);
  m_Controls.imageSelectionWidget->SetEmptyInfo(QString("Please select an image node"));
  m_Controls.imageSelectionWidget->SetPopUpTitel(QString("Select image node"));

  connect(m_Controls.imageSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageCropperView::OnImageSelectionChanged);

  m_Controls.boundingBoxSelectionWidget->SetDataStorage(GetDataStorage());
  m_Controls.boundingBoxSelectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::GeometryData>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  m_Controls.boundingBoxSelectionWidget->SetSelectionIsOptional(true);
  m_Controls.boundingBoxSelectionWidget->SetAutoSelectNewNodes(true);
  m_Controls.boundingBoxSelectionWidget->SetEmptyInfo(QString("Please select a bounding box"));
  m_Controls.boundingBoxSelectionWidget->SetPopUpTitel(QString("Select bounding box node"));

  connect(m_Controls.boundingBoxSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageCropperView::OnBoundingBoxSelectionChanged);

  connect(m_Controls.buttonCreateNewBoundingBox, SIGNAL(clicked()), this, SLOT(OnCreateNewBoundingBox()));
  connect(m_Controls.buttonCropping, SIGNAL(clicked()), this, SLOT(OnCropping()));
  connect(m_Controls.buttonMasking, SIGNAL(clicked()), this, SLOT(OnMasking()));
  auto lambda = [this]()
  {
    m_Controls.groupImageSettings->setVisible(!m_Controls.groupImageSettings->isVisible());
  };

  connect(m_Controls.buttonAdvancedSettings, &ctkExpandButton::clicked, this, lambda);

  connect(m_Controls.spinBoxOutsidePixelValue, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));

  SetDefaultGUI();

  m_ParentWidget = parent;

  this->OnImageSelectionChanged(m_Controls.imageSelectionWidget->GetSelectedNodes());
  this->OnBoundingBoxSelectionChanged(m_Controls.boundingBoxSelectionWidget->GetSelectedNodes());
}

void QmitkImageCropperView::OnImageSelectionChanged(QList<mitk::DataNode::Pointer>)
{
  bool rotationEnabled = false;
  auto imageNode = m_Controls.imageSelectionWidget->GetSelectedNode();
  if (imageNode.IsNull())
  {
    SetDefaultGUI();
    return;
  }

  auto image = dynamic_cast<mitk::Image*>(imageNode->GetData());
  if (nullptr != image)
  {
    if (image->GetDimension() < 3)
    {
      QMessageBox::warning(nullptr,
        tr("Invalid image selected"),
        tr("ImageCropper only works with 3 or more dimensions."),
        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      SetDefaultGUI();
      return;
    }

    m_ParentWidget->setEnabled(true);
    m_Controls.buttonCreateNewBoundingBox->setEnabled(true);

    vtkSmartPointer<vtkMatrix4x4> imageMat = image->GetGeometry()->GetVtkMatrix();
    // check whether the image geometry is rotated; if so, no pixel aligned cropping or masking can be performed
    if ((imageMat->GetElement(1, 0) == 0.0) && (imageMat->GetElement(0, 1) == 0.0) &&
      (imageMat->GetElement(1, 2) == 0.0) && (imageMat->GetElement(2, 1) == 0.0) &&
      (imageMat->GetElement(2, 0) == 0.0) && (imageMat->GetElement(0, 2) == 0.0))
    {
      rotationEnabled = false;
      m_Controls.labelWarningRotation->setVisible(false);
    }
    else
    {
      rotationEnabled = true;
      m_Controls.labelWarningRotation->setStyleSheet(" QLabel { color: rgb(255, 0, 0) }");
      m_Controls.labelWarningRotation->setVisible(true);
    }

    this->CreateBoundingShapeInteractor(rotationEnabled);

    if (itk::ImageIOBase::SCALAR == image->GetPixelType().GetPixelType())
    {
      // Might be changed with the upcoming new image statistics plugin
      //(recomputation might be very expensive for large images ;) )
      auto statistics = image->GetStatistics();
      auto minPixelValue = statistics->GetScalarValueMin();
      auto maxPixelValue = statistics->GetScalarValueMax();

      if (minPixelValue < std::numeric_limits<int>::min())
      {
        minPixelValue = std::numeric_limits<int>::min();
      }
      if (maxPixelValue > std::numeric_limits<int>::max())
      {
        maxPixelValue = std::numeric_limits<int>::max();
      }

      m_Controls.spinBoxOutsidePixelValue->setEnabled(true);
      m_Controls.spinBoxOutsidePixelValue->setMaximum(static_cast<int>(maxPixelValue));
      m_Controls.spinBoxOutsidePixelValue->setMinimum(static_cast<int>(minPixelValue));
      m_Controls.spinBoxOutsidePixelValue->setValue(static_cast<int>(minPixelValue));
    }
    else
    {
      m_Controls.spinBoxOutsidePixelValue->setEnabled(false);
    }

    unsigned int dim = image->GetDimension();
    if (dim < 2 || dim > 4)
    {
      m_ParentWidget->setEnabled(false);
    }

    if (m_Controls.boundingBoxSelectionWidget->GetSelectedNode().IsNotNull())
    {
      m_Controls.buttonCropping->setEnabled(true);
      m_Controls.buttonMasking->setEnabled(true);
      m_Controls.buttonAdvancedSettings->setEnabled(true);
      m_Controls.groupImageSettings->setEnabled(true);
    }
  }
}

void QmitkImageCropperView::OnBoundingBoxSelectionChanged(QList<mitk::DataNode::Pointer>)
{
  auto boundingBoxNode = m_Controls.boundingBoxSelectionWidget->GetSelectedNode();
  if (boundingBoxNode.IsNull())
  {
    SetDefaultGUI();

    m_BoundingShapeInteractor->EnableInteraction(false);
    m_BoundingShapeInteractor->SetDataNode(nullptr);

    if (m_Controls.imageSelectionWidget->GetSelectedNode().IsNotNull())
    {
      m_Controls.buttonCreateNewBoundingBox->setEnabled(true);
    }

    return;
  }

  auto boundingBox = dynamic_cast<mitk::GeometryData*>(boundingBoxNode->GetData());
  if (nullptr != boundingBox)
  {
    // node newly selected
    boundingBoxNode->SetVisibility(true);

    m_BoundingShapeInteractor->EnableInteraction(true);
    m_BoundingShapeInteractor->SetDataNode(boundingBoxNode);

    mitk::RenderingManager::GetInstance()->InitializeViews();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    if (m_Controls.imageSelectionWidget->GetSelectedNode().IsNotNull())
    {
      m_Controls.buttonCropping->setEnabled(true);
      m_Controls.buttonMasking->setEnabled(true);
      m_Controls.buttonAdvancedSettings->setEnabled(true);
      m_Controls.groupImageSettings->setEnabled(true);
    }
  }
}

void QmitkImageCropperView::OnCreateNewBoundingBox()
{
  auto imageNode = m_Controls.imageSelectionWidget->GetSelectedNode();
  if (imageNode.IsNull())
  {
    return;
  }
  if (nullptr == imageNode->GetData())
  {
    return;
  }

  QString name = QString::fromStdString(imageNode->GetName() + " Bounding Shape");

  auto boundingShape = this->GetDataStorage()->GetNode(mitk::NodePredicateFunction::New([&name](const mitk::DataNode *node)
  {
    return 0 == node->GetName().compare(name.toStdString());
  }));

  if (nullptr != boundingShape)
  {
    name = this->AdaptBoundingObjectName(name);
  }

  // get current timestep to support 3d+t images
  auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
  const auto timePoint = renderWindowPart->GetSelectedTimePoint();
  const auto imageGeometry = imageNode->GetData()->GetTimeGeometry()->GetGeometryForTimePoint(timePoint);

  auto boundingBox = mitk::GeometryData::New();
  boundingBox->SetGeometry(static_cast<mitk::Geometry3D*>(this->InitializeWithImageGeometry(imageGeometry)));
  auto boundingBoxNode = mitk::DataNode::New();
  boundingBoxNode->SetData(boundingBox);
  boundingBoxNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
  boundingBoxNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
  boundingBoxNode->SetProperty("opacity", mitk::FloatProperty::New(0.6));
  boundingBoxNode->SetProperty("layer", mitk::IntProperty::New(99));
  boundingBoxNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
  boundingBoxNode->SetBoolProperty("pickable", true);

  if (!this->GetDataStorage()->Exists(boundingBoxNode))
  {
    GetDataStorage()->Add(boundingBoxNode, imageNode);
  }

  m_Controls.boundingBoxSelectionWidget->SetCurrentSelectedNode(boundingBoxNode);
}

void QmitkImageCropperView::OnCropping()
{
  this->ProcessImage(false);
}

void QmitkImageCropperView::OnMasking()
{
  this->ProcessImage(true);
}

void QmitkImageCropperView::OnSliderValueChanged(int slidervalue)
{
  m_CropOutsideValue = slidervalue;
}

void QmitkImageCropperView::CreateBoundingShapeInteractor(bool rotationEnabled)
{
  if (m_BoundingShapeInteractor.IsNull())
  {
    m_BoundingShapeInteractor = mitk::BoundingShapeInteractor::New();
    m_BoundingShapeInteractor->LoadStateMachine("BoundingShapeInteraction.xml", us::ModuleRegistry::GetModule("MitkBoundingShape"));
    m_BoundingShapeInteractor->SetEventConfig("BoundingShapeMouseConfig.xml", us::ModuleRegistry::GetModule("MitkBoundingShape"));
  }
  m_BoundingShapeInteractor->SetRotationEnabled(rotationEnabled);
}

mitk::Geometry3D::Pointer QmitkImageCropperView::InitializeWithImageGeometry(const mitk::BaseGeometry* geometry) const
{
  // convert a BaseGeometry into a Geometry3D (otherwise IO is not working properly)
  if (geometry == nullptr)
    mitkThrow() << "Geometry is not valid.";

  auto boundingGeometry = mitk::Geometry3D::New();
  boundingGeometry->SetBounds(geometry->GetBounds());
  boundingGeometry->SetImageGeometry(geometry->GetImageGeometry());
  boundingGeometry->SetOrigin(geometry->GetOrigin());
  boundingGeometry->SetSpacing(geometry->GetSpacing());
  boundingGeometry->SetIndexToWorldTransform(geometry->GetIndexToWorldTransform()->Clone());
  boundingGeometry->Modified();
  return boundingGeometry;
}

void QmitkImageCropperView::ProcessImage(bool mask)
{
  auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
  const auto timePoint = renderWindowPart->GetSelectedTimePoint();

  auto imageNode = m_Controls.imageSelectionWidget->GetSelectedNode();
  if (imageNode.IsNull())
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select an image before starting image processing.");
    return;
  }

  auto boundingBoxNode = m_Controls.boundingBoxSelectionWidget->GetSelectedNode();
  if (boundingBoxNode.IsNull())
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select a cropping object before starting image processing.");
    return;
  }

  if (!imageNode->GetData()->GetTimeGeometry()->IsValidTimePoint(timePoint))
  {
    QMessageBox::information(nullptr, "Warning", "Please select a time point that is within the time bounds of the selected image.");
    return;
  }
  const auto timeStep = imageNode->GetData()->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  auto image = dynamic_cast<mitk::Image*>(imageNode->GetData());
  auto boundingBox = dynamic_cast<mitk::GeometryData*>(boundingBoxNode->GetData());
  if (nullptr != image && nullptr != boundingBox)
  {
    QString imageName;
    if (mask)
    {
      imageName = QString::fromStdString(imageNode->GetName() + "_" + boundingBoxNode->GetName() + "_masked");
    }
    else
    {
      imageName = QString::fromStdString(imageNode->GetName() + "_" + boundingBoxNode->GetName() + "_cropped");
    }

    if (m_Controls.checkBoxCropTimeStepOnly->isChecked())
    {
      imageName = imageName + "_T" + QString::number(timeStep);
    }

    // image and bounding shape ok, set as input
    auto croppedImageNode = mitk::DataNode::New();
    auto cutter = mitk::BoundingShapeCropper::New();
    cutter->SetGeometry(boundingBox);

    // adjustable in advanced settings
    cutter->SetUseWholeInputRegion(mask); //either mask (mask=true) or crop (mask=false)
    cutter->SetOutsideValue(m_CropOutsideValue);
    cutter->SetUseCropTimeStepOnly(m_Controls.checkBoxCropTimeStepOnly->isChecked());
    cutter->SetCurrentTimeStep(timeStep);

    // TODO: Add support for MultiLayer (right now only Mulitlabel support)
    auto labelsetImageInput = dynamic_cast<mitk::LabelSetImage*>(image);
    if (nullptr != labelsetImageInput)
    {
      cutter->SetInput(labelsetImageInput);
      // do the actual cutting
      try
      {
        cutter->Update();
      }
      catch (const itk::ExceptionObject& e)
      {
        std::string message = std::string("The Cropping filter could not process because of: \n ") + e.GetDescription();
        QMessageBox::warning(nullptr, tr("Cropping not possible!"), tr(message.c_str()),
          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
      }

      auto labelSetImage = mitk::LabelSetImage::New();
      labelSetImage->InitializeByLabeledImage(cutter->GetOutput());

      for (unsigned int i = 0; i < labelsetImageInput->GetNumberOfLayers(); i++)
      {
        labelSetImage->AddLabelSetToLayer(i, labelsetImageInput->GetLabelSet(i));
      }

      croppedImageNode->SetData(labelSetImage);
      croppedImageNode->SetProperty("name", mitk::StringProperty::New(imageName.toStdString()));

      //add cropping result to the current data storage as child node to the image node
      if (!m_Controls.checkOverwriteImage->isChecked())
      {
        if (!this->GetDataStorage()->Exists(croppedImageNode))
        {
          this->GetDataStorage()->Add(croppedImageNode, imageNode);
        }
      }
      else // original image will be overwritten by the result image and the bounding box of the result is adjusted
      {
        imageNode->SetData(labelSetImage);
        imageNode->Modified();
        // Adjust coordinate system by doing a reinit on
        auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
        tempDataStorage->InsertElement(0, imageNode);

        // initialize the views to the bounding geometry
        auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(tempDataStorage);
        mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
    else
    {
      cutter->SetInput(image);
      // do the actual cutting
      try
      {
        cutter->Update();
      }
      catch (const itk::ExceptionObject& e)
      {
        std::string message = std::string("The Cropping filter could not process because of: \n ") + e.GetDescription();
        QMessageBox::warning(nullptr, tr("Cropping not possible!"), tr(message.c_str()),
          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        return;
      }

      //add cropping result to the current data storage as child node to the image node
      if (!m_Controls.checkOverwriteImage->isChecked())
      {
        croppedImageNode->SetData(cutter->GetOutput());
        croppedImageNode->SetProperty("name", mitk::StringProperty::New(imageName.toStdString()));
        croppedImageNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));

        mitk::LevelWindow levelWindow;
        imageNode->GetLevelWindow(levelWindow);
        croppedImageNode->SetLevelWindow(levelWindow);

        if (!this->GetDataStorage()->Exists(croppedImageNode))
        {
          this->GetDataStorage()->Add(croppedImageNode, imageNode);
          imageNode->SetVisibility(mask); // Give the user a visual clue that something happened when image was cropped
        }
      }
      else // original image will be overwritten by the result image and the bounding box of the result is adjusted
      {
        mitk::LevelWindow levelWindow;
        imageNode->GetLevelWindow(levelWindow);
        imageNode->SetData(cutter->GetOutput());
        imageNode->SetLevelWindow(levelWindow);
        // Adjust coordinate system by doing a reinit on
        auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
        tempDataStorage->InsertElement(0, imageNode);

        // initialize the views to the bounding geometry
        auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(tempDataStorage);
        mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
  else
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select an image before starting image processing.");
  }
}

void QmitkImageCropperView::SetDefaultGUI()
{
  m_Controls.labelWarningRotation->setVisible(false);
  m_Controls.buttonCreateNewBoundingBox->setEnabled(false);
  m_Controls.buttonCropping->setEnabled(false);
  m_Controls.buttonMasking->setEnabled(false);
  m_Controls.buttonAdvancedSettings->setEnabled(false);
  m_Controls.groupImageSettings->setEnabled(false);
  m_Controls.groupImageSettings->setVisible(false);
  m_Controls.checkOverwriteImage->setChecked(false);
  m_Controls.checkBoxCropTimeStepOnly->setChecked(false);
}

QString QmitkImageCropperView::AdaptBoundingObjectName(const QString& name) const
{
  unsigned int counter = 2;
  QString newName = QString("%1 %2").arg(name).arg(counter);

  while (nullptr != this->GetDataStorage()->GetNode(mitk::NodePredicateFunction::New([&newName](const mitk::DataNode *node)
  {
    return 0 == node->GetName().compare(newName.toStdString());
  })))
  {
    newName = QString("%1 %2").arg(name).arg(++counter);
  }

  return newName;
}
