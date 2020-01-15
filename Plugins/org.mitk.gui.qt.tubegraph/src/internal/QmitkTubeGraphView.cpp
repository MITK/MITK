/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTubeGraphView.h"

#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include "mitkNodePredicateDataType.h"
#include "usModuleRegistry.h"
#include "QmitkTubeGraphLabelGroupWidget.h"
#include "QmitkTubeGraphDeleteLabelGroupDialog.h"
#include "QmitkTubeGraphNewLabelGroupDialog.h"
#include "QmitkTubeGraphNewAnnotationDialog.h"

const std::string QmitkTubeGraphView::VIEW_ID = "org.mitk.views.tubegraph";

QmitkTubeGraphView::QmitkTubeGraphView()
  :QmitkAbstractView(),
  m_ActiveTubeGraph(),
  m_ActiveProperty(),
  m_ActivationMode(mitk::TubeGraphDataInteractor::None)
{
}

QmitkTubeGraphView::~QmitkTubeGraphView()
{
  //m_ActivationMode.mitk::TubeGraphDataInteractor::ActivationMode::~ActivationMode();

  //remove observer
  //if (m_ActiveInteractor.IsNotNull())
  //m_ActiveInteractor->RemoveObserver(m_InformationChangedObserverTag);

}

void QmitkTubeGraphView::SetFocus()
{
  m_Controls.activeNodeGroupBox->setFocus();
}

void QmitkTubeGraphView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);
  this->CreateConnections();
}

void QmitkTubeGraphView::NodeRemoved(const mitk::DataNode* /*node*/)
{

}

void QmitkTubeGraphView::CreateConnections()
{
  mitk::NodePredicateDataType::Pointer tubeGraphPred = mitk::NodePredicateDataType::New("TubeGraph");
  m_Controls.activeGraphComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.activeGraphComboBox->SetPredicate(tubeGraphPred);

  //activation mode
  connect( m_Controls.activeGraphComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(OnActiveGraphChanged(int)) );
  connect( m_Controls.noneModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.singleModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.multipleModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.peripheryModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.rootModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.pointModeRadioButton, SIGNAL(clicked()), this, SLOT(OnActivationModeChanged()));
  connect( m_Controls.setRootButton, SIGNAL(toggled(bool)), this, SLOT(OnSetRootToggled(bool)));

  //deselect tubes
  connect( m_Controls.deselectAllButton, SIGNAL(clicked()), this, SLOT(OnDeselectAllTubes()));

  // tab switch
  connect( m_Controls.tubeGraphTabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabSwitched(int)));

  //attributation tab
  connect( m_Controls.addLabelGroupButton, SIGNAL(clicked()), this, SLOT(OnAddingLabelGroup()));
  connect( m_Controls.removeLabelGroupButton, SIGNAL(clicked()), this, SLOT(OnRemoveLabelGroup()));

  //annotation tab
  connect( m_Controls.addAnnotationButton, SIGNAL(clicked()), this, SLOT(OnAddingAnnotation()));
  connect( m_Controls.deleteAnnotationButton, SIGNAL(clicked()), this, SLOT(OnRemoveAnnotation()));

  //edit tab
  connect( m_Controls.editSelectionNewButton, SIGNAL(clicked()), this, SLOT(OnAddTubeBetweenSelection()));
  connect( m_Controls.editSelectionSeperateButton, SIGNAL(clicked()), this, SLOT(OnSeperateSelection()));
  connect( m_Controls.editSelectionDeleteButton, SIGNAL(clicked()), this, SLOT(OnDeleteSelection()));


  this->SetTabsEnable(false);
}

void QmitkTubeGraphView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  for (mitk::DataNode::Pointer node: nodes)
  {
    if( node.IsNotNull() && dynamic_cast< mitk::TubeGraph* >(node->GetData()) )
    {
      m_Controls.activeGraphComboBox->SetSelectedNode(node);
      m_ActiveTubeGraph = dynamic_cast< mitk::TubeGraph* >(node->GetData());

      m_ActiveProperty = dynamic_cast<mitk::TubeGraphProperty*>(m_ActiveTubeGraph->GetProperty( "Tube Graph.Visualization Information" ).GetPointer());

      this->UpdateActiveTubeGraphInInteractors();
      this->UpdateLabelGroups();
      this->UpdateAnnotation();
    }
  }
}

void QmitkTubeGraphView::OnActiveGraphChanged(int)
{
  mitk::DataNode::Pointer selectedNode = m_Controls.activeGraphComboBox->GetSelectedNode();

  if(selectedNode.IsNotNull())
  {
    m_ActiveTubeGraph = dynamic_cast <mitk::TubeGraph* >( selectedNode->GetData() );

    m_ActiveProperty = dynamic_cast<mitk::TubeGraphProperty*>(m_ActiveTubeGraph->GetProperty("Tube Graph.Visualization Information" ).GetPointer());
    this->SetTabsEnable(true);
    this->UpdateGraphInformation();
  }
  else
  {
    m_ActiveTubeGraph = nullptr;
    m_ActiveProperty = nullptr;
    this->SetTabsEnable(false);

  }
  this->UpdateActiveTubeGraphInInteractors();
  this->UpdateLabelGroups();
  this->UpdateAnnotation();
}

void QmitkTubeGraphView::UpdateActiveTubeGraphInInteractors()
{
  //
  // traverse all tube graphs and check, and add an interactor if needed...
  //
  mitk::NodePredicateDataType::Pointer tubeGraphPred = mitk::NodePredicateDataType::New("TubeGraph");
  mitk::DataStorage::SetOfObjects::ConstPointer tubeGraphs = this->GetDataStorage()->GetSubset(tubeGraphPred);
  for(mitk::DataStorage::SetOfObjects::ConstIterator it = tubeGraphs->Begin();
    it != tubeGraphs->End();
    ++it)
  {
    mitk::DataNode::Pointer node = it.Value().GetPointer();
    if ( node->GetDataInteractor().IsNull() )
    {
      mitk::TubeGraphDataInteractor::Pointer tubeGraphInteractor = mitk::TubeGraphDataInteractor::New();
      tubeGraphInteractor->LoadStateMachine("TubeGraphInteraction.xml", us::ModuleRegistry::GetModule("MitkTubeGraph"));
      tubeGraphInteractor->SetEventConfig("TubeGraphConfig.xml", us::ModuleRegistry::GetModule("MitkTubeGraph"));
      tubeGraphInteractor->SetActivationMode(m_ActivationMode);
      tubeGraphInteractor->SetDataNode(node);
    }
  }

  if (m_ActiveTubeGraph.IsNotNull())
  {
    //remove old observer
   // if (m_ActiveInteractor.IsNotNull())
      //  m_ActiveInteractor->RemoveObserver(m_InformationChangedObserverTag);

        mitk::DataNode::Pointer node = m_Controls.activeGraphComboBox->GetSelectedNode();
    //set active interactor to interactor from selected node
    m_ActiveInteractor = dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer());

    m_ActiveInteractor->SetActivationMode(m_ActivationMode);

    //add observer to activeInteractor
    typedef itk::SimpleMemberCommand< QmitkTubeGraphView > SimpleCommandType;
    SimpleCommandType::Pointer changeInformationCommand = SimpleCommandType::New();
    changeInformationCommand->SetCallbackFunction(this, &QmitkTubeGraphView::SelectionInformationChanged);
    //m_InformationChangedObserverTag = m_ActiveInteractor->AddObserver(mitk::SelectionChangedTubeGraphEvent(), changeInformationCommand);

    ////Add selected node to all tube graph interactors as current node
    //mitk::TubeGraphDataInteractor::SetCurrentSelectedDataNode( node );

    if (m_ActiveProperty.IsNull())
    {
      mitk::TubeGraphProperty::Pointer newProperty = mitk::TubeGraphProperty::New();
      m_ActiveTubeGraph->SetProperty("Tube Graph.Visualization Information",newProperty);
      m_ActiveProperty = newProperty;
    }
  }
}

void QmitkTubeGraphView::UpdateLabelGroups()
{
  for(int i = m_Controls.toolBox->count()- 1; i >= 0  ; i--)
    m_Controls.toolBox->removeItem(i);

  if (m_ActiveProperty.IsNotNull())
  {
    if(m_ActiveProperty->GetNumberOfLabelGroups()!= 0)
    {
      std::vector<LabelGroupType*> labelGroups = m_ActiveProperty->GetLabelGroups();

      for (unsigned int k = 0; k < labelGroups.size(); k++)
        this->CreateLabelGroupWidget(labelGroups.at(k));
    }
  }
}

void QmitkTubeGraphView::OnActivationModeChanged()
{
  if (m_Controls.noneModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::None;
  else if (m_Controls.singleModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::Single;
  else if (m_Controls.multipleModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::Multiple;
  else if (m_Controls.rootModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::ToRoot;
  else if (m_Controls.peripheryModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::ToPeriphery;
  else if (m_Controls.pointModeRadioButton->isChecked())
    m_ActivationMode = mitk::TubeGraphDataInteractor::Points;
  else //normally not possible, but.... set to single mode
    m_ActivationMode = mitk::TubeGraphDataInteractor::Single;

  mitk::DataNode::Pointer node = m_Controls.activeGraphComboBox->GetSelectedNode();
  if(node.IsNotNull() && m_ActiveProperty.IsNotNull())
  {
    dynamic_cast< mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActivationMode(m_ActivationMode);

    //m_ActiveTubeGraph->Modified();
    // render new selection
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

mitk::TubeGraphDataInteractor::ActivationMode QmitkTubeGraphView::GetActivationMode ()
{
  return m_ActivationMode;
}

void QmitkTubeGraphView::OnSetRootToggled(bool enable)
{
  this->SetTabsEnable(!enable);
  if (m_ActiveTubeGraph.IsNotNull())
  {
    mitk::DataNode::Pointer node = m_Controls.activeGraphComboBox->GetSelectedNode();
    if(node.IsNotNull())
    {
      if (enable)
        dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::RootMode);
      else
        dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::AttributationMode);
    }
  }
}

void QmitkTubeGraphView::OnDeselectAllTubes()
{
  mitk::DataNode::Pointer node = m_Controls.activeGraphComboBox->GetSelectedNode();
  if(node.IsNotNull())
    dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->ResetPickedTubes();

  if (m_ActiveProperty.IsNull())
  {
    MITK_ERROR << "Oho!No Property..mhhhh";
    return;
  }
  m_ActiveProperty->DeactivateAllTubes();
  m_ActiveTubeGraph->Modified();
  // render new selection
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTubeGraphView::OnTabSwitched(int tabIndex)
{
  mitk::DataNode::Pointer node = m_Controls.activeGraphComboBox->GetSelectedNode();
  if(node.IsNotNull())
  {
    if (dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer()) == nullptr)
    {
      this->UpdateActiveTubeGraphInInteractors();
      /* if (dynamic_cast<mitk::TubeGraphDataInteractor::Pointer>(node->GetDataInteractor().GetPointer()).IsNull());*/
      return;
    }

    switch(tabIndex)
    {
    case 0:
      dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::AttributationMode);
      m_Controls.noneModeRadioButton->setEnabled(true);
      m_Controls.singleModeRadioButton->setEnabled(true);
      m_Controls.multipleModeRadioButton->setEnabled(true);
      m_Controls.peripheryModeRadioButton->setEnabled(true);
      m_Controls.rootModeRadioButton->setEnabled(true);
      m_Controls.pointModeRadioButton->setEnabled(true);
      break;
    case 1:
      dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::AnnotationMode);
      m_ActiveProperty->DeactivateAllTubes();
      m_Controls.noneModeRadioButton->setEnabled(true);
      m_Controls.singleModeRadioButton->setEnabled(true);
      m_Controls.multipleModeRadioButton->setEnabled(false);
      m_Controls.peripheryModeRadioButton->setEnabled(false);
      m_Controls.rootModeRadioButton->setEnabled(false);
      m_Controls.pointModeRadioButton->setEnabled(false);
      break;
    case 2:
      dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::EditMode);
      m_Controls.noneModeRadioButton->setEnabled(true);
      m_Controls.singleModeRadioButton->setEnabled(true);
      m_Controls.multipleModeRadioButton->setEnabled(true);
      m_Controls.peripheryModeRadioButton->setEnabled(true);
      m_Controls.rootModeRadioButton->setEnabled(true);
      m_Controls.pointModeRadioButton->setEnabled(true);
      break;
    case 3:
      dynamic_cast<mitk::TubeGraphDataInteractor*>(node->GetDataInteractor().GetPointer())->SetActionMode(mitk::TubeGraphDataInteractor::InformationMode);
      m_Controls.noneModeRadioButton->setEnabled(false);
      m_Controls.singleModeRadioButton->setEnabled(false);
      m_Controls.multipleModeRadioButton->setEnabled(false);
      m_Controls.peripheryModeRadioButton->setEnabled(false);
      m_Controls.rootModeRadioButton->setEnabled(false);
      m_Controls.pointModeRadioButton->setEnabled(false);
      this->UpdateGraphInformation();
      break;
    default:
      break;
    }
  }
}

void QmitkTubeGraphView::OnAddingLabelGroup()
{
  QmitkTubeGraphNewLabelGroupDialog* dialog = new QmitkTubeGraphNewLabelGroupDialog( m_Parent );

  int dialogReturnValue = dialog->exec();

  LabelGroupType* newLabelGroup = dialog->GetLabelGroup();
  delete dialog;

  if ( dialogReturnValue != QDialog::Rejected ) // user doesn't clicked cancel or pressed Esc or something similar
  {
    m_ActiveProperty->AddLabelGroup(newLabelGroup, m_ActiveProperty->GetNumberOfLabelGroups());
    this->UpdateLabelGroups();
  }
}

void QmitkTubeGraphView::CreateLabelGroupWidget(LabelGroupType* labelGroup)
{
  QmitkTubeGraphLabelGroupWidget* labelGroupWidget = new QmitkTubeGraphLabelGroupWidget(m_Controls.toolBox, (labelGroup->labelGroupName).c_str());

  for (std::vector<LabelType*>::iterator it = labelGroup->labels.begin(); it != labelGroup->labels.end(); it++)
  {
    labelGroupWidget->AddLabel(QString::fromStdString((*it)->labelName), (*it)->labelColor);
  }

  m_Controls.toolBox->addItem(labelGroupWidget, QString::fromStdString(labelGroup->labelGroupName));

  connect(labelGroupWidget, SIGNAL(SignalLabelVisibilityInGroupToggled(bool, QString, QString)), this, SLOT(OnLabelVisibilityChanged(bool, QString, QString)));
  connect(labelGroupWidget, SIGNAL(SignalLabelButtonInGroupClicked(QString, QString)), this, SLOT(OnLabelChanged(QString, QString)));
  connect(labelGroupWidget, SIGNAL(SignalLabelColorInGroupChanged(mitk::Color, QString, QString)), this, SLOT(OnLabelColorChanged(mitk::Color, QString, QString)));
}

void QmitkTubeGraphView::OnRemoveLabelGroup()
{
  QStringList allLabelGroups;
  std::vector<LabelGroupType*> labelGroups = m_ActiveProperty->GetLabelGroups();
  for (unsigned int i = 0; i < labelGroups.size(); i++)
    allLabelGroups.push_back(QString::fromStdString(labelGroups.at(i)->labelGroupName));

  QmitkTubeGraphDeleteLabelGroupDialog* dialog = new QmitkTubeGraphDeleteLabelGroupDialog( m_Parent );
  dialog->SetLabelGroups(allLabelGroups);

  int dialogReturnValue = dialog->exec();

  QList<QString> toDeleteItemsList = dialog->GetSelectedLabelGroups();

  delete dialog;

  if ( dialogReturnValue != QDialog::Rejected ) // user clicked cancel or pressed Esc or something similar
  {
    for (QList<QString>::iterator it = toDeleteItemsList.begin(); it != toDeleteItemsList.end(); it++ )
    {
      std::string labelGroupName = (*it).toStdString();
      LabelGroupType* labelGroup = m_ActiveProperty->GetLabelGroupByName(labelGroupName);
      m_ActiveProperty->RemoveLabelGroup(labelGroup);
    }
    this->UpdateLabelGroups();
  }
}

void QmitkTubeGraphView::OnAddingAnnotation()
{
  std::vector <mitk::TubeGraph::TubeDescriptorType> activeTubes = m_ActiveProperty->GetActiveTubes();
  if (activeTubes.size()!= 0)
  {
    QmitkTubeGraphNewAnnotationDialog* dialog = new QmitkTubeGraphNewAnnotationDialog( m_Parent );

    int dialogReturnValue = dialog->exec();

    QString newAnnotationName = dialog->GetAnnotationName();
    QString newDescription = dialog->GetAnnotationDescription();
    QString tubeString = QString("[%1,%2]").arg(activeTubes.begin()->first).arg(activeTubes.begin()->second);

    delete dialog;

    if ( dialogReturnValue != QDialog::Rejected ) // user doesn't clicked cancel or pressed Esc or something similar
    {

      mitk::TubeGraphProperty::Annotation* annotation = new mitk::TubeGraphProperty::Annotation();
      annotation->name =  newAnnotationName.toStdString();
      annotation->description = newDescription.toStdString();
      annotation->tube = (*activeTubes.begin());

      m_ActiveProperty->AddAnnotation(annotation);
      this->UpdateAnnotation();
    }
  }
  else
  {
    MITK_INFO<< "No Tube select for annotation";
  }
}

void QmitkTubeGraphView::UpdateAnnotation()
{
  m_Controls.annotationsTableWidget->clear();
  m_Controls.annotationsTableWidget->setRowCount(0);
  if (m_ActiveProperty.IsNotNull())
  {
    std::vector<mitk::TubeGraphProperty::Annotation*> annotations = m_ActiveProperty->GetAnnotations();

    if (annotations.size() != 0)
    {
      for (unsigned int k = 0; k < annotations.size(); k++)
      {
        m_Controls.annotationsTableWidget->setRowCount(k+1); // add one row

        QTableWidgetItem* annotationNameItem = new QTableWidgetItem(QString::fromStdString(annotations[k]->name));
        QTableWidgetItem* annotationDescriptionItem = new QTableWidgetItem(QString::fromStdString(annotations[k]->description));
        QString tubeString = QString("[%1,%2]").arg(annotations[k]->tube.first).arg(annotations[k]->tube.second);
        QTableWidgetItem* annotationTubeItem = new QTableWidgetItem(tubeString);

        m_Controls.annotationsTableWidget->setItem(k, 0, annotationNameItem);
        m_Controls.annotationsTableWidget->setItem(k, 1, annotationDescriptionItem);
        m_Controls.annotationsTableWidget->setItem(k, 2, annotationTubeItem);
      }
    }
  }
}


void QmitkTubeGraphView::OnRemoveAnnotation()
{
  //m_Controls.annotationsTableWidget->removeRow(m_Controls.annotationsTableWidget->currentRow());
  int row = m_Controls.annotationsTableWidget->currentRow();

  if (row >= 0)
  {
    std::string name =  m_Controls.annotationsTableWidget->item(row, 0)->text().toStdString();

    mitk::TubeGraphProperty::Annotation* annotation = m_ActiveProperty->GetAnnotationByName(name);
    m_ActiveProperty->RemoveAnnotation(annotation);
    this->UpdateAnnotation();
  }

}


void QmitkTubeGraphView::OnAddTubeBetweenSelection()
{
}

void QmitkTubeGraphView::OnSeperateSelection()
{
  std::vector <mitk::TubeGraph::TubeDescriptorType> activeTubes = m_ActiveProperty->GetActiveTubes();
  if (activeTubes.size()!= 0)
  {
    mitk::TubeGraph::Pointer subGraph = m_ActiveTubeGraph->CreateSubGraph(activeTubes);

    mitk::DataNode::Pointer originGraphNode = m_Controls.activeGraphComboBox->GetSelectedNode();

    mitk::DataNode::Pointer newGraphNode = mitk::DataNode::New();
    newGraphNode->SetData(subGraph);
    newGraphNode->SetName(originGraphNode->GetName() + "-SubGraph");

    mitk::TubeGraphDataInteractor::Pointer tubeGraphInteractor = mitk::TubeGraphDataInteractor::New();
    tubeGraphInteractor->LoadStateMachine("TubeGraphInteraction.xml", us::ModuleRegistry::GetModule("TubeGraph"));
    tubeGraphInteractor->SetEventConfig("TubeGraphConfig.xml", us::ModuleRegistry::GetModule("TubeGraph"));
    tubeGraphInteractor->SetDataNode(newGraphNode);

    mitk::TubeGraphProperty::Pointer newProperty = mitk::TubeGraphProperty::New();
    subGraph->SetProperty("Tube Graph.Visualization Information",newProperty);

    this->GetDataStorage()->Add(newGraphNode, originGraphNode);
  }
}

void QmitkTubeGraphView::OnDeleteSelection()
{ //TODO Action for Interactor + undo/redo

  std::vector <mitk::TubeGraph::TubeDescriptorType> activeTubes = m_ActiveProperty->GetActiveTubes();
  m_ActiveTubeGraph->RemoveSubGraph(m_ActiveProperty->GetActiveTubes());
}

void QmitkTubeGraphView::OnLabelVisibilityChanged(bool isVisible, QString labelName, QString labelGroupName)
{
  if (m_ActiveProperty.IsNull())
  {
    MITK_ERROR << "Oho!No Property..mhhhh";
    return;
  }
  LabelGroupType* labelGroup = m_ActiveProperty->GetLabelGroupByName(labelGroupName.toStdString());
  LabelType* label = m_ActiveProperty->GetLabelByName(labelGroup, labelName.toStdString());

  if(labelGroup == nullptr || label == nullptr )
  {
    MITK_ERROR << "This label group or label doesn't exist!";
    return;
  }

  m_ActiveProperty->SetLabelVisibility(label, isVisible);
  m_ActiveTubeGraph->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTubeGraphView::OnLabelChanged(QString labelName, QString labelGroupName)
{
  if (m_ActiveProperty.IsNull())
  {
    MITK_ERROR << "Oho!No Property..mhhhh";
    return;
  }
  LabelGroupType* labelGroup = m_ActiveProperty->GetLabelGroupByName(labelGroupName.toStdString());
  LabelType* label = m_ActiveProperty->GetLabelByName(labelGroup, labelName.toStdString());
  if(labelGroup == nullptr || label == nullptr )
  {
    MITK_ERROR << "This label group or label doesn't exist!";
    return;
  }

  m_ActiveProperty->SetLabelForActivatedTubes(labelGroup, label);
  m_ActiveTubeGraph->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTubeGraphView::OnLabelColorChanged (mitk::Color color, QString labelName, QString labelGroupName)
{
  if (m_ActiveProperty.IsNull())
  {
    MITK_ERROR << "Oho!No Property..mhhhh";
    return;
  }

  LabelGroupType* labelGroup = m_ActiveProperty->GetLabelGroupByName(labelGroupName.toStdString());
  LabelType* label = m_ActiveProperty->GetLabelByName(labelGroup, labelName.toStdString());

  if(labelGroup == nullptr || label == nullptr )
  {
    MITK_ERROR << "This label group or label doesn't exist!";
    return;
  }

  m_ActiveProperty->SetLabelColor(label, color);
  m_ActiveTubeGraph->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkTubeGraphView::SetTabsEnable(bool enable)
{
  m_Controls.tubeGraphTabWidget->setTabEnabled(0, enable); //attributationTab
  m_Controls.tubeGraphTabWidget->setTabEnabled(1, enable);//annotationTab
  m_Controls.tubeGraphTabWidget->setTabEnabled(2, enable);//editTab
  m_Controls.tubeGraphTabWidget->setTabEnabled(3, enable);//informationTab
}

void QmitkTubeGraphView::UpdateGraphInformation()
{
  mitk::TubeGraph::TubeDescriptorType root = m_ActiveTubeGraph->GetRootTube();
  mitk::BaseGeometry::Pointer geometry = m_ActiveTubeGraph->GetGeometry();
  mitk::Point3D origin = geometry->GetOrigin();
  mitk::Vector3D spacing = geometry->GetSpacing();
  QString information;
  information.append(QString("Number of Vertices: %1\n").arg(m_ActiveTubeGraph->GetNumberOfVertices()));
  information.append(QString("Number of Edges: %1\n").arg(m_ActiveTubeGraph->GetNumberOfEdges()));
  information.append(QString("Root Id: [%1,%2]\n\n").arg(root.first).arg(root.second));
  information.append(QString("Origin: [%1,%2,%3]\n").arg(origin[0]).arg(origin[1]).arg(origin[2]));
  information.append(QString("Spacing: [%1,%2,%3]").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]));

  m_Controls.informationLabel->setText(information);
}

void QmitkTubeGraphView::SelectionInformationChanged()
{
  std::vector <mitk::TubeGraph::TubeDescriptorType> activeTubes = m_ActiveProperty->GetActiveTubes();
  if(activeTubes.size()==0)
    m_Controls.shortInformationLabel->setText("");
  else
  {
    mitk::TubeGraph::TubeDescriptorType root = m_ActiveTubeGraph->GetRootTube();
    /*double averageDiameter;
    for (int i = 0; i < activeTubes.size(); i++)
    {
    mitk::TubeGraph::EdgeDescriptorType edgeDescriptor = m_ActiveTubeGraph->GetEdgeDescriptorByVerices(activeTubes.at(i).first, activeTubes.at(i).second);
    mitk::TubeGraphEdge edge =  m_ActiveTubeGraph->GetEdge(edgeDescriptor);
    averageDiameter += edge.GetEdgeAverageDiameter(m_ActiveTubeGraph->GetVertex(activeTubes.at(i).first), m_ActiveTubeGraph->GetVertex(activeTubes.at(i).second));
    }
    averageDiameter = averageDiameter/activeTubes.size();*/

    QString information;
    information.append(QString("Number of Selected Tubes: %1\n").arg(activeTubes.size()));
    //information.append(QString("Average diameter\n").arg(averageDiameter));
    information.append(QString("Root Id: [%1,%2]").arg(root.first).arg(root.second));
    m_Controls.shortInformationLabel->setText(information);
  }
}
