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
#include <mitkNodePredicateDataProperty.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkDoseNodeHelper.h>
#include <mitkPropertyNameHelper.h>
#include <mitkRTConstants.h>
#include <mitkIsoDoseLevelSetProperty.h>
#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkIsoLevelsGenerator.h>
#include <mitkRTUIConstants.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>


#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>

// Qmitk
#include "RTDoseVisualizer.h"

#include <QmitkDoseColorDelegate.h>
#include <QmitkDoseValueDelegate.h>
#include <QmitkDoseVisualStyleDelegate.h>
#include <QmitkIsoDoseLevelSetModel.h>
#include <QmitkFreeIsoDoseLevelWidget.h>
#include "QmitkRenderWindow.h"

#include "org_mitk_gui_qt_dosevisualization_Activator.h"

#include <vtkMath.h>
#include "vtkDecimatePro.h"

const std::string RTDoseVisualizer::VIEW_ID = "org.mitk.views.rt.dosevisualization";

const std::string RTDoseVisualizer::ISO_LINE_NODE_NAME = "dose iso lines";

namespace mitk
{
  /** @brief Predicate to identify dose nodes. Didn't use NodePredicateeDataProperty, because we want only depend
  on the property value ('RTDOSE') and not on the property type (e.g. StringProperty or mitkTemperoSpatialStringProperty).*/
  class NodePredicateDose : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDose, NodePredicateBase);
    itkNewMacro(NodePredicateDose);

    ~NodePredicateDose() override
    {};

    bool CheckNode(const mitk::DataNode *node) const override
    {
      if (!node) return false;

      auto data = node->GetData();
      if (!data) {
        return false;
      }

      auto modalityProperty = data->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str());
      if (modalityProperty.IsNull())
      {
        return false;
      }
      std::string modality = modalityProperty->GetValueAsString();
      return modality == "RTDOSE";
    };

  protected:
    NodePredicateDose()
    {};
  };

} // namespace mitk


RTDoseVisualizer::RTDoseVisualizer()
{
  m_selectedNode = nullptr;
  m_selectedPresetName = "";
  m_PrescribedDose_Data = 0.0;
  m_freeIsoValuesCount = 0;

  m_internalUpdate = false;

  m_isDoseOrIsoPredicate = mitk::NodePredicateDose::New();
  m_isIsoPredicate = mitk::NodePredicateProperty::New("name", mitk::StringProperty::New(ISO_LINE_NODE_NAME));
  m_isDosePredicate = mitk::NodePredicateAnd::New(m_isDoseOrIsoPredicate, mitk::NodePredicateNot::New(m_isIsoPredicate));
}

RTDoseVisualizer::~RTDoseVisualizer()
{
  mitk::DataStorage::SetOfObjects::ConstPointer isoNodes = this->GetDataStorage()->GetSubset(m_isIsoPredicate);
  this->GetDataStorage()->Remove(isoNodes);

  delete m_LevelSetModel;
  delete m_DoseColorDelegate;
  delete m_DoseValueDelegate;
  delete m_DoseVisualDelegate;
}

void RTDoseVisualizer::SetFocus(){}

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

  auto doseNodes = this->GetDataStorage()->GetSubset(this->m_isDosePredicate);
  auto doseNodeIter = doseNodes->begin();

  while (doseNodeIter != doseNodes->end())
  {
    PrepareDoseNode(*doseNodeIter);
    ++doseNodeIter;
  }

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

  this->m_Controls.doseBtn->setVisible(false);

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

    propsForSlot[ctkEventConstants::EVENT_TOPIC] = mitk::RTCTKEventConstants::TOPIC_GLOBAL_VISIBILITY_CHANGED.c_str();
    eventAdmin->subscribeSlot(this, SLOT(OnHandleCTKEventGlobalVisChanged(ctkEvent)), propsForSlot);
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
        mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
        if (isoDoseNode.IsNotNull()) isoDoseNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);
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
  newColor.setHsv((m_freeIsoValuesCount*85)%360,255,255);
  mitk::IsoDoseLevel::ColorType mColor;
  mColor[0]=newColor.redF();
  mColor[1]=newColor.greenF();
  mColor[2]=newColor.blueF();

  mitk::IsoDoseLevelVector::Pointer freeIsoDoseVector;
  mitk::IsoDoseLevelVectorProperty::Pointer propIsoVector;

  mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);

  if (isoDoseNode.IsNotNull())
  {
    propIsoVector = dynamic_cast<mitk::IsoDoseLevelVectorProperty*>(isoDoseNode->GetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str()));

    freeIsoDoseVector = propIsoVector->GetValue();
    if (freeIsoDoseVector.IsNull())
      mitk::IsoDoseLevelVector::Pointer freeIsoDoseVector = mitk::IsoDoseLevelVector::New();

    mitk::IsoDoseLevel::Pointer newLevel = mitk::IsoDoseLevel::New(0.5, mColor, true, false);
    freeIsoDoseVector->InsertElement(m_freeIsoValuesCount, newLevel);
    propIsoVector = mitk::IsoDoseLevelVectorProperty::New(freeIsoDoseVector);

    isoDoseNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), propIsoVector);

    m_freeIsoValuesCount++;
    this->m_Controls.btnRemoveFreeValue->setEnabled(true);
    //Update Widget
    this->UpdateFreeIsoValues();
    //Update Rendering
    this->ActualizeFreeIsoLine();
  }
}

void RTDoseVisualizer::OnRemoveFreeValueClicked()
{
  int index = this->m_Controls.listFreeValues->currentRow();
  if (index > static_cast<int>(m_freeIsoValuesCount) || index < 0)
    return;

  mitk::IsoDoseLevelVectorProperty::Pointer propfreeIsoVec;

  mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
  propfreeIsoVec = dynamic_cast<mitk::IsoDoseLevelVectorProperty*>(isoDoseNode->GetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str()));

  mitk::IsoDoseLevelVector::Pointer freeIsoDoseLevelVec = propfreeIsoVec->GetValue();
  if(freeIsoDoseLevelVec->IndexExists(index))
  {
    //freeIsoDoseLevelVec->DeleteIndex(index);
    freeIsoDoseLevelVec->erase(freeIsoDoseLevelVec->begin()+index);
    --m_freeIsoValuesCount;
    if(m_freeIsoValuesCount == 0)
      this->m_Controls.btnRemoveFreeValue->setEnabled(true);
    this->UpdateFreeIsoValues();
    this->ActualizeFreeIsoLine();
  }

}

void RTDoseVisualizer::OnUsePrescribedDoseClicked()
{
  m_Controls.spinReferenceDose->setValue(this->m_PrescribedDose_Data);
}

void RTDoseVisualizer::OnDataChangedInIsoLevelSetView()
{
  //colorwash visibility changed, update the colorwash
  this->UpdateColorWashTransferFunction();

  if(m_selectedNode.IsNotNull())
  {
    //Hack: This is a dirty hack to reinit the isodose contour node. Only if the node (or property) has changed the rendering process register the RequestUpdateAll
    //Only way to render the isoline by changes without a global reinit
    mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
    if (isoDoseNode.IsNotNull()) isoDoseNode->Modified();

    // Reinit if visibility of colorwash or isodoselevel changed
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
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

  mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
  if (isoDoseNode.IsNotNull())
  {

    mitk::IsoDoseLevelVectorProperty::Pointer propfreeIsoVec;

    propfreeIsoVec = dynamic_cast<mitk::IsoDoseLevelVectorProperty*>(isoDoseNode->GetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str()));


    mitk::IsoDoseLevelVector::Pointer freeIsoDoseLevelVec = propfreeIsoVec->GetValue();

    for (mitk::IsoDoseLevelVector::Iterator pos = freeIsoDoseLevelVec->Begin(); pos != freeIsoDoseLevelVec->End(); ++pos)
    {
      QListWidgetItem* item = new QListWidgetItem;
      item->setSizeHint(QSize(0, 25));
      QmitkFreeIsoDoseLevelWidget* widget = new QmitkFreeIsoDoseLevelWidget;

      float pref;
      m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), pref);

      widget->setIsoDoseLevel(pos.Value());
      widget->setReferenceDose(pref);
      connect(m_Controls.spinReferenceDose, SIGNAL(valueChanged(double)), widget, SLOT(setReferenceDose(double)));
      connect(widget, SIGNAL(ColorChanged(mitk::IsoDoseLevel*)), this, SLOT(ActualizeFreeIsoLine()));
      connect(widget, SIGNAL(ValueChanged(mitk::IsoDoseLevel*, mitk::DoseValueRel)), this, SLOT(ActualizeFreeIsoLine()));
      connect(widget, SIGNAL(VisualizationStyleChanged(mitk::IsoDoseLevel*)), this, SLOT(ActualizeFreeIsoLine()));

      this->m_Controls.listFreeValues->addItem(item);
      this->m_Controls.listFreeValues->setItemWidget(item, widget);
    }
  }
}

void RTDoseVisualizer::ActualizeFreeIsoLine()
{
  if(m_selectedNode.IsNotNull())
  {
    //Hack: This is a dirty hack to reinit the isodose contour node. Only if the node (or property) has changed the rendering process register the RequestUpdateAll
    //Only way to render the isoline by changes without a global reinit
    mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
    if (isoDoseNode.IsNotNull()) isoDoseNode->Modified();

    // Reinit if visibility of colorwash or isodoselevel changed
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
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
    mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
    if (isoDoseNode.IsNotNull())
    {
      isoDoseNode->SetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsoLines);

      //toggle the visibility of the free isolevel sliders
      this->m_Controls.listFreeValues->setEnabled(showIsoLines);

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void RTDoseVisualizer::UpdateColorWashTransferFunction()
{
  //Generating the Colorwash
  vtkSmartPointer<vtkColorTransferFunction> transferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

  if(m_selectedNode.IsNotNull())
  {
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
}

/**Simple check if the passed node has dose visualization properties set.*/
bool hasDoseVisProperties(mitk::DataNode::Pointer doseNode)
{
  return doseNode->GetProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str()) != nullptr;
}

void RTDoseVisualizer::OnSelectionChanged( berry::IWorkbenchPart::Pointer,
                                          const QList<mitk::DataNode::Pointer>&)
{
  QList<mitk::DataNode::Pointer> dataNodes = this->GetDataManagerSelection();

  mitk::DataNode* selectedNode = nullptr;

  if (!dataNodes.empty())
  {
    bool isDoseNode = m_isDosePredicate->CheckNode(dataNodes[0].GetPointer());

    if (isDoseNode)
    {
      selectedNode = dataNodes[0];
    }
  }

  if (selectedNode != m_selectedNode.GetPointer())
  {
    m_selectedNode = selectedNode;
    if (selectedNode)
    {
      PrepareDoseNode(m_selectedNode);
    }
  }

  UpdateBySelectedNode();
}

void RTDoseVisualizer::PrepareDoseNode( mitk::DataNode* doseNode ) const
{
  mitk::DoseValueAbs dose;
  mitk::GetReferenceDoseValue(dose);
  auto presetMap = mitk::LoadPresetsMap();
  auto colorPreset = mitk::GenerateIsoLevels_Virtuos();
  auto finding = presetMap.find(mitk::GetSelectedPresetName());
  if (finding != presetMap.end())
  {
    colorPreset = finding->second;
  }

  if (!hasDoseVisProperties(doseNode))
  {
    mitk::ConfigureNodeAsDoseNode(doseNode, colorPreset, dose, true);
  }

  mitk::DataNode::Pointer doseOutlineNode = this->GetIsoDoseNode(doseNode);
  if (doseOutlineNode.IsNull())
  {
    doseOutlineNode = mitk::DataNode::New();
    doseOutlineNode->SetData(doseNode->GetData());
    doseOutlineNode->SetName(ISO_LINE_NODE_NAME);

    mitk::ConfigureNodeAsIsoLineNode(doseOutlineNode, colorPreset, dose, true);
    this->GetDataStorage()->Add(doseOutlineNode, doseNode);
  }
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
  }
  else
  {
    //dose specific information
      int fracCount = 1;
      m_selectedNode->GetIntProperty(mitk::RTConstants::DOSE_FRACTION_COUNT_PROPERTY_NAME.c_str(), fracCount);
      m_Controls.NrOfFractions->setText(QString::number(fracCount));

      m_PrescribedDose_Data = 0.0;

      auto prescibedDoseProperty = m_selectedNode->GetData()->GetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str());
      auto prescribedDoseGenericProperty = dynamic_cast<mitk::GenericProperty<double>*>(prescibedDoseProperty.GetPointer());
      m_PrescribedDose_Data = prescribedDoseGenericProperty->GetValue();

      m_Controls.prescribedDoseSpecific->setText(QString::number(m_PrescribedDose_Data));

    //free iso lines
    mitk::DataNode::Pointer isoDoseNode = this->GetIsoDoseNode(m_selectedNode);
    if (isoDoseNode)
    {
      mitk::IsoDoseLevelVectorProperty::Pointer propIsoVector;

      propIsoVector = dynamic_cast<mitk::IsoDoseLevelVectorProperty*>(isoDoseNode->GetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str()));

      if (propIsoVector.IsNull())
      {
        mitk::IsoDoseLevelVector::Pointer freeIsoValues = mitk::IsoDoseLevelVector::New();
        propIsoVector = mitk::IsoDoseLevelVectorProperty::New(freeIsoValues);
        isoDoseNode->SetProperty(mitk::RTConstants::DOSE_FREE_ISO_VALUES_PROPERTY_NAME.c_str(), propIsoVector);
      }

      UpdateFreeIsoValues();

      //global dose issues
      //ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring
      bool showIsoLine = mitk::GetGlobalIsolineVis();
      isoDoseNode->GetBoolProperty(mitk::RTConstants::DOSE_SHOW_ISOLINES_PROPERTY_NAME.c_str(), showIsoLine);
      m_Controls.checkGlobalVisIsoLine->setChecked(showIsoLine);
    }

    bool showColorWash = mitk::GetGlobalColorwashVis();
    m_selectedNode->GetBoolProperty(mitk::RTConstants::DOSE_SHOW_COLORWASH_PROPERTY_NAME.c_str(),showColorWash);
    m_Controls.checkGlobalVisColorWash->setChecked(showColorWash);

    float referenceDose = 0.0;
    m_selectedNode->GetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(),referenceDose);
    m_Controls.spinReferenceDose->setValue(referenceDose);

    mitk::IsoDoseLevelSetProperty::Pointer propIsoSet =
      dynamic_cast<mitk::IsoDoseLevelSetProperty* >(m_selectedNode->GetProperty(mitk::RTConstants::DOSE_ISO_LEVELS_PROPERTY_NAME.c_str()));

    if (propIsoSet)
    {
      this->m_LevelSetModel->setIsoDoseLevelSet(propIsoSet->GetValue());
    }
  }
}

void RTDoseVisualizer::NodeRemoved(const mitk::DataNode* node)
{
  /**@TODO This removal seems to be needed because of the current dose rendering approach (additional sub node for iso dose lines).
   As soon as we have a better and sound single node rendering mechanism for doses. This method should be removed.*/
  bool isdose = m_isDosePredicate->CheckNode(node);
  if (isdose)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(node, m_isIsoPredicate);
    auto iterChildNodes = childNodes->begin();

    while (iterChildNodes != childNodes->end())
    {
      this->GetDataStorage()->Remove((*iterChildNodes));
      ++iterChildNodes;
    }
  }
}

void RTDoseVisualizer::NodeChanged(const mitk::DataNode *node)
{
  /**@TODO This event seems to be needed because of the current dose rendering approach (additional sub node for iso dose lines).
  As soon as we have a better and sound single node rendering mechanism for doses. This method should be removed.*/
  bool isdose = m_isDosePredicate->CheckNode(node);
  if(isdose)
  {
    bool isvisible = true;
    if(node->GetBoolProperty("visible", isvisible))
    {
      mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(node, m_isIsoPredicate);
      mitk::DataStorage::SetOfObjects::const_iterator iterChildNodes = childNodes->begin();

      while (iterChildNodes != childNodes->end())
      {
        (*iterChildNodes)->SetVisibility(isvisible);
        ++iterChildNodes;
      }
    }
  }
}

void RTDoseVisualizer::UpdateByPreferences()
{
  m_Presets = mitk::LoadPresetsMap();
  m_internalUpdate = true;
  m_Controls.comboPresets->clear();
  this->m_selectedPresetName = mitk::GetSelectedPresetName();

  m_Controls.checkGlobalVisIsoLine->setChecked(mitk::GetGlobalIsolineVis());
  m_Controls.checkGlobalVisColorWash->setChecked(mitk::GetGlobalColorwashVis());

  if(m_Presets.empty())
    return;

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
    MITK_WARN << "Error. Iso dose level preset specified in preferences does not exist. Preset name: "<<this->m_selectedPresetName;
    this->m_selectedPresetName = m_Presets.begin()->first;
    mitk::SetSelectedPresetName(this->m_selectedPresetName);
    MITK_INFO << "Changed selected iso dose level preset to first existing preset. New preset name: "<<this->m_selectedPresetName;
  }

  m_Controls.comboPresets->setCurrentIndex(selectedIndex);

  this->m_LevelSetModel->setIsoDoseLevelSet(this->m_Presets[this->m_selectedPresetName]);

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

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetSubset(m_isDoseOrIsoPredicate);

  if(presetMap.empty())
    return;

  mitk::IsoDoseLevelSet* selectedPreset = presetMap[presetName];

  if (!selectedPreset)
  {
    mitkThrow() << "Error. Cannot actualize iso dose level preset. Selected preset does not exist. Preset name: "<<presetName;
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
    mitk::DoseValueAbs value = 0;
    bool sync = mitk::GetReferenceDoseValue(value);

    if (sync)
    {
      mitk::DataStorage::SetOfObjects::ConstPointer nodes = this->GetDataStorage()->GetSubset(m_isDoseOrIsoPredicate);

        for(mitk::DataStorage::SetOfObjects::const_iterator pos = nodes->begin(); pos != nodes->end(); ++pos)
        {
            (*pos)->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), value);

            /**@TODO: ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring*/
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

void RTDoseVisualizer::OnHandleCTKEventReferenceDoseChanged(const ctkEvent&)
{
  mitk::DoseValueAbs referenceDose = 0.0;
  mitk::GetReferenceDoseValue(referenceDose);

  this->m_Controls.spinReferenceDose->setValue(referenceDose);
}

void RTDoseVisualizer::OnHandleCTKEventGlobalVisChanged(const ctkEvent&)
{
  this->m_Controls.checkGlobalVisIsoLine->setChecked(mitk::GetGlobalIsolineVis());
  this->m_Controls.checkGlobalVisColorWash->setChecked(mitk::GetGlobalColorwashVis());
}

void RTDoseVisualizer::OnHandleCTKEventPresetsChanged(const ctkEvent&)
{
  std::string currentPresetName  = mitk::GetSelectedPresetName();

  this->OnCurrentPresetChanged(QString::fromStdString(currentPresetName));
}

/**@TODO ATM the IsoDoseContours have an own (helper) node which is a child of dose node; Will be fixed with the doseMapper refactoring*/
mitk::DataNode::Pointer RTDoseVisualizer::GetIsoDoseNode(mitk::DataNode::Pointer doseNode) const
{
  mitk::DataStorage::SetOfObjects::ConstPointer childNodes = this->GetDataStorage()->GetDerivations(doseNode, m_isIsoPredicate);
  if (childNodes->empty())
  {
    return nullptr;
  }
  else
  {
    return (*childNodes->begin());
  }
}
