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

#include "QmitkDeformableClippingPlaneView.h"

#include "mitkAffineDataInteractor3D.h"
#include "mitkHeightFieldSurfaceClipImageFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkInteractionConst.h"
#include "mitkLabeledImageLookupTable.h"
#include "mitkLabeledImageVolumeCalculator.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkRenderingModeProperty.h"
#include "mitkRotationOperation.h"
#include "mitkSurfaceDeformationDataInteractor3D.h"
#include "mitkSurfaceVtkMapper3D.h"
#include "mitkVtkRepresentationProperty.h"
#include "usModuleRegistry.h"

#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include <vtkPlaneSource.h>


const std::string QmitkDeformableClippingPlaneView::VIEW_ID = "org.mitk.views.deformableclippingplane";

QmitkDeformableClippingPlaneView::QmitkDeformableClippingPlaneView()
  : QmitkFunctionality()
  , m_MultiWidget(NULL)
  , m_ReferenceNode(NULL)
  , m_WorkingNode(NULL)
{
}

QmitkDeformableClippingPlaneView::~QmitkDeformableClippingPlaneView()
{
}

void QmitkDeformableClippingPlaneView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);
  this->CreateConnections();
}

void QmitkDeformableClippingPlaneView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkDeformableClippingPlaneView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkDeformableClippingPlaneView::CreateConnections()
{
  mitk::NodePredicateProperty::Pointer clipPredicate = mitk::NodePredicateProperty::New("clippingPlane",mitk::BoolProperty::New(true));
  //set only clipping planes in the list of the selector
  m_Controls.clippingPlaneSelector->SetDataStorage(this->GetDefaultDataStorage());
  m_Controls.clippingPlaneSelector->SetPredicate(clipPredicate);


  //No working data set, yet
  m_Controls.volumeGroupBox->setEnabled(false);
  m_Controls.interactionSelectionBox->setEnabled(false);
  m_Controls.noSelectedImageLabel->show();
  m_Controls.planesWarningLabel->hide();

  connect (m_Controls.translationPushButton, SIGNAL(toggled(bool)), this, SLOT(OnTranslationMode(bool)));
  connect (m_Controls.rotationPushButton, SIGNAL(toggled(bool)), this, SLOT(OnRotationMode(bool)));
  connect (m_Controls.deformationPushButton, SIGNAL(toggled(bool)), this, SLOT(OnDeformationMode(bool)));
  connect (m_Controls.createNewPlanePushButton, SIGNAL(clicked()), this, SLOT(OnCreateNewClippingPlane()));
  connect (m_Controls.updateVolumePushButton, SIGNAL(clicked()), this, SLOT(OnCalculateClippingVolume()));
  connect (m_Controls.clippingPlaneSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnComboBoxSelectionChanged(const mitk::DataNode*)));
}

void QmitkDeformableClippingPlaneView::Activated()
{
  QmitkFunctionality::Activated();
  //If a tube graph already exist in the data storage, set the working node correctly
  m_WorkingNode = m_Controls.clippingPlaneSelector->GetSelectedNode();
  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::Deactivated()
{
  if(m_WorkingNode.IsNotNull())
  {
    if(m_WorkingNode->GetDataInteractor().IsNotNull())
      m_WorkingNode->SetDataInteractor(NULL);
  }
  QmitkFunctionality::Deactivated();
}

void QmitkDeformableClippingPlaneView::OnComboBoxSelectionChanged( const mitk::DataNode* node )
{
  mitk::DataNode* selectedNode = const_cast<mitk::DataNode*>(node);
  if( selectedNode != NULL )
  {
    if(m_WorkingNode.IsNotNull())
      selectedNode->SetDataInteractor(m_WorkingNode->GetDataInteractor());

    m_WorkingNode = selectedNode;
  }
  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::OnSelectionChanged(mitk::DataNode* node)
{
  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);
  this->OnSelectionChanged(nodes);
}

void QmitkDeformableClippingPlaneView::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
  bool isClippingPlane(false);
  for(unsigned int i = 0; i < nodes.size(); ++i)
  {
    if(nodes.at(i)->GetBoolProperty("clippingPlane", isClippingPlane))
      m_Controls.clippingPlaneSelector->setCurrentIndex( m_Controls.clippingPlaneSelector->Find(nodes.at(i)) );

    else
    {
      if(dynamic_cast<mitk::Image*> (nodes.at(i)->GetData())&& nodes.at(i))
      {
        if(m_ReferenceNode.IsNotNull() && nodes.at(i)->GetData() == m_ReferenceNode->GetData())
          return;

        m_ReferenceNode =nodes.at(i);
      }
    }
  }
  this->UpdateView();
}

void::QmitkDeformableClippingPlaneView::NodeChanged(const mitk::DataNode* /*node*/)
{
  this->UpdateView();
}

void QmitkDeformableClippingPlaneView::NodeRemoved(const mitk::DataNode* node)
{
  bool isClippingPlane(false);

  if (node->GetBoolProperty("clippingPlane", isClippingPlane))
  {
    if(this->GetAllClippingPlanes()->Size()<=1)
    {
      m_WorkingNode = NULL;
      this->UpdateView();
    }
    else
    {
      if (GetAllClippingPlanes()->front()!= node)
        this->OnSelectionChanged(GetAllClippingPlanes()->front());
      else
        this->OnSelectionChanged(GetAllClippingPlanes()->ElementAt(1));
    }
  }
  else
  {
    if(m_ReferenceNode.IsNotNull())
    {
      if(node->GetData() == m_ReferenceNode->GetData())
      {
        m_ReferenceNode = NULL;
        m_Controls.volumeList->clear();
      }
      this->UpdateView();
    }
  }
}

void QmitkDeformableClippingPlaneView::UpdateView()
{
  if (m_ReferenceNode.IsNotNull())
  {
    m_Controls.noSelectedImageLabel->hide();

    m_Controls.selectedImageLabel->setText(QString::fromUtf8(m_ReferenceNode->GetName().c_str()));

    if (m_WorkingNode.IsNotNull())
    {
      bool isSegmentation(false);
      m_ReferenceNode->GetBoolProperty("binary", isSegmentation);
      m_Controls.interactionSelectionBox->setEnabled(true);

      m_Controls.volumeGroupBox->setEnabled(isSegmentation);

      //clear list --> than search for all shown clipping plans (max 7 planes)
      m_Controls.selectedVolumePlanesLabel->setText("");
      m_Controls.planesWarningLabel->hide();
      int volumePlanes=0;

      mitk::DataStorage::SetOfObjects::ConstPointer allClippingPlanes = this->GetAllClippingPlanes();
      for (mitk::DataStorage::SetOfObjects::ConstIterator itPlanes = allClippingPlanes->Begin(); itPlanes != allClippingPlanes->End(); itPlanes++)
      {
        bool isVisible(false);
        itPlanes.Value()->GetBoolProperty("visible",isVisible);
        if (isVisible)
        {
          if (volumePlanes<7)
          {
            volumePlanes ++;
            m_Controls.selectedVolumePlanesLabel->setText(m_Controls.selectedVolumePlanesLabel->text().append(QString::fromStdString(itPlanes.Value()->GetName()+"\n")));
          }
          else
          {
            m_Controls.planesWarningLabel->show();
            return;
          }
        }
      }
    }
    else
    {
      m_Controls.volumeGroupBox->setEnabled(false);
      m_Controls.interactionSelectionBox->setEnabled(false);
      m_Controls.selectedVolumePlanesLabel->setText("");
      m_Controls.volumeList->clear();
    }
  }

  else
  {
    m_Controls.volumeGroupBox->setEnabled(false);
    m_Controls.noSelectedImageLabel->show();
    m_Controls.selectedImageLabel->setText("");
    m_Controls.selectedVolumePlanesLabel->setText("");
    m_Controls.planesWarningLabel->hide();
    if(m_WorkingNode.IsNull())
      m_Controls.interactionSelectionBox->setEnabled(false);
    else
      m_Controls.interactionSelectionBox->setEnabled(true);
  }
}

void QmitkDeformableClippingPlaneView::OnCreateNewClippingPlane()
{
  //the new clipping plane
  mitk::Surface::Pointer plane = mitk::Surface::New();
  mitk::Image::Pointer referenceImage = mitk::Image::New();
  vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();

  // default initialization of the clipping plane
  planeSource->SetOrigin( -32.0, -32.0, 0.0 );
  planeSource->SetPoint1(  32.0, -32.0, 0.0 );
  planeSource->SetPoint2( -32.0,  32.0, 0.0 );
  planeSource->SetResolution( 128, 128 );
  planeSource->Update();

  plane->SetVtkPolyData(planeSource->GetOutput());


  double imageDiagonal = 200;

  if (m_ReferenceNode.IsNotNull())
  {
    referenceImage = dynamic_cast<mitk::Image*> (m_ReferenceNode->GetData());

    if (referenceImage.IsNotNull())
    {
      // check if user wants a surface model
      if(m_Controls.surfaceModelCheckBox->isChecked())
      {
        //Check if there is a surface node from the image. If not, create one
        bool createSurfaceFromImage(true);
        mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
        mitk::NodePredicateDataType::Pointer isSurface = mitk::NodePredicateDataType::New("Surface");
        mitk::DataStorage::SetOfObjects::ConstPointer childNodes =  GetDataStorage()->GetDerivations(m_ReferenceNode,isSurface, true);

        for (mitk::DataStorage::SetOfObjects::ConstIterator itChildNodes = childNodes->Begin();
          itChildNodes != childNodes->End(); itChildNodes++)
        {
          if (itChildNodes.Value().IsNotNull())
            createSurfaceFromImage=false;
        }

        if(createSurfaceFromImage)
        {
          //Lsg 2: Surface for the 3D-perspective
          mitk::ImageToSurfaceFilter::Pointer surfaceFilter = mitk::ImageToSurfaceFilter::New();
          surfaceFilter->SetInput(referenceImage);
          surfaceFilter->SetThreshold(1);
          surfaceFilter->SetSmooth(true);
          //Downsampling
          surfaceFilter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);

          mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
          surfaceNode->SetData(surfaceFilter->GetOutput());
          surfaceNode->SetProperty("color", m_ReferenceNode->GetProperty("color"));
          surfaceNode->SetOpacity(0.5);
          surfaceNode->SetName(m_ReferenceNode->GetName());
          GetDataStorage()->Add(surfaceNode, m_ReferenceNode);
        }
      }

      //If an image is selected trim the plane to this.
      imageDiagonal = referenceImage->GetGeometry()->GetDiagonalLength();
      plane->SetOrigin( referenceImage->GetGeometry()->GetCenter());

      // Rotate plane
      mitk::Vector3D rotationAxis;
      mitk::FillVector3D(rotationAxis, 0.0, 1.0, 0.0);
      mitk::RotationOperation op(mitk::OpROTATE, referenceImage->GetGeometry()->GetCenter(), rotationAxis, 90.0);
      plane->GetGeometry()->ExecuteOperation(&op);
    }
  }

  //set some properties for the clipping plane
  // plane->SetExtent(imageDiagonal * 0.9, imageDiagonal * 0.9);
  // plane->SetResolution(64, 64);

  // eequivalent to the extent and resolution function of the clipping plane
  const double x = imageDiagonal * 0.9;
  planeSource->SetOrigin( -x / 2.0, -x / 2.0, 0.0 );
  planeSource->SetPoint1(  x / 2.0, -x / 2.0, 0.0 );
  planeSource->SetPoint2( -x / 2.0,  x / 2.0, 0.0 );
  planeSource->SetResolution( 64, 64 );
  planeSource->Update();

  plane->SetVtkPolyData(planeSource->GetOutput());

  // Set scalars (for colorization of plane)
  vtkFloatArray *scalars = vtkFloatArray::New();
  scalars->SetName("Distance");
  scalars->SetNumberOfComponents(1);

  for ( unsigned int i = 0; i < plane->GetVtkPolyData(0)->GetNumberOfPoints(); ++i)
  {
    scalars->InsertNextValue(-1.0);
  }
  plane->GetVtkPolyData(0)->GetPointData()->SetScalars(scalars);
  plane->GetVtkPolyData(0)->GetPointData()->Update();

  mitk::DataNode::Pointer planeNode = mitk::DataNode::New();
  planeNode->SetData(plane);

  std::stringstream planeName;
  planeName << "ClippingPlane ";
  planeName << this->GetAllClippingPlanes()->Size() + 1;

  planeNode->SetName(planeName.str());
  planeNode->AddProperty("clippingPlane",mitk::BoolProperty::New(true));
  // Make plane pickable
  planeNode->SetBoolProperty("pickable", true);

  mitk::SurfaceVtkMapper3D::SetDefaultProperties(planeNode);

  // Don't include plane in bounding box!
  planeNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));

  // Set lookup table for plane surface visualization
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetHueRange(0.6, 0.0);
  lookupTable->SetSaturationRange(1.0, 1.0);
  lookupTable->SetValueRange(1.0, 1.0);
  lookupTable->SetTableRange(-1.0, 1.0);
  lookupTable->Build();

  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut->SetVtkLookupTable(lookupTable);

  mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);

  planeNode->SetProperty("LookupTable", prop);
  planeNode->SetBoolProperty("scalar visibility", true);
  planeNode->SetBoolProperty("color mode", true);
  planeNode->SetFloatProperty("ScalarsRangeMinimum", -1.0);
  planeNode->SetFloatProperty("ScalarsRangeMaximum", 1.0);

  // Configure material so that only scalar colors are shown
  planeNode->SetColor(0.0f,0.0f,0.0f);
  planeNode->SetOpacity(1.0f);
  planeNode->SetFloatProperty("material.wireframeLineWidth",2.0f);

  //Set view of plane to wireframe
  planeNode->SetProperty("material.representation", mitk::VtkRepresentationProperty::New(VTK_WIREFRAME));

  //Set the plane as working data for the tools and selected it
  this->OnSelectionChanged (planeNode);

  //Add the plane to data storage
  this->GetDataStorage()->Add(planeNode);

  //Change the index of the selector to the new generated node
  m_Controls.clippingPlaneSelector->setCurrentIndex( m_Controls.clippingPlaneSelector->Find(planeNode) );

  m_Controls.interactionSelectionBox->setEnabled(true);

  // set crosshair invisible
  mitk::DataNode* dataNode;

  dataNode = this->m_MultiWidget->GetWidgetPlane1();
  if(dataNode) dataNode->SetVisibility(false);
  dataNode = this->m_MultiWidget->GetWidgetPlane2();
  if(dataNode) dataNode->SetVisibility(false);
  dataNode = this->m_MultiWidget->GetWidgetPlane3();
  if(dataNode) dataNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableClippingPlaneView::OnCalculateClippingVolume()
{
  bool isSegmentation(false);
  m_ReferenceNode->GetBoolProperty("binary", isSegmentation);

  if(m_ReferenceNode.IsNull() || !isSegmentation)
  {
    MITK_ERROR << "No segmentation selected! Can't calculate volume";
    return;
  }

  std::vector<mitk::Surface*> clippingPlanes;
  mitk::DataStorage::SetOfObjects::ConstPointer allClippingPlanes = this->GetAllClippingPlanes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator itPlanes = allClippingPlanes->Begin(); itPlanes != allClippingPlanes->End(); itPlanes++)
  {
    bool isVisible(false);
    itPlanes.Value()->GetBoolProperty("visible",isVisible);
    mitk::Surface* plane = dynamic_cast<mitk::Surface*>(itPlanes.Value()->GetData());

    if (isVisible && plane)
      clippingPlanes.push_back(plane);
  }

  if (clippingPlanes.empty())
  {
    MITK_ERROR << "No clipping plane selected! Can't calculate volume";
    return;
  }

  // deactivate Tools
  m_Controls.translationPushButton->setChecked(false);
  m_Controls.rotationPushButton->setChecked(false);
  m_Controls.deformationPushButton->setChecked(false);

  //Clear the list of volumes, before calculating the new values
  m_Controls.volumeList->clear();

  m_ReferenceNode->SetBoolProperty("visible", false);

  //set some properties for clipping the image-->Output: labled Image
  mitk::HeightFieldSurfaceClipImageFilter::Pointer surfaceClipFilter = mitk::HeightFieldSurfaceClipImageFilter::New();

  surfaceClipFilter->SetInput(dynamic_cast<mitk::Image*> (m_ReferenceNode->GetData()));
  surfaceClipFilter->SetClippingModeToMultiPlaneValue();
  surfaceClipFilter->SetClippingSurfaces(clippingPlanes);
  surfaceClipFilter->Update();

  //delete the old clipped image node
  mitk::DataStorage::SetOfObjects::ConstPointer oldClippedNode = this->GetDataStorage()->GetSubset(mitk::NodePredicateProperty::New("name", mitk::StringProperty::New("Clipped Image")));
  if (oldClippedNode.IsNotNull())
    this->GetDataStorage()->Remove(oldClippedNode);

  //add the new clipped image node
  mitk::DataNode::Pointer clippedNode = mitk::DataNode::New();
  mitk::Image::Pointer clippedImage = surfaceClipFilter->GetOutput();
  clippedImage->DisconnectPipeline();
  clippedNode->SetData(clippedImage);
  //clippedNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  clippedNode->SetName("Clipped Image");
  clippedNode->SetColor(1.0,1.0,1.0);  // color property will not be used, labeled image lookuptable will be used instead
  clippedNode->SetProperty ("use color", mitk::BoolProperty::New(false));
  clippedNode->SetOpacity(0.4);
  this->GetDataStorage()->Add(clippedNode);

  mitk::LabeledImageVolumeCalculator::Pointer volumeCalculator = mitk::LabeledImageVolumeCalculator::New();
  volumeCalculator->SetImage(clippedImage);
  volumeCalculator->Calculate();

  std::vector<double> volumes = volumeCalculator->GetVolumes();

  mitk::LabeledImageLookupTable::Pointer lut = mitk::LabeledImageLookupTable::New();
  int lablesWithVolume=0;

  for(unsigned int i = 1; i < volumes.size(); ++i)
  {
    if(volumes.at(i)!=0)
    {
      lablesWithVolume++;

      mitk::Color color (GetLabelColor(lablesWithVolume));
      lut->SetColorForLabel(i,color.GetRed(), color.GetGreen(), color.GetBlue(), 1.0);

      QColor qcolor;
      qcolor.setRgbF(color.GetRed(), color.GetGreen(), color.GetBlue(), 0.7);

      //output volume as string "x.xx ml"
      std::stringstream stream;
      stream<< std::fixed << std::setprecision(2)<<volumes.at(i)/1000;
      stream<<" ml";

      QListWidgetItem* item = new QListWidgetItem();
      item->setText(QString::fromStdString(stream.str()));
      item->setBackgroundColor(qcolor);
      m_Controls.volumeList->addItem(item);
    }
  }

  //set the rendering mode to use the lookup table and level window
  clippedNode->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
  mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New(lut.GetPointer());
  clippedNode->SetProperty("LookupTable", lutProp);
  // it is absolutely important, to use the LevelWindow settings provided by
  // the LUT generator, otherwise, it is not guaranteed, that colors show
  // up correctly.
  clippedNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(lut->GetLevelWindow()));
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDeformableClippingPlaneView::GetAllClippingPlanes()
{
  mitk::NodePredicateProperty::Pointer clipPredicate= mitk::NodePredicateProperty::New("clippingPlane",mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer allPlanes = GetDataStorage()->GetSubset(clipPredicate);
  return allPlanes;
}

mitk::Color QmitkDeformableClippingPlaneView::GetLabelColor(int label)
{
  float red, green, blue;
  switch ( label % 6 )
  {
  case 0:
    {red = 1.0; green = 0.0; blue = 0.0; break;}
  case 1:
    {red = 0.0; green = 1.0; blue = 0.0; break;}
  case 2:
    {red = 0.0; green = 0.0; blue = 1.0;break;}
  case 3:
    {red = 1.0; green = 1.0; blue = 0.0;break;}
  case 4:
    {red = 1.0; green = 0.0; blue = 1.0;break;}
  case 5:
    {red = 0.0; green = 1.0; blue = 1.0;break;}
  default:
    {red = 0.0; green = 0.0; blue = 0.0;}
  }

  float tmp[3] = { red, green, blue };

  double factor;

  int outerCycleNr = label / 6;
  int cycleSize = pow(2.0,(int)(log((double)(outerCycleNr))/log( 2.0 )));
  if (cycleSize==0)
    cycleSize = 1;
  int insideCycleCounter = outerCycleNr % cycleSize;

  if ( outerCycleNr == 0)
    factor = 255;
  else
    factor = ( 256 / ( 2 * cycleSize ) ) + ( insideCycleCounter * ( 256 / cycleSize ) );

  tmp[0]= tmp[0]/256*factor;
  tmp[1]= tmp[1]/256*factor;
  tmp[2]= tmp[2]/256*factor;

  return mitk::Color(tmp);
}

void QmitkDeformableClippingPlaneView::OnTranslationMode(bool check)
{
  if(check)
  { //uncheck all other buttons
    m_Controls.rotationPushButton->setChecked(false);
    m_Controls.deformationPushButton->setChecked(false);

    mitk::AffineDataInteractor3D::Pointer affineDataInteractor = mitk::AffineDataInteractor3D::New();
    affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("AffineTranslationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(NULL);
}

void QmitkDeformableClippingPlaneView::OnRotationMode(bool check)
{
  if(check)
  { //uncheck all other buttons
    m_Controls.translationPushButton->setChecked(false);
    m_Controls.deformationPushButton->setChecked(false);

    mitk::AffineDataInteractor3D::Pointer affineDataInteractor = mitk::AffineDataInteractor3D::New();
    affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("AffineRotationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(NULL);
}

void QmitkDeformableClippingPlaneView::OnDeformationMode(bool check)
{
  if(check)
  { //uncheck all other buttons
    m_Controls.translationPushButton->setChecked(false);
    m_Controls.rotationPushButton->setChecked(false);

    mitk::SurfaceDeformationDataInteractor3D::Pointer surfaceDataInteractor = mitk::SurfaceDeformationDataInteractor3D::New();
    surfaceDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    surfaceDataInteractor->SetEventConfig("AffineDeformationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    surfaceDataInteractor->SetDataNode(m_WorkingNode);
  }
  else
    m_WorkingNode->SetDataInteractor(NULL);
}
