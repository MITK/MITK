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

// Qt
#include <QMessageBox>
#include <QMenu>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK
#include <mitkNodePredicateProperty.h>

#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

// Qmitk
#include "RTDoseVisualizer.h"

#include <DataStructures/mitkRTConstants.h>
#include <DataStructures/mitkIsoDoseLevelSetProperty.h>
#include <DataStructures/mitkIsoDoseLevelVectorProperty.h>
#include <mitkIsoLevelsGenerator.h>
#include <mitkRTUIConstants.h>

#include <QmitkDoseColorDelegate.h>
#include <QmitkDoseValueDelegate.h>
#include <QmitkDoseVisualStyleDelegate.h>
#include <QmitkIsoDoseLevelSetModel.h>
#include <QmitkFreeIsoDoseLevelWidget.h>

#include "org_mitk_gui_qt_dosevisualization_Activator.h"

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <vtkMath.h>

#include <mitkSurface.h>
#include "QmitkRenderWindow.h"
#include <mitkExtractSliceFilter.h>

#include "mitkSurfaceVtkMapper3D.h"

#include "mitkSliceNavigationController.h"

#include "vtkDecimatePro.h"

const std::string RTDoseVisualizer::VIEW_ID = "org.mitk.views.rt.dosevisualization";

RTDoseVisualizer::RTDoseVisualizer()
{
  m_freeIsoValues = mitk::IsoDoseLevelVector::New();
  m_selectedNodeIsoSet = mitk::IsoDoseLevelSet::New();

  m_freeIsoFilter = vtkSmartPointer<vtkContourFilter>::New();

  m_FreeIsoAdded = false;
  m_selectedNode = NULL;
  m_selectedPresetName = "";
  m_internalUpdate = false;
  m_PrescribedDose_Data = 0.0;
}

RTDoseVisualizer::~RTDoseVisualizer()
{
  delete m_LevelSetModel;
  delete m_DoseColorDelegate;
  delete m_DoseValueDelegate;
  delete m_DoseVisualDelegate;
}

void RTDoseVisualizer::SetFocus(){}

void RTDoseVisualizer::OnSliceChanged(itk::Object *sender, const itk::EventObject &e)
{
  for(int i=0; i<m_StdIsoLines.size();++i)
  {
    GetDataStorage()->Remove(m_StdIsoLines.at(i));
  }
  m_StdIsoLines.clear();
  this->UpdateStdIsolines();

  if(m_FreeIsoAdded)
  {
    float pref;
    //get the iso dose node
    mitk::DataNode::Pointer isoNode = this->GetIsoDoseNode();
    isoNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(isoNode->GetData());
    mitk::Image::Pointer slicedImage = this->GetExtractedSlice(image);

    m_Filters.at(0)->SetInputData(slicedImage->GetVtkImageData());
    m_Filters.at(0)->GenerateValues(1,m_FreeIsoValue->GetDoseValue()*pref,m_FreeIsoValue->GetDoseValue()*pref);
    m_Filters.at(0)->Update();

    m_FreeIsoline->GetData()->GetGeometry()->SetOrigin(slicedImage->GetGeometry()->GetOrigin());

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

mitk::DataNode::Pointer RTDoseVisualizer::GetIsoDoseNode()
{
  //holt zuerst alle isonodes prüft dann auf visibility und nimmt zuletzt den mit dem höchsten layer
  mitk::NodePredicateProperty::Pointer isDosePredicate = mitk::NodePredicateProperty::New(mitk::RTConstants::DOSE_PROPERTY_NAME.c_str(),mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer allIsoDoseNodes = this->GetDataStorage()->GetSubset(isDosePredicate);
  int tmp = -1;
  int layer = -1;
  mitk::DataNode::Pointer isoNode = mitk::DataNode::New();
  for(mitk::DataStorage::SetOfObjects::ConstIterator itIsoDose = allIsoDoseNodes->Begin(); itIsoDose != allIsoDoseNodes->End(); ++itIsoDose)
  {
    bool isVisible(false);
    itIsoDose.Value()->GetBoolProperty("visible",isVisible);
    if(isVisible)
    {
      if(itIsoDose.Value()->GetIntProperty("layer",tmp) && tmp > layer)
      {
        isoNode = itIsoDose.Value();
      }
    }
  }
  if(isoNode.IsNotNull())
    return isoNode;
  else
    return NULL;
}

void RTDoseVisualizer::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_LevelSetModel = new QmitkIsoDoseLevelSetModel(this);
  m_LevelSetModel->setVisibilityEditOnly(true);
  m_DoseColorDelegate = new QmitkDoseColorDelegate(this);
  m_DoseValueDelegate = new QmitkDoseValueDelegate(this);
  m_DoseVisualDelegate = new QmitkDoseVisualStyleDelegate(this);

  this->UpdateByPreferences();
  this->ActualizeIsoLevelsForAllDoseDataNodes();
  this->ActualizeReferenceDoseForAllDoseDataNodes();
  this->ActualizeDisplayStyleForAllDoseDataNodes();

  this->m_Controls.isoLevelSetView->setModel(m_LevelSetModel);
  this->m_Controls.isoLevelSetView->setItemDelegateForColumn(0,m_DoseColorDelegate);
  this->m_Controls.isoLevelSetView->setItemDelegateForColumn(1,m_DoseValueDelegate);
  this->m_Controls.isoLevelSetView->setItemDelegateForColumn(2,m_DoseVisualDelegate);
  this->m_Controls.isoLevelSetView->setItemDelegateForColumn(3,m_DoseVisualDelegate);
  this->m_Controls.isoLevelSetView->setContextMenuPolicy(Qt::CustomContextMenu);

  this->m_Controls.btnRemoveFreeValue->setDisabled(true);

  connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), this, SLOT(OnReferenceDoseChanged(double)));
  connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), m_LevelSetModel, SLOT(setReferenceDose(double)));
  connect(m_Controls.radioAbsDose, SIGNAL(toggled(bool)), m_LevelSetModel, SLOT(setShowAbsoluteDose(bool)));
  connect(m_Controls.radioAbsDose, SIGNAL(toggled(bool)), this, SLOT(OnAbsDoseToggled(bool)));
  connect(m_Controls.btnAddFreeValue, SIGNAL(clicked()), this, SLOT(OnAddFreeValueClicked()));
  connect(m_Controls.btnRemoveFreeValue, SIGNAL(clicked()), this, SLOT(OnRemoveFreeValueClicked()));
  connect(m_Controls.checkGlobalVisColorWash, SIGNAL(toggled(bool)), this, SLOT(OnGlobalVisColorWashToggled(bool)));
  connect(m_Controls.checkGlobalVisIsoLine, SIGNAL(toggled(bool)), this, SLOT(OnGlobalVisIsoLineToggled(bool)));
  connect(m_Controls.isoLevelSetView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowContextMenuIsoSet(const QPoint&)));
  connect(m_Controls.comboPresets, SIGNAL(currentIndexChanged ( const QString&)), this, SLOT(OnCurrentPresetChanged(const QString&)));
  connect(m_Controls.btnUsePrescribedDose, SIGNAL(clicked()), this, SLOT(OnUsePrescribedDoseClicked()));
  connect(m_Controls.isoLevelSetView->model(), SIGNAL( modelReset()), this, SLOT(OnDataChangedInIsoLevelSetView()));

  ctkServiceReference ref = mitk::org_mitk_gui_qt_dosevisualization_Activator::GetContext()->getServiceReference<ctkEventAdmin>();

  ctkDictionary propsForSlot;
  if (ref)
  {
    ctkEventAdmin* eventAdmin = mitk::org_mitk_gui_qt_dosevisualization_Activator::GetContext()->getService<ctkEventAdmin>(ref);

    propsForSlot[ctkEventConstants::EVENT_TOPIC] = mitk::RTCTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED.c_str();
    eventAdmin->subscribeSlot(this, SLOT(OnHandleCTKEventPresetsChanged(ctkEvent)), propsForSlot);

    propsForSlot[ctkEventConstants::EVENT_TOPIC] = mitk::RTCTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED.c_str();
    eventAdmin->subscribeSlot(this, SLOT(OnHandleCTKEventReferenceDoseChanged(ctkEvent)), propsForSlot);
  }

  this->UpdateBySelectedNode();
}

void RTDoseVisualizer::OnReferenceDoseChanged(double value)
{
  if (!  m_internalUpdate)
  {
    mitk::DoseValueAbs referenceDose = 0.0;
    bool globalSync = mitk::GetReferenceDoseValue(referenceDose);

    if (globalSync)
    {
      mitk::SetReferenceDoseValue(globalSync, value);
      this->ActualizeReferenceDoseForAllDoseDataNodes();
    }
    else
    {
      if (this->m_selectedNode.IsNotNull())
      {
        this->m_selectedNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
        //ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring
        mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(m_selectedNode);
        mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();

        while (iterChildNodes != childNodes->end())
        {
          (*iterChildNodes)->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
          ++iterChildNodes;
        }
      }
    }
    if (this->m_selectedNode.IsNotNull())
    {
      this->UpdateColorWashTransferFunction();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void RTDoseVisualizer::OnAddFreeValueClicked()
{
  QColor newColor;
  //Use HSV schema of QColor to calculate a different color depending on the
  //number of already existing free iso lines.
  newColor.setHsv((m_freeIsoValues->Size()*85)%360,255,255);
  mitk::Color mColor;
  mColor[0]=newColor.redF();
  mColor[1]=newColor.greenF();
  mColor[2]=newColor.blueF();

  mitk::DataNode::Pointer isoNode = this->UpdatePolyData(1,m_Controls.spinReferenceDose->value()*0.5,m_Controls.spinReferenceDose->value()*0.5);
  isoNode->SetColor(mColor);
  m_FreeIsoLines.push_back(isoNode);

  mitk::IsoDoseLevel::ColorType color;
  color[0] = newColor.redF();
  color[1] = newColor.greenF();
  color[2] = newColor.blueF();
  m_freeIsoValues->push_back(mitk::IsoDoseLevel::New(0.5,color,true,false));
  m_FreeIsoValue = mitk::IsoDoseLevel::New(0.5,color,true,false);
  UpdateFreeIsoValues();
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  if(m_FreeIsoLines.size()>=1)
  {
    this->m_Controls.btnAddFreeValue->setDisabled(true);
    m_FreeIsoAdded = true;
  }
  this->m_Controls.btnRemoveFreeValue->setEnabled(true);
}

void RTDoseVisualizer::OnRemoveFreeValueClicked()
{
  m_freeIsoValues->pop_back();
  mitk::DataNode::Pointer isoNode = m_FreeIsoLines.at(m_FreeIsoLines.size()-1);
  m_FreeIsoLines.pop_back();
  m_Filters.pop_back();
  if(m_FreeIsoLines.empty())
    this->m_Controls.btnRemoveFreeValue->setDisabled(true);
  if(m_FreeIsoLines.size()<1)
  {
    m_FreeIsoAdded = false;
    this->m_Controls.btnAddFreeValue->setEnabled(true);
  }
  this->GetDataStorage()->Remove(isoNode);
  UpdateFreeIsoValues();
}

void RTDoseVisualizer::OnUsePrescribedDoseClicked()
{
  m_Controls.spinReferenceDose->setValue(this->m_PrescribedDose_Data);
}

void RTDoseVisualizer::OnDataChangedInIsoLevelSetView()
{
  //colorwash visibility changed, update the colorwash
  this->UpdateColorWashTransferFunction();

  //Hack: This is a dirty hack to reinit the isodose contour node. Only if the node (or property) has changed the rendering process register the RequestUpdateAll
  //Only way to render the isoline by changes without a global reinit
  mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(m_selectedNode);
  mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();
  while (iterChildNodes != childNodes->end())
  {
    (*iterChildNodes)->Modified();
    ++iterChildNodes;
  }

  // Reinit if visibility of colorwash or isodoselevel changed
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void RTDoseVisualizer::OnShowContextMenuIsoSet(const QPoint& pos)
{
  QPoint globalPos = m_Controls.isoLevelSetView->viewport()->mapToGlobal(pos);

  QMenu viewMenu;
  QAction* invertIsoLineAct = viewMenu.addAction("Invert iso line visibility");
  QAction* activateIsoLineAct = viewMenu.addAction("Activate all iso lines");
  QAction* deactivateIsoLineAct = viewMenu.addAction("Deactivate all iso lines");
  viewMenu.addSeparator();
  QAction* invertColorWashAct = viewMenu.addAction("Invert color wash visibility");
  QAction* activateColorWashAct = viewMenu.addAction("Activate all color wash levels");
  QAction* deactivateColorWashAct = viewMenu.addAction("Deactivate all color wash levels");
  viewMenu.addSeparator();
  QAction* swapAct = viewMenu.addAction("Swap iso line/color wash visibility");
  // ...

  QAction* selectedItem = viewMenu.exec(globalPos);
  if (selectedItem == invertIsoLineAct)
  {
    this->m_LevelSetModel->invertVisibilityIsoLines();
  }
  else if (selectedItem == activateIsoLineAct)
  {
    this->m_LevelSetModel->switchVisibilityIsoLines(true);
  }
  else if (selectedItem == deactivateIsoLineAct)
  {
    this->m_LevelSetModel->switchVisibilityIsoLines(false);
  }
  else if (selectedItem == invertColorWashAct)
  {
    this->m_LevelSetModel->invertVisibilityColorWash();
  }
  else if (selectedItem == activateColorWashAct)
  {
    this->m_LevelSetModel->switchVisibilityColorWash(true);
  }
  else if (selectedItem == deactivateColorWashAct)
  {
    this->m_LevelSetModel->switchVisibilityColorWash(false);
  }
  else if (selectedItem == swapAct)
  {
    this->m_LevelSetModel->swapVisibility();
  }
}

void RTDoseVisualizer::UpdateFreeIsoValues()
{
  this->m_Controls.listFreeValues->clear();

  for (mitk::IsoDoseLevelVector::Iterator pos = this->m_freeIsoValues->Begin(); pos != this->m_freeIsoValues->End(); ++pos)
  {
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(0,25));
    QmitkFreeIsoDoseLevelWidget* widget = new QmitkFreeIsoDoseLevelWidget;

    float pref;
    m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);

    widget->setIsoDoseLevel(pos->Value().GetPointer());
    widget->setReferenceDose(pref);
    connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), widget, SLOT(setReferenceDose(double)));
    connect(widget,SIGNAL(ColorChanged(mitk::IsoDoseLevel*)), this, SLOT(UpdateFreeIsoLineColor(mitk::IsoDoseLevel*)));
    connect(widget,SIGNAL(ValueChanged(mitk::IsoDoseLevel*,mitk::DoseValueRel)), this, SLOT(UpdateFreeIsoLine(mitk::IsoDoseLevel*,mitk::DoseValueRel)));

    this->m_Controls.listFreeValues->addItem(item);
    this->m_Controls.listFreeValues->setItemWidget(item,widget);
  }
}

void RTDoseVisualizer::UpdateFreeIsoLineColor(mitk::IsoDoseLevel *level)
{
  //push it in and get the key!
  //  m_freeIsoValues
  ::itk::RGBPixel<float> color = level->GetColor();
  mitk::Color mColor;
  mColor[0]=color.GetRed();
  mColor[1]=color.GetGreen();
  mColor[2]=color.GetBlue();
  m_FreeIsoline->SetColor(mColor);
}

void RTDoseVisualizer::UpdateFreeIsoLine(mitk::IsoDoseLevel * level, mitk::DoseValueRel old)
{
  float pref;
  m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
  mitk::Image::Pointer slicedImage = this->GetExtractedSlice(image);

  m_FreeIsoValue = level;
  //  m_freeIsoValues->at(0) = level;

  m_Filters.at(0)->SetInputData(slicedImage->GetVtkImageData());
  m_Filters.at(0)->GenerateValues(1,level->GetDoseValue()*pref,level->GetDoseValue()*pref);
  m_Filters.at(0)->Update();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void RTDoseVisualizer::OnAbsDoseToggled(bool showAbs)
{
  if (!  m_internalUpdate)
  {
    mitk::SetDoseDisplayAbsolute(showAbs);
    this->ActualizeDisplayStyleForAllDoseDataNodes();
  }
}

void RTDoseVisualizer::OnGlobalVisColorWashToggled(bool showColorWash)
{
  if (m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), showColorWash);
    //The rendering mode could be set in the dose mapper: Refactoring!
    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    if(showColorWash)
      renderingMode->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);
    else
      renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
    m_selectedNode->SetProperty("Image Rendering.Mode", renderingMode);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void RTDoseVisualizer::OnGlobalVisIsoLineToggled(bool showIsoLines)
{
  if (m_selectedNode.IsNotNull())
  {
    //ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring
    mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(m_selectedNode);
    mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();

    while (iterChildNodes != childNodes->end())
    {
      (*iterChildNodes)->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsoLines);
      ++iterChildNodes;
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

const mitk::Geometry2D* RTDoseVisualizer::GetGeometry2D(char* dim)
{
  QmitkRenderWindow* rw = this->GetRenderWindowPart()->GetQmitkRenderWindow(dim);
  const mitk::Geometry2D* worldGeo = rw->GetRenderer()->GetCurrentWorldGeometry2D();
  return worldGeo;
}

mitk::Image::Pointer RTDoseVisualizer::GetExtractedSlice(mitk::Image::Pointer image)
{
  mitk::ExtractSliceFilter::Pointer extractFilter = mitk::ExtractSliceFilter::New();
  extractFilter->SetInput(image);
  extractFilter->SetWorldGeometry(this->GetGeometry2D("axial"));
  extractFilter->SetResliceTransformByGeometry( image->GetTimeGeometry()->GetGeometryForTimeStep(0) );
  extractFilter->Update();
  mitk::Image::Pointer reslicedImage = extractFilter->GetOutput();
  return reslicedImage;
}

mitk::DataNode::Pointer RTDoseVisualizer::UpdatePolyData(int num, double min, double max)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
  mitk::Image::Pointer reslicedImage = this->GetExtractedSlice(image);

  vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
  m_Filters.push_back(contourFilter);
  contourFilter->SetInputData(reslicedImage->GetVtkImageData());
  contourFilter->GenerateValues(num,min,max);
  contourFilter->Update();
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData =contourFilter->GetOutput();

  mitk::Surface::Pointer isoline = mitk::Surface::New();
  isoline->SetVtkPolyData(polyData);
  //  isoline->SetGeometry(reslicedImage->GetGeometry()->Clone());
  isoline->SetGeometry(reslicedImage->GetGeometry());
  mitk::Vector3D spacing;
  spacing.Fill(1);
  isoline->GetGeometry()->SetSpacing(spacing);

  mitk::DataNode::Pointer isolineNode = mitk::DataNode::New();
  isolineNode->SetData(isoline);
  mitk::SurfaceVtkMapper3D::Pointer mapper = mitk::SurfaceVtkMapper3D::New();
  isolineNode->SetMapper(1, mapper);
  isolineNode->SetName("Isoline1");
  isolineNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
  isolineNode->SetProperty( "line width", mitk::IntProperty::New(1));
  isolineNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
  isolineNode->SetBoolProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),true);
  m_FreeIsoline = isolineNode;
  this->GetDataStorage()->Add(isolineNode);
  return isolineNode;
}

void RTDoseVisualizer::UpdateColorWashTransferFunction()
{
  //Generating the Colorwash
  vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

  mitk::IsoDoseLevelSetProperty::Pointer propIsoSet = dynamic_cast<mitk::IsoDoseLevelSetProperty* >(m_selectedNode->GetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str()));
  mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = propIsoSet->GetValue();

  float referenceDose;
  m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),referenceDose);
  mitk::TransferFunction::ControlPoints scalarOpacityPoints;
  scalarOpacityPoints.push_back( std::make_pair(0, 1 ) );
  //Backgroud
  transferFunction->AddHSVPoint(((isoDoseLevelSet->Begin())->GetDoseValue()*referenceDose)-0.001,0,0,0,1.0,1.0);

  for(mitk::IsoDoseLevelSet::ConstIterator itIsoDoseLevel = isoDoseLevelSet->Begin(); itIsoDoseLevel != isoDoseLevelSet->End(); ++itIsoDoseLevel)
  {
    float *hsv = new float[3];
    //used for transfer rgb to hsv
    vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();

    if(itIsoDoseLevel->GetVisibleColorWash())
    {
      cCalc->RGBToHSV(itIsoDoseLevel->GetColor()[0],itIsoDoseLevel->GetColor()[1],itIsoDoseLevel->GetColor()[2],&hsv[0],&hsv[1],&hsv[2]);
      transferFunction->AddHSVPoint(itIsoDoseLevel->GetDoseValue()*referenceDose,hsv[0],hsv[1],hsv[2],1.0,1.0);
    }
    else
    {
      scalarOpacityPoints.push_back( std::make_pair(itIsoDoseLevel->GetDoseValue()*referenceDose, 1 ) );
    }
  }

  mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
  mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
  mitkTransFunc->SetColorTransferFunction(transferFunction);
  mitkTransFunc->SetScalarOpacityPoints(scalarOpacityPoints);
  mitkTransFuncProp->SetValue(mitkTransFunc);
  m_selectedNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
}

void RTDoseVisualizer::UpdateStdIsolines()
{
  bool isDoseNode = false;
  mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = this->m_Presets[this->m_selectedPresetName];
  mitk::DataNode::Pointer isoDataNode = this->GetIsoDoseNode();
  if(isoDataNode && isoDataNode->GetBoolProperty(mitk::RTConstants::DOSE_PROPERTY_NAME.c_str(),isDoseNode) && isDoseNode)
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(isoDataNode->GetData());
    mitk::Image::Pointer reslicedImage = this->GetExtractedSlice(image);

    reslicedImage->SetSpacing(image->GetGeometry()->GetSpacing());

    float pref;
    isoDataNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);

    unsigned int count (0);
    for(mitk::IsoDoseLevelSet::ConstIterator doseIT = isoDoseLevelSet->Begin(); doseIT!=isoDoseLevelSet->End();++doseIT)
    {
      if(doseIT->GetVisibleIsoLine()){
        ++count;
        vtkSmartPointer<vtkContourFilter> isolineFilter = vtkSmartPointer<vtkContourFilter>::New();
        isolineFilter->SetInputData(reslicedImage->GetVtkImageData());
        isolineFilter->GenerateValues(1,doseIT->GetDoseValue()*pref,doseIT->GetDoseValue()*pref);
        isolineFilter->Update();
        vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
        polyData=isolineFilter->GetOutput();

        mitk::Surface::Pointer surface = mitk::Surface::New();

        surface->SetVtkPolyData(polyData);
        //        surface->SetGeometry(reslicedImage->GetGeometry()->Clone());
        surface->SetGeometry(reslicedImage->GetGeometry());
        mitk::Vector3D spacing;
        spacing.Fill(1);
        surface->GetGeometry()->SetSpacing(spacing);

        mitk::DataNode::Pointer isoNode = mitk::DataNode::New();
        isoNode->SetData(surface);
        mitk::SurfaceVtkMapper3D::Pointer mapper = mitk::SurfaceVtkMapper3D::New();
        mitk::Color color;
        color[0]=doseIT->GetColor()[0];color[1]=doseIT->GetColor()[1];color[2]=doseIT->GetColor()[2];
        isoNode->SetMapper(1,mapper);
        isoNode->SetColor(color);
        isoNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
        isoNode->SetProperty( "includeInBoundingBox", mitk::BoolProperty::New(false) );
        isoNode->SetProperty( "line width", mitk::IntProperty::New(1));
        std::stringstream strstr;
        strstr<<"StdIsoline_";
        strstr<<count;
        isoNode->SetName(strstr.str());
        isoNode->SetBoolProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),true);
        m_StdIsoLines.push_back(isoNode);
        this->GetDataStorage()->Add(isoNode, isoDataNode);
      }
    }
  }
}

void RTDoseVisualizer::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                          const QList<mitk::DataNode::Pointer>& nodes )
{
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();

  mitk::DataNode* selectedNode = NULL;

  if (!dataNodes.empty())
  {
    bool isDoseNode = false;
    dataNodes[0]->GetBoolProperty(mitk::RTConstants::DOSE_PROPERTY_NAME.c_str(),isDoseNode);

    if (isDoseNode)
    {
      selectedNode = dataNodes[0];
    }
  }

  if (selectedNode != m_selectedNode.GetPointer())
  {
    m_selectedNode = selectedNode;
  }

  UpdateBySelectedNode();
}

void RTDoseVisualizer::UpdateBySelectedNode()
{
  m_Controls.groupNodeSpecific->setEnabled(m_selectedNode.IsNotNull());
  m_Controls.groupFreeValues->setEnabled(m_selectedNode.IsNotNull());
  m_Controls.checkGlobalVisColorWash->setEnabled(m_selectedNode.IsNotNull());
  m_Controls.checkGlobalVisIsoLine->setEnabled(m_selectedNode.IsNotNull());
  m_Controls.isoLevelSetView->setEnabled(m_selectedNode.IsNotNull());


  if(m_selectedNode.IsNull())
  {
    m_Controls.NrOfFractions->setText(QString("N/A. No dose selected"));
    m_Controls.prescribedDoseSpecific->setText(QString("N/A. No dose selected"));
    m_freeIsoValues = mitk::IsoDoseLevelVector::New();
    UpdateFreeIsoValues();
  }
  else
  {
    //dose specific information
    int fracCount = 1;
    m_selectedNode->GetIntProperty(mitk::RTConstants::DOSE_FRACTION_COUNT_PROPERTY_NAME.c_str(),fracCount);
    m_Controls.NrOfFractions->setText(QString::number(fracCount));

    m_PrescribedDose_Data = 0.0;

    float tmp;
    m_selectedNode->GetFloatProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(),tmp);
    m_PrescribedDose_Data = (double)tmp;

    m_Controls.prescribedDoseSpecific->setText(QString::number(m_PrescribedDose_Data));

    //free iso lines
    mitk::IsoDoseLevelVectorProperty::Pointer propIsoVector;
    m_selectedNode->GetProperty<mitk::IsoDoseLevelVectorProperty>(propIsoVector, mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str());

    if (propIsoVector.IsNull())
    {
      m_freeIsoValues = mitk::IsoDoseLevelVector::New();
      propIsoVector = mitk::IsoDoseLevelVectorProperty::New(m_freeIsoValues);
      m_selectedNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),propIsoVector);
    }
    else
    {
      m_freeIsoValues = propIsoVector->GetValue();
    }

    UpdateFreeIsoValues();

    //global dose issues
    //ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring
    bool showIsoLine = false;
    mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(m_selectedNode);
    mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();

    while (iterChildNodes != childNodes->end())
    {
      (*iterChildNodes)->GetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(),showIsoLine);
      if(showIsoLine)
        break;
      ++iterChildNodes;
    }
    m_Controls.checkGlobalVisIsoLine->setChecked(showIsoLine);

    bool showColorWash = false;
    m_selectedNode->GetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(),showColorWash);
    m_Controls.checkGlobalVisColorWash->setChecked(showColorWash);

    float referenceDose = 0.0;
    m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),referenceDose);
    m_Controls.spinReferenceDose->setValue(referenceDose);

    mitk::IsoDoseLevelSetProperty::Pointer propIsoSet =
      dynamic_cast<mitk::IsoDoseLevelSetProperty* >(m_selectedNode->GetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str()));

    if (propIsoSet)
    {
      this->m_selectedNodeIsoSet = propIsoSet->GetValue();
      this->m_LevelSetModel->setIsoDoseLevelSet(m_selectedNodeIsoSet);
    }
  }
}

void RTDoseVisualizer::UpdateByPreferences()
{
  m_Presets = mitk::LoadPresetsMap();
  m_internalUpdate = true;
  m_Controls.comboPresets->clear();
  this->m_selectedPresetName = mitk::GetSelectedPresetName();

  int index = 0;
  int selectedIndex = -1;
  for (mitk::PresetMapType::const_iterator pos = m_Presets.begin(); pos != m_Presets.end(); ++pos, ++index)
  {
    m_Controls.comboPresets->addItem(QString(pos->first.c_str()));
    if (this->m_selectedPresetName == pos->first)
    {
      selectedIndex = index;
    }
  }

  if (selectedIndex == -1)
  {
    selectedIndex = 0;
    MITK_WARN << "Error. Cannot iso dose level preset specified in preferences does not exist. Preset name: "<<this->m_selectedPresetName;
    this->m_selectedPresetName = m_Presets.begin()->first;
    mitk::SetSelectedPresetName(this->m_selectedPresetName);
    MITK_INFO << "Changed selected iso dose level preset to first existing preset. New preset name: "<<this->m_selectedPresetName;
  }

  m_Controls.comboPresets->setCurrentIndex(selectedIndex);

  this->m_selectedNodeIsoSet = this->m_Presets[this->m_selectedPresetName];
  this->m_LevelSetModel->setIsoDoseLevelSet(m_selectedNodeIsoSet);

  mitk::DoseValueAbs referenceDose = 0.0;
  bool globalSync = mitk::GetReferenceDoseValue(referenceDose);
  if (globalSync || this->m_selectedNode.IsNull())
  {
    m_Controls.spinReferenceDose->setValue(referenceDose);
  }

  bool displayAbsoluteDose = mitk::GetDoseDisplayAbsolute();
  m_Controls.radioAbsDose->setChecked(displayAbsoluteDose);
  m_Controls.radioRelDose->setChecked(!displayAbsoluteDose);
  this->m_LevelSetModel->setShowAbsoluteDose(displayAbsoluteDose);
  m_internalUpdate = false;
}

void RTDoseVisualizer::OnCurrentPresetChanged(const QString& presetName)
{
  if (!  m_internalUpdate)
  {
    mitk::SetSelectedPresetName(presetName.toStdString());
    this->UpdateByPreferences();
    this->ActualizeIsoLevelsForAllDoseDataNodes();
    this->UpdateBySelectedNode();
  }
}

void RTDoseVisualizer::ActualizeIsoLevelsForAllDoseDataNodes()
{
  std::string presetName = mitk::GetSelectedPresetName();

  mitk::PresetMapType presetMap = mitk::LoadPresetsMap();

  mitk::NodePredicateProperty::Pointer isDoseNode = mitk::NodePredicateProperty::New(mitk::RTConstants::DOSE_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetSubset(isDoseNode);

  mitk::IsoDoseLevelSet* selectedPreset = presetMap[presetName];

  if (!selectedPreset)
  {
    mitkThrow() << "Error. Cannot actualize iso dose level preset. Selected preset idoes not exist. Preset name: "<<presetName;
  }

  for(mitk::DataStorage::SetOfObjects::const_iterator pos = nodes->begin(); pos != nodes->end(); ++pos)
  {
    mitk::IsoDoseLevelSet::Pointer clonedPreset = selectedPreset->Clone();
    mitk::IsoDoseLevelSetProperty::Pointer propIsoSet = mitk::IsoDoseLevelSetProperty::New(clonedPreset);
    (*pos)->SetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),propIsoSet);
  }
}

void RTDoseVisualizer::ActualizeReferenceDoseForAllDoseDataNodes()
{
  /** @TODO Klären ob diese präsentations info genauso wie*/
  mitk::DoseValueAbs value = 0;
  bool sync = mitk::GetReferenceDoseValue(value);

  if (sync)
  {
    mitk::NodePredicateProperty::Pointer isDoseNode = mitk::NodePredicateProperty::New(mitk::RTConstants::DOSE_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));

    mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetSubset(isDoseNode);

    for(mitk::DataStorage::SetOfObjects::const_iterator pos = nodes->begin(); pos != nodes->end(); ++pos)
    {
      (*pos)->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
      //ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring
      mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(*pos);
      mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();

      while (iterChildNodes != childNodes->end())
      {
        (*iterChildNodes)->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
        ++iterChildNodes;
      }
    }
  }
}

void RTDoseVisualizer::ActualizeDisplayStyleForAllDoseDataNodes()
{
  /** @TODO Klären ob diese präsentations info global oder auch per node gespeichert wird*/
}

void RTDoseVisualizer::OnHandleCTKEventReferenceDoseChanged(const ctkEvent& event)
{
  mitk::DoseValueAbs referenceDose = 0.0;
  bool globalSync = mitk::GetReferenceDoseValue(referenceDose);

  this->m_Controls.spinReferenceDose->setValue(referenceDose);
}

void RTDoseVisualizer::OnHandleCTKEventPresetsChanged(const ctkEvent& event)
{
  std::string currentPresetName  = mitk::GetSelectedPresetName();

  this->OnCurrentPresetChanged(QString::fromStdString(currentPresetName));
}
