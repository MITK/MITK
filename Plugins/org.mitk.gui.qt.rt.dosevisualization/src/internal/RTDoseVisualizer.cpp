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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
// MITK
#include <mitkNodePredicateProperty.h>

// Qmitk
#include "RTDoseVisualizer.h"

#include <DataStructures/mitkRTConstants.h>
#include <DataStructures/mitkIsoDoseLevelSetProperty.h>
#include <DataStructures/mitkIsoDoseLevelVectorProperty.h>
#include <mitkIsoLevelsGenerator.h>

#include <QmitkDoseColorDelegate.h>
#include <QmitkDoseValueDelegate.h>
#include <QmitkDoseVisualStyleDelegate.h>
#include <QmitkIsoDoseLevelSetModel.h>
#include <QmitkFreeIsoDoseLevelWidget.h>

// Qt
#include <QMessageBox>
#include <QMenu>

const std::string RTDoseVisualizer::VIEW_ID = "org.mitk.views.rt.dosevisualization";

RTDoseVisualizer::RTDoseVisualizer()
{
    m_freeIsoValues = mitk::IsoDoseLevelVector::New();
    m_selectedNodeIsoSet = mitk::IsoDoseLevelSet::New();
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

void RTDoseVisualizer::SetFocus()
{
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
  }
}

void RTDoseVisualizer::OnAddFreeValueClicked()
{
  QColor newColor;
  //Use HSV schema of QColor to calculate a different color depending on the
  //number of already existing free iso lines.
  newColor.setHsv((m_freeIsoValues->Size()*85)%360,255,255);

  mitk::IsoDoseLevel::ColorType color;
  color[0] = newColor.redF();
  color[1] = newColor.greenF();
  color[2] = newColor.blueF();
  m_freeIsoValues->push_back(mitk::IsoDoseLevel::New(0.5,color,true,false));
  UpdateFreeIsoValues();
}

void RTDoseVisualizer::OnRemoveFreeValueClicked()
{
  m_freeIsoValues->pop_back();
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

    widget->setIsoDoseLevel(pos->Value().GetPointer());
    connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), widget, SLOT(setReferenceDose(double)));

    this->m_Controls.listFreeValues->addItem(item);
    this->m_Controls.listFreeValues->setItemWidget(item,widget);
  }
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
  }
}


void RTDoseVisualizer::OnConvertButtonClicked()
{
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

    selectedNode->GetData()->SetProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(), mitk::DoubleProperty::New(1.0));

    mitk::IsoDoseLevelSet::Pointer clonedPreset = this->m_Presets[this->m_selectedPresetName]->Clone();
    mitk::IsoDoseLevelSetProperty::Pointer levelSetProp = mitk::IsoDoseLevelSetProperty::New(clonedPreset);
    selectedNode->SetProperty(mitk::rt::Constants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str(),levelSetProp);

    mitk::IsoDoseLevelVector::Pointer levelVector = mitk::IsoDoseLevelVector::New();
    mitk::IsoDoseLevelVectorProperty::Pointer levelVecProp = mitk::IsoDoseLevelVectorProperty::New(levelVector);
    selectedNode->SetProperty(mitk::rt::Constants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(),levelVecProp);

    UpdateBySelectedNode();

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
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
    ///////////////////////////////////////////
    //dose specific information
    int fracCount = 1;
    m_selectedNode->GetIntProperty(mitk::rt::Constants::DOSE_FRACTION_COUNT_PROPERTY_NAME.c_str(),fracCount);   
    m_Controls.NrOfFractions->setText(QString::number(fracCount));

    m_PrescribedDose_Data = 0.0;
    mitk::DoubleProperty* propDouble = dynamic_cast<mitk::DoubleProperty*>(m_selectedNode->GetData()->GetProperty(mitk::rt::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str()).GetPointer());
    if (propDouble)
    {
      m_PrescribedDose_Data = propDouble->GetValue();
    }

    m_Controls.prescribedDoseSpecific->setText(QString::number(m_PrescribedDose_Data));

    ///////////////////////////////////////////
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

    ///////////////////////////////////////////
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
