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

#include "org_mitk_gui_qt_rt_dosevisualization_Activator.h"

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <vtkMath.h>

#include <vtkMarchingSquares.h>
#include <mitkSurface.h>
#include "QmitkRenderWindow.h"
#include <mitkExtractSliceFilter.h>

#include "mitkSurfaceGLMapper2D.h"
#include "mitkPolyDataGLMapper2D.h"
#include "mitkSurfaceVtkMapper3D.h"

#include "mitkSliceNavigationController.h"

const std::string RTDoseVisualizer::VIEW_ID = "org.mitk.views.rt.dosevisualization";

RTDoseVisualizer::RTDoseVisualizer()
{
    m_freeIsoValues = mitk::IsoDoseLevelVector::New();
    m_selectedNodeIsoSet = mitk::IsoDoseLevelSet::New();

    m_freeIsoFilter = vtkSmartPointer<vtkContourFilter>::New();

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

void RTDoseVisualizer::InitScrolling(){
  QmitkRenderWindow* rw = this->GetRenderWindowPart()->GetQmitkRenderWindow("axial");

  itk::MemberCommand<RTDoseVisualizer>::Pointer sliceChangedCommand =
      itk::MemberCommand<RTDoseVisualizer>::New();
  sliceChangedCommand->SetCallbackFunction(this, &RTDoseVisualizer::OnSliceChanged);
  rw->GetSliceNavigationController()->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL,0), sliceChangedCommand);
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

  connect(m_Controls.btnConvert, SIGNAL(clicked()), this, SLOT(OnConvertButtonClicked()));
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

  ctkServiceReference ref = mitk::org_mitk_gui_qt_rt_dosevisualization_Activator::GetContext()->getServiceReference<ctkEventAdmin>();

  ctkDictionary propsForSlot;
  if (ref)
  {
    ctkEventAdmin* eventAdmin = mitk::org_mitk_gui_qt_rt_dosevisualization_Activator::GetContext()->getService<ctkEventAdmin>(ref);

    propsForSlot[ctkEventConstants::EVENT_TOPIC] = mitk::rt::CTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED.c_str();
    eventAdmin->subscribeSlot(this, SLOT(OnHandleCTKEventPresetsChanged(ctkEvent)), propsForSlot);

    propsForSlot[ctkEventConstants::EVENT_TOPIC] = mitk::rt::CTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED.c_str();
    eventAdmin->subscribeSlot(this, SLOT(OnHandleCTKEventReferenceDoseChanged(ctkEvent)), propsForSlot);
  }

  this->UpdateBySelectedNode();
}

void RTDoseVisualizer::OnReferenceDoseChanged(double value)
{
  if (!  m_internalUpdate)
  {
    mitk::DoseValueAbs referenceDose = 0.0;
    bool globalSync = mitk::rt::GetReferenceDoseValue(referenceDose);

    if (globalSync)
    {
      mitk::rt::SetReferenceDoseValue(globalSync, value);
      this->ActualizeReferenceDoseForAllDoseDataNodes();
    }
    else
    {
      if (this->m_selectedNode.IsNotNull())
      {
        this->m_selectedNode->SetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
      }
    }
    if (this->m_selectedNode.IsNotNull())
    {
      mitk::TransferFunction::ControlPoints scalarOpacityPoints;
      scalarOpacityPoints.push_back( std::make_pair(0, 1 ) );
      vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
      mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = this->m_Presets[this->m_selectedPresetName];
      for(mitk::IsoDoseLevelSet::ConstIterator setIT = isoDoseLevelSet->Begin(); setIT != isoDoseLevelSet->End(); ++setIT)
      {
        float *hsv = new float[3];
        vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
        if(setIT->GetVisibleColorWash()){
          cCalc->RGBToHSV(setIT->GetColor()[0],setIT->GetColor()[1],setIT->GetColor()[2],&hsv[0],&hsv[1],&hsv[2]);
          transferFunction->AddHSVPoint(setIT->GetDoseValue()*value,hsv[0],hsv[1],hsv[2],1.0,1.0);
        }
        else
        {
          scalarOpacityPoints.push_back( std::make_pair(setIT->GetDoseValue()*value, 1 ) );
        }
      }
      mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
      mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
      mitkTransFunc->SetColorTransferFunction(transferFunction);
      mitkTransFunc->SetScalarOpacityPoints(scalarOpacityPoints);
      mitkTransFuncProp->SetValue(mitkTransFunc);

      mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
      renderingMode->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

      m_selectedNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
      m_selectedNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

      mitk::TimeSlicedGeometry::Pointer geo3 = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
      mitk::RenderingManager::GetInstance()->InitializeViews( geo3 );
    }
  }
}

void RTDoseVisualizer::OnAddFreeValueClicked()
{
  QColor newColor;
  //Use HSV schema of QColor to calculate a different color depending on the
  //number of already existing free iso lines.
  newColor.setHsv((m_freeIsoValues->Size()*85)%360,255,255);

  mitk::DataNode::Pointer isoNode = this->UpdatePolyData(1,m_Controls.spinReferenceDose->value()*0.5,m_Controls.spinReferenceDose->value()*0.5);
  m_FreeIsoLines.push_back(isoNode);

  mitk::IsoDoseLevel::ColorType color;
  color[0] = newColor.redF();
  color[1] = newColor.greenF();
  color[2] = newColor.blueF();
  m_freeIsoValues->push_back(mitk::IsoDoseLevel::New(0.5,color,true,false));
  UpdateFreeIsoValues();
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  if(m_FreeIsoLines.size()>=3)
    this->m_Controls.btnAddFreeValue->setDisabled(true);
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
  if(m_FreeIsoLines.size()<3)
    this->m_Controls.btnAddFreeValue->setEnabled(true);
  this->GetDataStorage()->Remove(isoNode);
  UpdateFreeIsoValues();
}

void RTDoseVisualizer::OnUsePrescribedDoseClicked()
{
  m_Controls.spinReferenceDose->setValue(this->m_PrescribedDose_Data);
};


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
    m_selectedNode->GetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);

    widget->setIsoDoseLevel(pos->Value().GetPointer());
    widget->setReferenceDose(pref);
    connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), widget, SLOT(setReferenceDose(double)));
    connect(widget,SIGNAL(ValueChanged(mitk::IsoDoseLevel*,mitk::DoseValueRel)), this, SLOT(UpdateFreeIsoLine(mitk::IsoDoseLevel*,mitk::DoseValueRel)));

    this->m_Controls.listFreeValues->addItem(item);
    this->m_Controls.listFreeValues->setItemWidget(item,widget);
  }
}

void RTDoseVisualizer::UpdateFreeIsoLine(mitk::IsoDoseLevel * level, mitk::DoseValueRel old)
{
  float pref;
  m_selectedNode->GetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
  mitk::Image::Pointer slicedImage = this->GetExtractedSlice(image);

  m_Filters.at(0)->SetInput(slicedImage->GetVtkImageData());
  m_Filters.at(0)->GenerateValues(1,level->GetDoseValue()*pref,level->GetDoseValue()*pref);
  m_Filters.at(0)->Update();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void RTDoseVisualizer::OnAbsDoseToggled(bool showAbs)
{
  if (!  m_internalUpdate)
  {
    mitk::rt::SetDoseDisplayAbsolute(showAbs);
    this->ActualizeDisplayStyleForAllDoseDataNodes();
  }
}

void RTDoseVisualizer::OnGlobalVisColorWashToggled(bool showColorWash)
{
  if (m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), showColorWash);
  }
}

void RTDoseVisualizer::OnGlobalVisIsoLineToggled(bool showIsoLines)
{
  if (m_selectedNode.IsNotNull())
  {
    m_selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsoLines);
    mitk::NodePredicateProperty::Pointer isoProp = mitk::NodePredicateProperty::
        New(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));
    mitk::DataStorage::SetOfObjects::ConstPointer isoSet =  this->GetDataStorage()->GetSubset(isoProp);
    for(mitk::DataStorage::SetOfObjects::ConstIterator iso = isoSet->Begin(); iso!=isoSet->End(); ++iso)
    {
      mitk::DataNode::Pointer node = iso.Value();
      node->SetVisibility(showIsoLines);
    }
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}


void RTDoseVisualizer::OnConvertButtonClicked()
{
  this->InitScrolling();

  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();

  mitk::DataNode* selectedNode = NULL;

  if (!dataNodes.empty())
  {
    selectedNode = dataNodes[0];
  }

  if(selectedNode)
  {
    selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(), true);
    selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(), true);
    selectedNode->SetBoolProperty(mitk::rt::Constants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), true);
    selectedNode->SetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), m_Controls.spinReferenceDose->value());

    mitk::IsoDoseLevelSet::Pointer clonedPreset = this->m_Presets[this->m_selectedPresetName]->Clone();
    mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(clonedPreset);
    selectedNode->SetProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);


    double hsvValue = 0.002778;
    float prescribed;
    m_selectedNode->GetFloatProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(),prescribed);

    vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

    mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = this->m_Presets[this->m_selectedPresetName];

    MITK_INFO << "FUNCTION PRESCRIBE " << prescribed << endl;


    float pref;
    m_selectedNode->GetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
    mitk::Image::Pointer reslicedImage = this->GetExtractedSlice(image);

    //Generating the Colorwash
    for(mitk::IsoDoseLevelSet::ConstIterator setIT = isoDoseLevelSet->Begin(); setIT != isoDoseLevelSet->End(); ++setIT)
    {
      float *hsv = new float[3];
      //used for transfer rgb to hsv
      vtkSmartPointer<vtkMath> cCalc = vtkSmartPointer<vtkMath>::New();
      if(setIT->GetVisibleColorWash()){
        cCalc->RGBToHSV(setIT->GetColor()[0],setIT->GetColor()[1],setIT->GetColor()[2],&hsv[0],&hsv[1],&hsv[2]);
        transferFunction->AddHSVPoint(setIT->GetDoseValue()*pref,hsv[0],hsv[1],hsv[2],1.0,1.0);
      }
    }

    //Generating the standard isolines
    this->UpdateStdIsolines();

    mitk::TransferFunction::Pointer mitkTransFunc = mitk::TransferFunction::New();
    mitk::TransferFunctionProperty::Pointer mitkTransFuncProp = mitk::TransferFunctionProperty::New();
    mitkTransFunc->SetColorTransferFunction(transferFunction);
    mitkTransFuncProp->SetValue(mitkTransFunc);

    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    renderingMode->SetValue(mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR);

    selectedNode->SetProperty("Image Rendering.Transfer Function", mitkTransFuncProp);
    selectedNode->SetProperty("Image Rendering.Mode", renderingMode);
    m_selectedNode->SetProperty("opacity", mitk::FloatProperty::New(0.5));

    mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
    mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
    selectedNode->SetProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

    UpdateBySelectedNode();

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
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
  extractFilter->SetResliceTransformByGeometry( image->GetGeometry() );
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
  contourFilter->SetInput(reslicedImage->GetVtkImageData());
  contourFilter->GenerateValues(num,min,max);
  contourFilter->Update();
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData =contourFilter->GetOutput();

  mitk::Surface::Pointer isoline = mitk::Surface::New();
  isoline->SetVtkPolyData(polyData);
  isoline->SetGeometry(const_cast<mitk::Geometry2D*>(this->GetGeometry2D("axial"))->Clone());
  isoline->GetGeometry()->SetSpacing(image->GetGeometry()->GetSpacing());
  isoline->SetOrigin(reslicedImage->GetGeometry()->GetOrigin());

  mitk::DataNode::Pointer isolineNode = mitk::DataNode::New();
  isolineNode->SetData(isoline);
  mitk::SurfaceVtkMapper3D::Pointer mapper = mitk::SurfaceVtkMapper3D::New();
  isolineNode->SetMapper(1, mapper);
  isolineNode->SetName("Isoline1");
  isolineNode->SetBoolProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),true);
  this->GetDataStorage()->Add(isolineNode);
  return isolineNode;
}

void RTDoseVisualizer::UpdateStdIsolines()
{
  mitk::IsoDoseLevelSet::Pointer isoDoseLevelSet = this->m_Presets[this->m_selectedPresetName];
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_selectedNode->GetData());
  mitk::Image::Pointer reslicedImage = this->GetExtractedSlice(image);

  float pref;
  m_selectedNode->GetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),pref);

  for(mitk::IsoDoseLevelSet::ConstIterator doseIT = isoDoseLevelSet->Begin(); doseIT!=isoDoseLevelSet->End();++doseIT)
  {
    if(doseIT->GetVisibleIsoLine()){
      vtkSmartPointer<vtkContourFilter> isolineFilter = vtkSmartPointer<vtkContourFilter>::New();
      isolineFilter->SetInput(reslicedImage->GetVtkImageData());
      isolineFilter->GenerateValues(1,doseIT->GetDoseValue()*pref,doseIT->GetDoseValue()*pref);
      isolineFilter->Update();
      vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
      polyData=isolineFilter->GetOutput();

      mitk::Surface::Pointer surface = mitk::Surface::New();
      surface->SetVtkPolyData(polyData);
      surface->SetGeometry(const_cast<mitk::Geometry2D*>(this->GetGeometry2D("axial"))->Clone());
      surface->GetGeometry()->SetSpacing(image->GetGeometry()->GetSpacing());
      surface->SetOrigin(reslicedImage->GetGeometry()->GetOrigin());

      mitk::DataNode::Pointer isoNode = mitk::DataNode::New();
      isoNode->SetData(surface);
      mitk::SurfaceVtkMapper3D::Pointer mapper = mitk::SurfaceVtkMapper3D::New();
      mitk::Color color;
      color[0]=doseIT->GetColor()[0];color[1]=doseIT->GetColor()[1];color[2]=doseIT->GetColor()[2];
      isoNode->SetMapper(1,mapper);
      isoNode->SetColor(color);
      isoNode->SetProperty( "helper object", mitk::BoolProperty::New(true) );
      isoNode->SetName("StdIsoline");
      isoNode->SetBoolProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),true);
      m_StdIsoLines.push_back(isoNode);
      this->GetDataStorage()->Add(isoNode);
    }
  }
}

void RTDoseVisualizer::HideIsoline()
{
  mitk::NodePredicateProperty::Pointer isoProp = mitk::NodePredicateProperty::
      New(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));
  mitk::DataStorage::SetOfObjects::ConstPointer isoSet =  this->GetDataStorage()->GetSubset(isoProp);
  for(mitk::DataStorage::SetOfObjects::ConstIterator iso = isoSet->Begin(); iso!=isoSet->End(); ++iso)
  {
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
    dataNodes[0]->GetBoolProperty(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(),isDoseNode);

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
    m_selectedNode->GetIntProperty(mitk::rt::Constants::DOSE_FRACTION_COUNT_PROPERTY_NAME.c_str(),fracCount);
    m_Controls.NrOfFractions->setText(QString::number(fracCount));

    m_PrescribedDose_Data = 0.0;

    float tmp;
    m_selectedNode->GetFloatProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(),tmp);
    m_PrescribedDose_Data = (double)tmp;

    m_Controls.prescribedDoseSpecific->setText(QString::number(m_PrescribedDose_Data));

    //free iso lines
    mitk::IsoDoseLevelVectorProperty::Pointer propIsoVector;
    m_selectedNode->GetProperty<mitk::IsoDoseLevelVectorProperty>(propIsoVector, mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str());

    if (propIsoVector.IsNull())
    {
      m_freeIsoValues = mitk::IsoDoseLevelVector::New();
      propIsoVector = mitk::IsoDoseLevelVectorProperty::New(m_freeIsoValues);
      m_selectedNode->SetProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),propIsoVector);
    }
    else
    {
      m_freeIsoValues = propIsoVector->GetValue();
    }

    UpdateFreeIsoValues();

    //global dose issues
    bool showIsoLine = false;
    m_selectedNode->GetBoolProperty(mitk::rt::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(),showIsoLine);
    m_Controls.checkGlobalVisIsoLine->setChecked(showIsoLine);

    bool showColorWash = false;
    m_selectedNode->GetBoolProperty(mitk::rt::Constants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(),showColorWash);
    m_Controls.checkGlobalVisColorWash->setChecked(showColorWash);

    float referenceDose = 0.0;
    m_selectedNode->GetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),referenceDose);
    m_Controls.spinReferenceDose->setValue(referenceDose);

    mitk::IsoDoseLevelSetProperty::Pointer propIsoSet =
      dynamic_cast<mitk::IsoDoseLevelSetProperty* >(m_selectedNode->GetProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str()));

    if (propIsoSet)
    {
      this->m_selectedNodeIsoSet = propIsoSet->GetValue();
      this->m_LevelSetModel->setIsoDoseLevelSet(m_selectedNodeIsoSet);
    }
  }
}

void RTDoseVisualizer::UpdateByPreferences()
{
  m_Presets = mitk::rt::LoadPresetsMap();
  m_internalUpdate = true;
  m_Controls.comboPresets->clear();
  this->m_selectedPresetName = mitk::rt::GetSelectedPresetName();

  int index = 0;
  int selectedIndex = -1;
  for (mitk::rt::PresetMapType::const_iterator pos = m_Presets.begin(); pos != m_Presets.end(); ++pos, ++index)
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
    mitk::rt::SetSelectedPresetName(this->m_selectedPresetName);
    MITK_INFO << "Changed selected iso dose level preset to first existing preset. New preset name: "<<this->m_selectedPresetName;
  }

  m_Controls.comboPresets->setCurrentIndex(selectedIndex);

  this->m_selectedNodeIsoSet = this->m_Presets[this->m_selectedPresetName];
  this->m_LevelSetModel->setIsoDoseLevelSet(m_selectedNodeIsoSet);

  mitk::DoseValueAbs referenceDose = 0.0;
  bool globalSync = mitk::rt::GetReferenceDoseValue(referenceDose);
  if (globalSync || this->m_selectedNode.IsNull())
  {
    m_Controls.spinReferenceDose->setValue(referenceDose);
  }

  bool displayAbsoluteDose = mitk::rt::GetDoseDisplayAbsolute();
  m_Controls.radioAbsDose->setChecked(displayAbsoluteDose);
  m_Controls.radioRelDose->setChecked(!displayAbsoluteDose);
  this->m_LevelSetModel->setShowAbsoluteDose(displayAbsoluteDose);
  m_internalUpdate = false;
}

void RTDoseVisualizer::OnCurrentPresetChanged(const QString& presetName)
{
  if (!  m_internalUpdate)
  {
    mitk::rt::SetSelectedPresetName(presetName.toStdString());
    this->UpdateByPreferences();
    this->ActualizeIsoLevelsForAllDoseDataNodes();
    this->UpdateBySelectedNode();
  }
}

void RTDoseVisualizer::ActualizeIsoLevelsForAllDoseDataNodes()
{
  std::string presetName = mitk::rt::GetSelectedPresetName();

  mitk::rt::PresetMapType presetMap = mitk::rt::LoadPresetsMap();

  mitk::NodePredicateProperty::Pointer isDoseNode = mitk::NodePredicateProperty::New(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));

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
    (*pos)->SetProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),propIsoSet);
  }
}

void RTDoseVisualizer::ActualizeReferenceDoseForAllDoseDataNodes()
{
  /** @TODO Klären ob diese präsentations info genauso wie*/
  mitk::DoseValueAbs value = 0;
  bool sync = mitk::rt::GetReferenceDoseValue(value);

  if (sync)
  {
    mitk::NodePredicateProperty::Pointer isDoseNode = mitk::NodePredicateProperty::New(mitk::rt::Constants::DOSE_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(true));

    mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetSubset(isDoseNode);

    for(mitk::DataStorage::SetOfObjects::const_iterator pos = nodes->begin(); pos != nodes->end(); ++pos)
    {
      (*pos)->SetFloatProperty(mitk::rt::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
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
  bool globalSync = mitk::rt::GetReferenceDoseValue(referenceDose);

  this->m_Controls.spinReferenceDose->setValue(referenceDose);
}

void RTDoseVisualizer::OnHandleCTKEventPresetsChanged(const ctkEvent& event)
{
  std::string currentPresetName  = mitk::rt::GetSelectedPresetName();

  this->OnCurrentPresetChanged(QString::fromStdString(currentPresetName));
}
