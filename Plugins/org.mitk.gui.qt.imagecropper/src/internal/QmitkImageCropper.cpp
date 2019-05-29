/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkImageCropper.h"

#include <QAction>
#include <QCheckBox>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QSlider>
#include <QApplication>

#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkPlaneWidget.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <mitkBoundingShapeInteractor.h>
#include <mitkBoundingShapeCropper.h>
#include <mitkDisplayInteractor.h>
#include <mitkIDataStorageService.h>
#include <mitkImageCast.h> // Includes for image casting between ITK and MITK: added after using Plugin Generator
#include <mitkImageAccessByItk.h>
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
#include <mitkProperties.h>

#include <itkBoundingBox.h>
#include <itkCommand.h>

#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string QmitkImageCropper::VIEW_ID = "org.mitk.views.qmitkimagecropper";

QmitkImageCropper::QmitkImageCropper(QObject *)
  : m_ParentWidget(0),
  m_ImageNode(nullptr),
  m_CroppingObject(nullptr),
  m_CroppingObjectNode(nullptr),
  m_BoundingShapeInteractor(nullptr),
  m_CropOutsideValue(0),
  m_Advanced(0),
  m_Active(0),
  m_ScrollEnabled(true)
{
  CreateBoundingShapeInteractor(false);
}

QmitkImageCropper::~QmitkImageCropper()
{
  //delete pointer objects
  m_CroppingObjectNode = nullptr;
  m_CroppingObject = nullptr;

  //disable interactor
  if (m_BoundingShapeInteractor != nullptr)
  {
    m_BoundingShapeInteractor->SetDataNode(nullptr);
    m_BoundingShapeInteractor->EnableInteraction(false);
  }
}

void QmitkImageCropper::SetFocus()
{
  m_Controls.buttonCreateNewBoundingBox->setFocus();
}

void QmitkImageCropper::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  m_Controls.boundingShapeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.boundingShapeSelector->SetPredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::GeometryData>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  m_CroppingObjectNode = m_Controls.boundingShapeSelector->GetSelectedNode();

  connect(m_Controls.buttonCropping, SIGNAL(clicked()), this, SLOT(DoCropping()));
  connect(m_Controls.buttonMasking, SIGNAL(clicked()), this, SLOT(DoMasking()));
  connect(m_Controls.boundingShapeSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnDataSelectionChanged(const mitk::DataNode*)));
  connect(m_Controls.buttonCreateNewBoundingBox, SIGNAL(clicked()), this, SLOT(DoCreateNewBoundingObject()));
  connect(m_Controls.buttonAdvancedSettings, SIGNAL(clicked()), this, SLOT(OnAdvancedSettingsButtonToggled()));
  connect(m_Controls.spinBoxOutsidePixelValue, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));

  setDefaultGUI();
  m_Controls.labelWarningRotation->setVisible(false);

  m_Advanced = false;
  this->OnAdvancedSettingsButtonToggled();
  m_ParentWidget = parent;
}

void QmitkImageCropper::OnDataSelectionChanged(const mitk::DataNode*)
{
  m_Controls.boundingShapeSelector->setEnabled(true);
  m_CroppingObjectNode = m_Controls.boundingShapeSelector->GetSelectedNode();

  if (m_CroppingObjectNode.IsNotNull() && dynamic_cast<mitk::GeometryData*>(this->m_CroppingObjectNode->GetData()))
  {
    m_Controls.buttonAdvancedSettings->setEnabled(true);
    m_Controls.groupImageSettings->setEnabled(true);
    m_Controls.labelWarningBB->setVisible(false);
    m_CroppingObject = dynamic_cast<mitk::GeometryData*>(m_CroppingObjectNode->GetData());
    m_Advanced = true;

    m_BoundingShapeInteractor->EnableInteraction(true);
    m_BoundingShapeInteractor->SetDataNode(m_CroppingObjectNode);

    mitk::RenderingManager::GetInstance()->InitializeViews();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    setDefaultGUI();
    m_CroppingObject = nullptr;
    m_BoundingShapeInteractor->EnableInteraction(false);
    m_BoundingShapeInteractor->SetDataNode(nullptr);
    m_Advanced = false;
    this->OnAdvancedSettingsButtonToggled();
  }
}

void QmitkImageCropper::OnAdvancedSettingsButtonToggled()
{
  m_Controls.groupImageSettings->setVisible(m_Advanced);
  m_Advanced = !m_Advanced;
}

void QmitkImageCropper::CreateBoundingShapeInteractor(bool rotationEnabled)
{
  if (m_BoundingShapeInteractor.IsNull())
  {
    m_BoundingShapeInteractor = mitk::BoundingShapeInteractor::New();
    m_BoundingShapeInteractor->LoadStateMachine("BoundingShapeInteraction.xml", us::ModuleRegistry::GetModule("MitkBoundingShape"));
    m_BoundingShapeInteractor->SetEventConfig("BoundingShapeMouseConfig.xml", us::ModuleRegistry::GetModule("MitkBoundingShape"));
  }
  m_BoundingShapeInteractor->SetRotationEnabled(rotationEnabled);
}

mitk::Geometry3D::Pointer QmitkImageCropper::InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry)
{
  // convert a basegeometry into a Geometry3D (otherwise IO is not working properly)
  if (geometry == nullptr)
    mitkThrow() << "Geometry is not valid.";

  auto boundingGeometry = mitk::Geometry3D::New();
  boundingGeometry->SetBounds(geometry->GetBounds());
  boundingGeometry->SetImageGeometry(geometry->GetImageGeometry());
  boundingGeometry->SetOrigin(geometry->GetOrigin());
  boundingGeometry->SetSpacing(geometry->GetSpacing());
  boundingGeometry->SetIndexToWorldTransform(geometry->GetIndexToWorldTransform());
  boundingGeometry->Modified();
  return boundingGeometry;
}

QString QmitkImageCropper::AdaptBoundingObjectName(const QString& name) const
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

void QmitkImageCropper::DoCreateNewBoundingObject()
{
  if (!m_ImageNode.IsExpired())
  {
    auto imageNode = m_ImageNode.Lock();
    QString name = QString::fromStdString(imageNode->GetName() + " Bounding Shape");

    auto boundingShape = this->GetDataStorage()->GetNode(mitk::NodePredicateFunction::New([&name](const mitk::DataNode *node)
    {
      return 0 == node->GetName().compare(name.toStdString());
    }));

    if (nullptr != boundingShape)
      name = this->AdaptBoundingObjectName(name);

    m_Controls.buttonCropping->setEnabled(true);
    m_Controls.buttonMasking->setEnabled(true);
    m_Controls.boundingShapeSelector->setEnabled(true);
    m_Controls.groupImageSettings->setEnabled(true);

    // get current timestep to support 3d+t images
    auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
    int timeStep = renderWindowPart->GetTimeNavigationController()->GetTime()->GetPos();
    mitk::BaseGeometry::Pointer imageGeometry = static_cast<mitk::BaseGeometry*>(imageNode->GetData()->GetGeometry(timeStep));

    m_CroppingObject = mitk::GeometryData::New();
    m_CroppingObject->SetGeometry(static_cast<mitk::Geometry3D*>(this->InitializeWithImageGeometry(imageGeometry)));
    m_CroppingObjectNode = mitk::DataNode::New();
    m_CroppingObjectNode->SetData(m_CroppingObject);
    m_CroppingObjectNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
    m_CroppingObjectNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
    m_CroppingObjectNode->SetProperty("opacity", mitk::FloatProperty::New(0.6));
    m_CroppingObjectNode->SetProperty("layer", mitk::IntProperty::New(99));
    m_CroppingObjectNode->AddProperty("handle size factor", mitk::DoubleProperty::New(1.0 / 40.0));
    m_CroppingObjectNode->SetBoolProperty("pickable", true);

    if (!this->GetDataStorage()->Exists(m_CroppingObjectNode))
    {
      GetDataStorage()->Add(m_CroppingObjectNode, imageNode);
      m_Controls.boundingShapeSelector->SetSelectedNode(m_CroppingObjectNode);
      m_CroppingObjectNode->SetVisibility(true);
      m_BoundingShapeInteractor->EnableInteraction(true);
      m_BoundingShapeInteractor->SetDataNode(this->m_CroppingObjectNode);
      this->OnDataSelectionChanged(m_CroppingObjectNode);
    }
  }
  // Adjust coordinate system by doing a reinit on
  auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
  tempDataStorage->InsertElement(0, m_CroppingObjectNode);

  //// initialize the views to the bounding geometry
  //mitk::TimeGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(tempDataStorage);
  //mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
void QmitkImageCropper::setDefaultGUI()
{
  m_Controls.labelWarningImage->setStyleSheet(" QLabel { color: rgb(255, 0, 0) }");
  m_Controls.labelWarningImage->setText(QString::fromStdString("Select an image."));
  m_Controls.labelWarningImage->setVisible(true);
  m_Controls.labelWarningBB->setStyleSheet(" QLabel { color: rgb(255, 0, 0) }");
  m_Controls.labelWarningBB->setText(QString::fromStdString("Create a bounding shape below."));
  m_Controls.labelWarningBB->setVisible(true);
  m_Controls.buttonCreateNewBoundingBox->setEnabled(false);
  m_Controls.labelWarningRotation->setVisible(false);
  m_Controls.buttonCropping->setEnabled(false);
  m_Controls.buttonMasking->setEnabled(false);
  m_Controls.boundingShapeSelector->setEnabled(false);
  m_Controls.buttonAdvancedSettings->setEnabled(false);
  m_Controls.groupImageSettings->setEnabled(false);
  m_Controls.checkOverwriteImage->setChecked(false);
  m_Controls.checkBoxCropTimeStepOnly->setChecked(false);
}

void QmitkImageCropper::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
  bool rotationEnabled = false;
  if (nodes.empty())
  {
    setDefaultGUI();
    return;
  }
  m_ParentWidget->setEnabled(true);

  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()))
    {
      m_ImageNode = nodes[0];
      m_Controls.groupBoundingObject->setEnabled(true);
      m_Controls.labelWarningImage->setStyleSheet("");
      m_Controls.labelWarningImage->setText(QString::fromStdString("Selected image: " + nodes[0]->GetName()));
      m_Controls.buttonCreateNewBoundingBox->setEnabled(true);

      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(nodes[0]->GetData());
      if (image != nullptr)
      {
        if (image->GetDimension() < 3) {
          QMessageBox::warning(nullptr,
            tr("Invalid image selected"),
            tr("ImageCropper only works with 3 or more dimensions."),
            QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
          setDefaultGUI();
          return;
        }

        vtkSmartPointer<vtkMatrix4x4> imageMat = image->GetGeometry()->GetVtkMatrix();
        // check whether the image geometry is rotated, if so, no pixel aligned cropping or masking can be performed
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
        m_CroppingObjectNode = m_Controls.boundingShapeSelector->GetSelectedNode();
        if (m_CroppingObjectNode != nullptr)
        {
          this->OnDataSelectionChanged(m_CroppingObjectNode);
          m_BoundingShapeInteractor->EnableInteraction(true);
          m_BoundingShapeInteractor->SetDataNode(this->m_CroppingObjectNode);
          m_Controls.boundingShapeSelector->setEnabled(true);
        }

        if (image->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
        {
          // Might be changed with the upcoming new image statistics plugin
          //(recomputation might be very expensive for large images ;) )
          auto statistics = image->GetStatistics();
          auto minPixelValue = statistics->GetScalarValueMin();
          auto maxPixelValue = statistics->GetScalarValueMax();

          if (minPixelValue < std::numeric_limits<int>::min()) {
            minPixelValue = std::numeric_limits<int>::min();
          }
          if (maxPixelValue > std::numeric_limits<int>::max()) {
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
          m_Controls.labelWarningImage->setStyleSheet(" QLabel { color: rgb(255, 0, 0) }");
          m_Controls.labelWarningImage->setText(QString::fromStdString("Select an image."));
          m_ParentWidget->setEnabled(false);
        }
        if (m_CroppingObjectNode != nullptr)
        {
          m_Controls.buttonCropping->setEnabled(true);
          m_Controls.buttonMasking->setEnabled(true);
          m_Controls.boundingShapeSelector->setEnabled(true);
          m_Controls.labelWarningBB->setVisible(false);
        }
        else
        {
          m_Controls.buttonCropping->setEnabled(false);
          m_Controls.buttonMasking->setEnabled(false);
          m_Controls.boundingShapeSelector->setEnabled(false);
          m_Controls.labelWarningBB->setVisible(true);
        }
        return;
      }
      //  iterate all selected objects, adjust warning visibility
      setDefaultGUI();
      m_ParentWidget->setEnabled(true);
      m_Controls.labelWarningRotation->setVisible(false);
    }
  }
}

void QmitkImageCropper::OnComboBoxSelectionChanged(const mitk::DataNode* node)
{
  mitk::DataNode* selectedNode = const_cast<mitk::DataNode*>(node);
  if (selectedNode != nullptr)
  {
    if (!m_ImageNode.IsExpired())
      selectedNode->SetDataInteractor(m_ImageNode.Lock()->GetDataInteractor());
    // m_ImageNode->GetDataInteractor()->SetDataNode(selectedNode);
    m_ImageNode = selectedNode;
  }
}

void QmitkImageCropper::OnSliderValueChanged(int slidervalue)
{
  m_CropOutsideValue = slidervalue;
}

void QmitkImageCropper::DoMasking()
{
  this->ProcessImage(true);
}

void QmitkImageCropper::DoCropping()
{
  this->ProcessImage(false);
}

void QmitkImageCropper::ProcessImage(bool mask)
{
  // cropping only possible if valid bounding shape as well as a valid image are loaded
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

  auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
  int timeStep = renderWindowPart->GetTimeNavigationController()->GetTime()->GetPos();

  if (nodes.empty()) return;

  mitk::DataNode* node = nodes[0];

  if (node == nullptr)
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select an image before starting image processing.");
    return;
  }
  if (m_CroppingObject == nullptr)
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select a cropping object before starting image processing.");
    return;
  }

  mitk::BaseData* data = node->GetData(); //get data from node
  if (data != nullptr)
  {
    QString imageName;
    if (mask)
    {
      imageName = QString::fromStdString(node->GetName() + "_" + m_CroppingObjectNode->GetName() + "_masked");
    }
    else
    {
      imageName = QString::fromStdString(node->GetName() + "_" + m_CroppingObjectNode->GetName() + "_cropped");
    }

    if (m_Controls.checkBoxCropTimeStepOnly->isChecked())
    {
      imageName = imageName + "_T" + QString::number(timeStep);
    }

    // image and bounding shape ok, set as input
    auto croppedImageNode = mitk::DataNode::New();
    auto cutter = mitk::BoundingShapeCropper::New();
    cutter->SetGeometry(m_CroppingObject);

    // adjustable in advanced settings
    cutter->SetUseWholeInputRegion(mask); //either mask (mask=true) or crop (mask=false)
    cutter->SetOutsideValue(m_CropOutsideValue);
    cutter->SetUseCropTimeStepOnly(m_Controls.checkBoxCropTimeStepOnly->isChecked());
    cutter->SetCurrentTimeStep(timeStep);

    // TODO: Add support for MultiLayer (right now only Mulitlabel support)
    mitk::LabelSetImage* labelsetImageInput = dynamic_cast<mitk::LabelSetImage*>(data);
    if (labelsetImageInput != nullptr)
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
        QMessageBox::warning(nullptr,
          tr("Cropping not possible!"),
          tr(message.c_str()),
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
          this->GetDataStorage()->Add(croppedImageNode, m_ImageNode.Lock());
        }
      }
      else // original image will be overwritten by the result image and the bounding box of the result is adjusted
      {
        node->SetData(labelSetImage);
        node->Modified();
        // Adjust coordinate system by doing a reinit on
        auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
        tempDataStorage->InsertElement(0, node);

        // initialize the views to the bounding geometry
        auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(tempDataStorage);
        mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
    else
    {
      mitk::Image::Pointer imageInput = dynamic_cast<mitk::Image*>(data);
      if (imageInput != nullptr)
      {
        cutter->SetInput(imageInput);
        // do the actual cutting
        try
        {
          cutter->Update();
        }
        catch (const itk::ExceptionObject& e)
        {
          std::string message = std::string("The Cropping filter could not process because of: \n ") + e.GetDescription();

          QMessageBox::warning(nullptr,
            tr("Cropping not possible!"),
            tr(message.c_str()),
            QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
          return;
        }

        //add cropping result to the current data storage as child node to the image node
        if (!m_Controls.checkOverwriteImage->isChecked())
        {
          croppedImageNode->SetData(cutter->GetOutput());
          croppedImageNode->SetProperty("name", mitk::StringProperty::New(imageName.toStdString()));
          croppedImageNode->SetProperty("color", mitk::ColorProperty::New(1.0, 1.0, 1.0));
          croppedImageNode->SetProperty("layer", mitk::IntProperty::New(99)); // arbitrary, copied from segmentation functionality
          if (!this->GetDataStorage()->Exists(croppedImageNode))
          {
            this->GetDataStorage()->Add(croppedImageNode, m_ImageNode.Lock());
          }
        }
        else // original image will be overwritten by the result image and the bounding box of the result is adjusted
        {
          node->SetData(cutter->GetOutput());
          node->Modified();
          // Adjust coordinate system by doing a reinit on
          auto tempDataStorage = mitk::DataStorage::SetOfObjects::New();
          tempDataStorage->InsertElement(0, node);

          // initialize the views to the bounding geometry
          auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(tempDataStorage);
          mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }
    }
  }
  else
  {
    QMessageBox::information(nullptr, "Warning", "Please load and select an image before starting image processing.");
  }
}
