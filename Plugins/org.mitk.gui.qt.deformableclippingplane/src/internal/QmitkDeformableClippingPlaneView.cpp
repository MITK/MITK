/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDeformableClippingPlaneView.h"

#include <usModuleRegistry.h>

// mitk core
#include <mitkImageToSurfaceFilter.h>
#include <mitkInteractionConst.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkRenderingModeProperty.h>
#include <mitkRotationOperation.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>

#include <mitkHeightFieldSurfaceClipImageFilter.h>
#include <mitkClippingPlaneInteractor3D.h>
#include <mitkLabeledImageLookupTable.h>
#include <mitkLabeledImageVolumeCalculator.h>
#include <mitkSurfaceDeformationDataInteractor3D.h>

#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>

#include <mitkILinkedRenderWindowPart.h>

#include <array>

const std::string QmitkDeformableClippingPlaneView::VIEW_ID = "org.mitk.views.deformableclippingplane";

QmitkDeformableClippingPlaneView::QmitkDeformableClippingPlaneView()
  : QmitkAbstractView()
  , m_Controls(new Ui::QmitkDeformableClippingPlaneViewControls)
  , m_WorkingNode(nullptr)
{
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isNotHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));

  m_IsImagePredicate = mitk::NodePredicateAnd::New(isImage, isNotHelperObject);

  m_IsClippingPlanePredicate = mitk::NodePredicateProperty::New("clippingPlane", mitk::BoolProperty::New(true));
}

QmitkDeformableClippingPlaneView::~QmitkDeformableClippingPlaneView()
{
  if (m_WorkingNode.IsNotNull())
    m_WorkingNode->SetDataInteractor(nullptr);
}

void QmitkDeformableClippingPlaneView::SetFocus()
{
  m_Controls->createNewPlanePushButton->setFocus();
}

void QmitkDeformableClippingPlaneView::CreateQtPartControl(QWidget *parent)
{
  m_Controls->setupUi(parent);

  m_Controls->imageSelectionWidget->SetDataStorage(GetDataStorage());
  m_Controls->imageSelectionWidget->SetNodePredicate(m_IsImagePredicate);
  m_Controls->imageSelectionWidget->SetSelectionIsOptional(false);
  m_Controls->imageSelectionWidget->SetInvalidInfo("Select an image or segmentation.");
  m_Controls->imageSelectionWidget->SetPopUpTitel("Select an image or segmentation.");

  m_Controls->clippingPlaneSelector->SetDataStorage(this->GetDataStorage());
  m_Controls->clippingPlaneSelector->SetPredicate(m_IsClippingPlanePredicate);

  m_Controls->volumeGroupBox->setEnabled(false);
  m_Controls->interactionSelectionBox->setEnabled(false);
  m_Controls->planesWarningLabel->hide();

  this->CreateConnections();

  m_Controls->imageSelectionWidget->SetAutoSelectNewNodes(true);
}

void QmitkDeformableClippingPlaneView::OnCurrentSelectionChanged(const QList<mitk::DataNode::Pointer>& /*nodes*/)
{
  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::OnComboBoxSelectionChanged(const mitk::DataNode* node)
{
  this->DeactivateInteractionButtons();

  auto selectedNode = const_cast<mitk::DataNode*>(node);
  if(nullptr != selectedNode)
  {
    if(m_WorkingNode.IsNotNull())
      selectedNode->SetDataInteractor(m_WorkingNode->GetDataInteractor());

    m_WorkingNode = selectedNode;
  }

  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::OnCreateNewClippingPlane()
{
  this->DeactivateInteractionButtons();

  auto plane = mitk::Surface::New();
  auto planeSource = vtkSmartPointer<vtkPlaneSource>::New();

  planeSource->SetOrigin(-32.0, -32.0, 0.0);
  planeSource->SetPoint1(32.0, -32.0, 0.0);
  planeSource->SetPoint2(-32.0, 32.0, 0.0);
  planeSource->SetResolution(128, 128);
  planeSource->Update();

  plane->SetVtkPolyData(planeSource->GetOutput());

  mitk::ScalarType imageDiagonal = 200.0;

  auto selectedNode = m_Controls->imageSelectionWidget->GetSelectedNode();
  if (selectedNode.IsNotNull())
  {
    auto selectedImage = dynamic_cast<mitk::Image*>(selectedNode->GetData());
    if (nullptr != selectedImage)
    {
      // check if user wants a surface model
      if (m_Controls->surfaceModelCheckBox->isChecked())
      {
        //Check if there is a surface node from the image. If not, create one
        bool createSurfaceFromImage = true;
        auto isSurface = mitk::NodePredicateDataType::New("Surface");
        auto childNodes = GetDataStorage()->GetDerivations(selectedNode, isSurface, true);

        for (mitk::DataStorage::SetOfObjects::ConstIterator it = childNodes->Begin();
          it != childNodes->End(); it++)
        {
          if (it.Value().IsNotNull() && it->Value()->GetName() == selectedNode->GetName())
          {
            createSurfaceFromImage = false;
            it.Value()->SetVisibility(true);
          }
        }

        if (createSurfaceFromImage)
        {
          //Lsg 2: Surface for the 3D-perspective
          auto surfaceFilter = mitk::ImageToSurfaceFilter::New();
          surfaceFilter->SetInput(selectedImage);
          surfaceFilter->SetThreshold(1);
          surfaceFilter->SetSmooth(true);
          //Downsampling
          surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);

          auto surfaceNode = mitk::DataNode::New();
          surfaceNode->SetData(surfaceFilter->GetOutput());
          surfaceNode->SetProperty("color", selectedNode->GetProperty("color"));
          surfaceNode->SetOpacity(0.5);
          surfaceNode->SetName(selectedNode->GetName());
          this->GetDataStorage()->Add(surfaceNode, selectedNode);
        }
      }

      //If an image is selected trim the plane to this.
      imageDiagonal = selectedImage->GetGeometry()->GetDiagonalLength();
      plane->SetOrigin(selectedImage->GetGeometry()->GetCenter());

      // Rotate plane
      mitk::Vector3D rotationAxis;
      mitk::FillVector3D(rotationAxis, 0.0, 1.0, 0.0);
      mitk::RotationOperation op(mitk::OpROTATE, selectedImage->GetGeometry()->GetCenter(), rotationAxis, 90.0);
      plane->GetGeometry()->ExecuteOperation(&op);
    }
  }

  // equivalent to the extent and resolution function of the clipping plane
  const auto x = imageDiagonal * 0.9;
  planeSource->SetOrigin(-x / 2.0, -x / 2.0, 0.0);
  planeSource->SetPoint1(x / 2.0, -x / 2.0, 0.0);
  planeSource->SetPoint2(-x / 2.0, x / 2.0, 0.0);
  planeSource->SetResolution(64, 64);
  planeSource->Update();

  plane->SetVtkPolyData(planeSource->GetOutput());

  // Set scalars (for colorization of plane)
  vtkFloatArray *scalars = vtkFloatArray::New();
  scalars->SetName("Distance");
  scalars->SetNumberOfComponents(1);

  const auto numerOfPoints = plane->GetVtkPolyData(0)->GetNumberOfPoints();
  for (std::remove_const_t<decltype(numerOfPoints)> i = 0; i < plane->GetVtkPolyData(0)->GetNumberOfPoints(); ++i)
  {
    scalars->InsertNextValue(-1.0);
  }
  plane->GetVtkPolyData(0)->GetPointData()->SetScalars(scalars);
  plane->GetVtkPolyData(0)->GetPointData()->Update();

  auto planeNode = mitk::DataNode::New();
  planeNode->SetData(plane);

  std::stringstream planeName;
  planeName << "ClippingPlane ";
  planeName << this->GetAllClippingPlanes()->Size() + 1;

  planeNode->SetName(planeName.str());
  planeNode->AddProperty("clippingPlane", mitk::BoolProperty::New(true));
  // Make plane pickable
  planeNode->SetBoolProperty("pickable", true);

  mitk::SurfaceVtkMapper3D::SetDefaultProperties(planeNode);

  // Don't include plane in bounding box!
  planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

  // Set lookup table for plane surface visualization
  auto lookupTablevtk = vtkSmartPointer<vtkLookupTable>::New();
  lookupTablevtk->SetHueRange(0.6, 0.0);
  lookupTablevtk->SetSaturationRange(1.0, 1.0);
  lookupTablevtk->SetValueRange(1.0, 1.0);
  lookupTablevtk->SetTableRange(-1.0, 1.0);
  lookupTablevtk->Build();

  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetVtkLookupTable(lookupTablevtk);

  auto prop = mitk::LookupTableProperty::New(lookupTable);

  planeNode->SetProperty("LookupTable", prop);
  planeNode->SetBoolProperty("scalar visibility", true);
  planeNode->SetBoolProperty("color mode", true);
  planeNode->SetFloatProperty("ScalarsRangeMinimum", -1.0);
  planeNode->SetFloatProperty("ScalarsRangeMaximum", 1.0);

  // Configure material so that only scalar colors are shown
  planeNode->SetColor(0.0f, 0.0f, 0.0f);
  planeNode->SetOpacity(1.0f);
  planeNode->SetFloatProperty("material.wireframeLineWidth", 2.0f);

  //Set view of plane to wireframe
  planeNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));

  //Add the plane to data storage
  this->GetDataStorage()->Add(planeNode);

  //Change the index of the selector to the new generated node
  m_Controls->clippingPlaneSelector->setCurrentIndex(m_Controls->clippingPlaneSelector->Find(planeNode));

  m_Controls->interactionSelectionBox->setEnabled(true);

  if (auto renderWindowPart = dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
  {
    renderWindowPart->EnableSlicingPlanes(false);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableClippingPlaneView::OnCalculateClippingVolume()
{
  auto selectedNode = m_Controls->imageSelectionWidget->GetSelectedNode();
  if (selectedNode.IsNull())
  {
    MITK_ERROR << "No segmentation selected! Can't calculate volume";
    return;
  }

  bool isSegmentation = false;
  selectedNode->GetBoolProperty("binary", isSegmentation);
  if (!isSegmentation)
  {
    MITK_ERROR << "No segmentation selected! Can't calculate volume";
    return;
  }

  std::vector<mitk::Surface*> clippingPlanes;
  mitk::DataStorage::SetOfObjects::ConstPointer allClippingPlanes = this->GetAllClippingPlanes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator itPlanes = allClippingPlanes->Begin(); itPlanes != allClippingPlanes->End(); itPlanes++)
  {
    bool isVisible = false;
    itPlanes.Value()->GetBoolProperty("visible", isVisible);
    auto plane = dynamic_cast<mitk::Surface*>(itPlanes.Value()->GetData());

    if (isVisible && nullptr != plane)
      clippingPlanes.push_back(plane);
  }

  if (clippingPlanes.empty())
  {
    MITK_ERROR << "No clipping plane selected! Can't calculate volume";
    return;
  }

  // deactivate Tools
  this->DeactivateInteractionButtons();
  //Clear the list of volumes, before calculating the new values
  m_Controls->volumeList->clear();

  selectedNode->SetBoolProperty("visible", false);

  //set some properties for clipping the image-->Output: labled Image
  mitk::HeightFieldSurfaceClipImageFilter::Pointer surfaceClipFilter = mitk::HeightFieldSurfaceClipImageFilter::New();

  surfaceClipFilter->SetInput(dynamic_cast<mitk::Image*>(selectedNode->GetData()));
  surfaceClipFilter->SetClippingModeToMultiPlaneValue();
  surfaceClipFilter->SetClippingSurfaces(clippingPlanes);
  surfaceClipFilter->Update();

  //delete the old clipped image node
  mitk::DataStorage::SetOfObjects::ConstPointer oldClippedNode = this->GetDataStorage()->GetSubset(mitk::NodePredicateProperty::New("name", mitk::StringProperty::New("Clipped Image")));
  if (oldClippedNode.IsNotNull())
    this->GetDataStorage()->Remove(oldClippedNode);

  //add the new clipped image node
  auto clippedNode = mitk::DataNode::New();
  mitk::Image::Pointer clippedImage = surfaceClipFilter->GetOutput();
  clippedImage->DisconnectPipeline();
  clippedNode->SetData(clippedImage);
  clippedNode->SetName("Clipped Image");
  clippedNode->SetColor(1.0, 1.0, 1.0);  // color property will not be used, labeled image lookuptable will be used instead
  clippedNode->SetProperty("use color", mitk::BoolProperty::New(false));
  clippedNode->SetProperty("reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_NEAREST));
  clippedNode->SetOpacity(0.4);
  this->GetDataStorage()->Add(clippedNode);

  auto volumeCalculator = mitk::LabeledImageVolumeCalculator::New();
  volumeCalculator->SetImage(clippedImage);
  volumeCalculator->Calculate();

  auto volumes = volumeCalculator->GetVolumes();

  auto lookupTable = mitk::LabeledImageLookupTable::New();
  int lablesWithVolume = 0;
  const auto numberOfVolumes = volumes.size();
  for (std::remove_const_t<decltype(numberOfVolumes)> i = 1; i < numberOfVolumes; ++i)
  {
    if (0 != volumes[0])
    {
      lablesWithVolume++;

      mitk::Color color(GetLabelColor(lablesWithVolume));
      lookupTable->SetColorForLabel(i, color.GetRed(), color.GetGreen(), color.GetBlue(), 1.0);

      QColor qcolor;
      qcolor.setRgbF(color.GetRed(), color.GetGreen(), color.GetBlue(), 0.7);

      //output volume as string "x.xx ml"
      std::stringstream stream;
      stream << std::fixed << std::setprecision(2) << 0.001 * volumes[i] << " ml";
      stream << " ml";

      auto item = new QListWidgetItem();
      item->setText(QString::fromStdString(stream.str()));
      item->setBackgroundColor(qcolor);
      m_Controls->volumeList->addItem(item);
    }
  }

  //set the rendering mode to use the lookup table and level window
  clippedNode->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
  mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New(lookupTable.GetPointer());
  clippedNode->SetProperty("LookupTable", lutProp);
  // it is absolutely important, to use the LevelWindow settings provided by
  // the LUT generator, otherwise, it is not guaranteed, that colors show
  // up correctly.
  clippedNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(lookupTable->GetLevelWindow()));
}

void QmitkDeformableClippingPlaneView::OnTranslationMode(bool check)
{
  if (check)
  { //uncheck all other buttons
    m_Controls->rotationPushButton->setChecked(false);
    m_Controls->deformationPushButton->setChecked(false);

    mitk::ClippingPlaneInteractor3D::Pointer affineDataInteractor = mitk::ClippingPlaneInteractor3D::New();
    affineDataInteractor->LoadStateMachine("ClippingPlaneInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("ClippingPlaneTranslationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(nullptr);
}

void QmitkDeformableClippingPlaneView::OnRotationMode(bool check)
{
  if (check)
  { //uncheck all other buttons
    m_Controls->translationPushButton->setChecked(false);
    m_Controls->deformationPushButton->setChecked(false);

    mitk::ClippingPlaneInteractor3D::Pointer affineDataInteractor = mitk::ClippingPlaneInteractor3D::New();
    affineDataInteractor->LoadStateMachine("ClippingPlaneInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("ClippingPlaneRotationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(nullptr);
}

void QmitkDeformableClippingPlaneView::OnDeformationMode(bool check)
{
  if (check)
  { //uncheck all other buttons
    m_Controls->translationPushButton->setChecked(false);
    m_Controls->rotationPushButton->setChecked(false);

    mitk::SurfaceDeformationDataInteractor3D::Pointer surfaceDataInteractor = mitk::SurfaceDeformationDataInteractor3D::New();
    surfaceDataInteractor->LoadStateMachine("ClippingPlaneInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    surfaceDataInteractor->SetEventConfig("ClippingPlaneDeformationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    surfaceDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(nullptr);
}

void QmitkDeformableClippingPlaneView::CreateConnections()
{
  connect(m_Controls->imageSelectionWidget, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkDeformableClippingPlaneView::OnCurrentSelectionChanged);
  connect(m_Controls->translationPushButton, &QPushButton::toggled,
    this, &QmitkDeformableClippingPlaneView::OnTranslationMode);
  connect(m_Controls->rotationPushButton, &QPushButton::toggled,
    this, &QmitkDeformableClippingPlaneView::OnRotationMode);
  connect(m_Controls->deformationPushButton, &QPushButton::toggled,
    this, &QmitkDeformableClippingPlaneView::OnDeformationMode);
  connect(m_Controls->createNewPlanePushButton, &QPushButton::clicked,
    this, &QmitkDeformableClippingPlaneView::OnCreateNewClippingPlane);
  connect(m_Controls->updateVolumePushButton, &QPushButton::clicked,
    this, &QmitkDeformableClippingPlaneView::OnCalculateClippingVolume);
  connect(m_Controls->clippingPlaneSelector, &QmitkDataStorageComboBox::OnSelectionChanged,
    this, &QmitkDeformableClippingPlaneView::OnComboBoxSelectionChanged);
}

void QmitkDeformableClippingPlaneView::NodeRemoved(const mitk::DataNode* node)
{
  if (m_IsClippingPlanePredicate->CheckNode(node))
  {
    if (this->GetAllClippingPlanes()->Size() <= 1)
    {
      m_WorkingNode = nullptr;
      this->UpdateView();
    }
  }
}

void::QmitkDeformableClippingPlaneView::NodeChanged(const mitk::DataNode*)
{
  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::UpdateView()
{
  auto selectedNode = m_Controls->imageSelectionWidget->GetSelectedNode();
  if (selectedNode.IsNotNull())
  {
    m_Controls->selectedReferenceImageLabel->setText(QString::fromUtf8(selectedNode->GetName().c_str()));
    if (m_WorkingNode.IsNotNull())
    {
      bool isSegmentation = false;
      selectedNode->GetBoolProperty("binary", isSegmentation);
      m_Controls->interactionSelectionBox->setEnabled(true);

      m_Controls->volumeGroupBox->setEnabled(isSegmentation);

      //clear list --> than search for all shown clipping plans (max 7 planes)
      m_Controls->selectedClippingPlanesLabel->setText("");
      m_Controls->planesWarningLabel->hide();
      int volumePlanes = 0;

      auto allClippingPlanes = this->GetAllClippingPlanes();
      for (mitk::DataStorage::SetOfObjects::ConstIterator itPlanes = allClippingPlanes->Begin(); itPlanes != allClippingPlanes->End(); itPlanes++)
      {
        bool isVisible = false;
        itPlanes.Value()->GetBoolProperty("visible", isVisible);
        if (isVisible)
        {
          if (volumePlanes < 7)
          {
            ++volumePlanes;
            m_Controls->selectedClippingPlanesLabel->setText(m_Controls->selectedClippingPlanesLabel->text().append(QString::fromStdString(itPlanes.Value()->GetName()+"\n")));
          }
          else
          {
            m_Controls->planesWarningLabel->show();
            return;
          }
        }
      }
    }
    else
    {
      m_Controls->volumeGroupBox->setEnabled(false);
      m_Controls->interactionSelectionBox->setEnabled(false);
      m_Controls->selectedClippingPlanesLabel->setText("");
      m_Controls->volumeList->clear();
    }
  }
  else
  {
    m_Controls->volumeGroupBox->setEnabled(false);
    m_Controls->selectedReferenceImageLabel->setText("");
    m_Controls->selectedClippingPlanesLabel->setText("");
    m_Controls->planesWarningLabel->hide();

    m_Controls->interactionSelectionBox->setEnabled(m_WorkingNode.IsNotNull());
  }
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDeformableClippingPlaneView::GetAllClippingPlanes()
{
  auto allPlanes = GetDataStorage()->GetSubset(m_IsClippingPlanePredicate);
  return allPlanes;
}

mitk::Color QmitkDeformableClippingPlaneView::GetLabelColor(int label)
{
  std::array<float, 3> color = { 0.0f, 0.0f, 0.0f };

  switch (label % 6)
  {
  case 0: // red
    color[0] = 1.0f;
    break;
  case 1: // green
    color[1] = 1.0f;
    break;
  case 2: // blue
    color[2] = 1.0f;
    break;
  case 3: // yellow
    color[0] = 1.0f;
    color[1] = 1.0f;
    break;
  case 4: // magenta
    color[0] = 1.0f;
    color[2] = 1.0f;
    break;
  case 5: // cyan
    color[1] = 1.0f;
    color[2] = 1.0f;
  default: // black
    break;
  }

  int outerCycleNr = label / 6;
  int cycleSize = std::min(1, static_cast<int>(std::pow(2.0, std::log(outerCycleNr) / std::log(2.0))));
  int insideCycleCounter = outerCycleNr % cycleSize;

  float factor;
  if (0 == outerCycleNr)
  {
    factor = 255.0f;
  }
  else
  {
    factor = 256.0f / (2.0f * cycleSize) + insideCycleCounter * (256.0f / cycleSize);
  }

  color = {
    std::min(1.0f, color[0] / 256.0f * factor),
    std::min(1.0f, color[1] / 256.0f * factor),
    std::min(1.0f, color[2] / 256.0f * factor)
  };

  return mitk::Color(color.data());
}

void QmitkDeformableClippingPlaneView::DeactivateInteractionButtons()
{
  m_Controls->translationPushButton->setChecked(false);
  m_Controls->rotationPushButton->setChecked(false);
  m_Controls->deformationPushButton->setChecked(false);
}
