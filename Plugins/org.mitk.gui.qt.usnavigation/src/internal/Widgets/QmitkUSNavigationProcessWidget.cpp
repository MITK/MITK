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

#include "QmitkUSNavigationProcessWidget.h"
#include "ui_QmitkUSNavigationProcessWidget.h"

#include "../NavigationStepWidgets/QmitkUSAbstractNavigationStep.h"
#include "../SettingsWidgets/QmitkUSNavigationAbstractSettingsWidget.h"

#include "mitkDataNode.h"
#include "mitkNavigationDataToNavigationDataFilter.h"

#include <QTimer>
#include <QSignalMapper>
#include <QShortcut>

QmitkUSNavigationProcessWidget::QmitkUSNavigationProcessWidget(QWidget* parent) :
  QWidget(parent),
  m_SettingsWidget(0),
  m_BaseNode(mitk::DataNode::New()), m_CurrentTabIndex(0), m_CurrentMaxStep(0),
  m_ImageAlreadySetToNode(false),
  m_ReadySignalMapper(new QSignalMapper(this)), m_NoLongerReadySignalMapper(new QSignalMapper(this)),
  m_StdMultiWidget(0),
  m_UsePlanningStepWidget(false),
  ui(new Ui::QmitkUSNavigationProcessWidget)
{
  m_Parent = parent;

  ui->setupUi(this);

  // remove the default page
  ui->stepsToolBox->removeItem(0);

  //set shortcuts
  QShortcut *nextShortcut = new QShortcut(QKeySequence("F10"), parent);
  QShortcut *prevShortcut = new QShortcut(QKeySequence("F11"), parent);
  connect(nextShortcut, SIGNAL(activated()), this, SLOT(OnNextButtonClicked()));
  connect(prevShortcut, SIGNAL(activated()), this, SLOT(OnPreviousButtonClicked()));

  //connect other slots
  connect( ui->restartStepButton, SIGNAL(clicked()), this, SLOT(OnRestartStepButtonClicked()) );
  connect( ui->previousButton, SIGNAL(clicked()), this, SLOT(OnPreviousButtonClicked()) );
  connect( ui->nextButton, SIGNAL(clicked()), this, SLOT(OnNextButtonClicked()) );
  connect( ui->stepsToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)) );
  connect (ui->settingsButton, SIGNAL(clicked()), this, SLOT(OnSettingsButtonClicked()) );
  connect( m_ReadySignalMapper, SIGNAL(mapped(int)), this, SLOT(OnStepReady(int)) );
  connect( m_NoLongerReadySignalMapper, SIGNAL(mapped(int)), this, SLOT(OnStepNoLongerReady(int)) );

  ui->settingsFrameWidget->setHidden(true);
}

QmitkUSNavigationProcessWidget::~QmitkUSNavigationProcessWidget()
{
  ui->stepsToolBox->blockSignals(true);

  for ( NavigationStepVector::iterator it = m_NavigationSteps.begin();
    it != m_NavigationSteps.end(); ++it )
  {
    if ( (*it)->GetNavigationStepState() > QmitkUSAbstractNavigationStep::State_Stopped ) { (*it)->StopStep(); }
    delete *it;
  }
  m_NavigationSteps.clear();

  if ( m_SettingsNode.IsNotNull() && m_DataStorage.IsNotNull() )
  {
    m_DataStorage->Remove(m_SettingsNode);
  }

  delete ui;
}

void QmitkUSNavigationProcessWidget::EnableInteraction(bool enable)
{
  if (enable)
  {
    ui->restartStepButton->setEnabled(true);
    ui->previousButton->setEnabled(true);
    ui->nextButton->setEnabled(true);
    ui->stepsToolBox->setEnabled(true);
  }
  else
  {
    ui->restartStepButton->setEnabled(false);
    ui->previousButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->stepsToolBox->setEnabled(false);
  }
}

void QmitkUSNavigationProcessWidget::SetDataStorage(itk::SmartPointer<mitk::DataStorage> dataStorage)
{
  m_DataStorage = dataStorage;

  if ( dataStorage.IsNull() )
  {
    mitkThrow() << "Data Storage must not be null for QmitkUSNavigationProcessWidget.";
  }

  // test if base node is already in the data storage and add it if not
  m_BaseNode = dataStorage->GetNamedNode(QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
  if ( m_BaseNode.IsNull() )
  {
    m_BaseNode = mitk::DataNode::New();
    m_BaseNode->SetName(QmitkUSAbstractNavigationStep::DATANAME_BASENODE);
    dataStorage->Add(m_BaseNode);
  }

  // base node and image stream node may be the same node
  if ( strcmp(QmitkUSAbstractNavigationStep::DATANAME_BASENODE, QmitkUSAbstractNavigationStep::DATANAME_IMAGESTREAM) != 0)
  {
    m_ImageStreamNode = dataStorage->GetNamedNode(QmitkUSAbstractNavigationStep::DATANAME_IMAGESTREAM);
    if (m_ImageStreamNode.IsNull())
    {
      // Create Node for US Stream
      m_ImageStreamNode = mitk::DataNode::New();
      m_ImageStreamNode->SetName(QmitkUSAbstractNavigationStep::DATANAME_IMAGESTREAM);
      dataStorage->Add(m_ImageStreamNode);
    }
  }
  else
  {
    m_ImageStreamNode = m_BaseNode;
  }


  m_SettingsNode =
    dataStorage->GetNamedDerivedNode(QmitkUSAbstractNavigationStep::DATANAME_SETTINGS, m_BaseNode);

  if ( m_SettingsNode.IsNull() )
  {
    m_SettingsNode = mitk::DataNode::New();
    m_SettingsNode->SetName(QmitkUSAbstractNavigationStep::DATANAME_SETTINGS);
    dataStorage->Add(m_SettingsNode, m_BaseNode);
  }

  if (m_SettingsWidget) { m_SettingsWidget->SetSettingsNode(m_SettingsNode); }

}

void QmitkUSNavigationProcessWidget::SetSettingsWidget(QmitkUSNavigationAbstractSettingsWidget* settingsWidget)
{
  // disconnect slots to settings widget if there was a widget before
  if ( m_SettingsWidget )
  {
    disconnect( ui->settingsSaveButton, SIGNAL(clicked()), m_SettingsWidget, SLOT(OnSave()) );
    disconnect( ui->settingsCancelButton, SIGNAL(clicked()), m_SettingsWidget, SLOT(OnCancel()) );

    disconnect (m_SettingsWidget, SIGNAL(Saved()), this, SLOT(OnSettingsWidgetReturned()) );
    disconnect (m_SettingsWidget, SIGNAL(Canceled()), this, SLOT(OnSettingsWidgetReturned()) );
    disconnect (m_SettingsWidget, SIGNAL(SettingsChanged(itk::SmartPointer<mitk::DataNode>)), this, SLOT(OnSettingsChanged(itk::SmartPointer<mitk::DataNode>)) );

    ui->settingsWidget->removeWidget(m_SettingsWidget);
  }

  m_SettingsWidget = settingsWidget;
  if ( m_SettingsWidget )
  {
    m_SettingsWidget->LoadSettings();

    connect( ui->settingsSaveButton, SIGNAL(clicked()), m_SettingsWidget, SLOT(OnSave()) );
    connect( ui->settingsCancelButton, SIGNAL(clicked()), m_SettingsWidget, SLOT(OnCancel()) );

    connect (m_SettingsWidget, SIGNAL(Saved()), this, SLOT(OnSettingsWidgetReturned()) );
    connect (m_SettingsWidget, SIGNAL(Canceled()), this, SLOT(OnSettingsWidgetReturned()) );
    connect (m_SettingsWidget, SIGNAL(SettingsChanged(itk::SmartPointer<mitk::DataNode>)), this, SLOT(OnSettingsChanged(itk::SmartPointer<mitk::DataNode>)) );

    if ( m_SettingsNode.IsNotNull() ) { m_SettingsWidget->SetSettingsNode(m_SettingsNode, true); }

    ui->settingsWidget->addWidget(m_SettingsWidget);
  }
  ui->settingsButton->setEnabled(m_SettingsWidget != 0);
}

void QmitkUSNavigationProcessWidget::SetNavigationSteps(NavigationStepVector navigationSteps)
{
  disconnect( this, SLOT(OnTabChanged(int)) );

  for ( int n = ui->stepsToolBox->count()-1; n >= 0; --n )
  {
    ui->stepsToolBox->removeItem(n);
  }

  connect( ui->stepsToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)) );

  m_NavigationSteps.clear();

  m_NavigationSteps = navigationSteps;

  this->InitializeNavigationStepWidgets();

  // notify all navigation step widgets about the current settings
  for (NavigationStepIterator it = m_NavigationSteps.begin(); it != m_NavigationSteps.end(); ++it)
  {
    (*it)->OnSettingsChanged(m_SettingsNode);
  }
}

void QmitkUSNavigationProcessWidget::ResetNavigationProcess()
{
  MITK_INFO("QmitkUSNavigationProcessWidget") << "Resetting navigation process.";

  ui->stepsToolBox->blockSignals(true);
  for ( int n = 0; n <= m_CurrentMaxStep; ++n )
  {
    m_NavigationSteps.at(n)->StopStep();
    if ( n > 0 ) { ui->stepsToolBox->setItemEnabled(n, false); }
  }
  ui->stepsToolBox->blockSignals(false);

  m_CurrentMaxStep = 0;

  ui->stepsToolBox->setCurrentIndex(0);

  if ( m_NavigationSteps.size() > 0 )
  {
    m_NavigationSteps.at(0)->ActivateStep();
  }

  this->UpdatePrevNextButtons();
}

void QmitkUSNavigationProcessWidget::UpdateNavigationProgress()
{
  if ( m_CombinedModality.IsNotNull() && !m_CombinedModality->GetIsFreezed() )
  {
    m_CombinedModality->Modified();
    m_CombinedModality->Update();

    if ( m_LastNavigationDataFilter.IsNotNull() ) { m_LastNavigationDataFilter->Update(); }

    mitk::Image::Pointer image = m_CombinedModality->GetOutput();
    // make sure that always the current image is set to the data node
    if ( image.IsNotNull() && m_ImageStreamNode->GetData() != image.GetPointer() && image->IsInitialized() )
    {
      m_ImageStreamNode->SetData(image);
      m_ImageAlreadySetToNode = true;
    }
  }

  if ( m_CurrentTabIndex > 0 && static_cast<unsigned int>(m_CurrentTabIndex) < m_NavigationSteps.size() )
  {
    m_NavigationSteps.at(m_CurrentTabIndex)->Update();
  }
}

void QmitkUSNavigationProcessWidget::OnNextButtonClicked()
{
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetIsFreezed()) {return;} //no moving through steps when the modality is NULL or frozen

  int currentIndex = ui->stepsToolBox->currentIndex();
  if (currentIndex >= m_CurrentMaxStep)
  {
    MITK_WARN << "Next button clicked though no next tab widget is available.";
    return;
  }

  ui->stepsToolBox->setCurrentIndex(++currentIndex);

  this->UpdatePrevNextButtons();
}

void QmitkUSNavigationProcessWidget::OnPreviousButtonClicked()
{
  if (m_CombinedModality.IsNotNull() && m_CombinedModality->GetIsFreezed()) {return;} //no moving through steps when the modality is NULL or frozen

  int currentIndex = ui->stepsToolBox->currentIndex();
  if (currentIndex <= 0)
  {
    MITK_WARN << "Previous button clicked though no previous tab widget is available.";
    return;
  }

  ui->stepsToolBox->setCurrentIndex(--currentIndex);

  this->UpdatePrevNextButtons();
}

void QmitkUSNavigationProcessWidget::OnRestartStepButtonClicked()
{
  MITK_INFO("QmitkUSNavigationProcessWidget") << "Restarting step "
    << m_CurrentTabIndex << " (" << m_NavigationSteps.at(m_CurrentTabIndex)->GetTitle().toStdString() << ").";

  m_NavigationSteps.at(ui->stepsToolBox->currentIndex())->RestartStep();
  m_NavigationSteps.at(ui->stepsToolBox->currentIndex())->ActivateStep();
}

void QmitkUSNavigationProcessWidget::OnTabChanged(int index)
{
  if ( index < 0 || index >= static_cast<int>(m_NavigationSteps.size()) )
  {
    return;
  }
  else if ( m_CurrentTabIndex == index )
  {
    // just activate the step if it is the same step againg
    m_NavigationSteps.at(index)->ActivateStep();
    return;
  }

  MITK_INFO("QmitkUSNavigationProcessWidget") << "Activating navigation step "
    << index << " (" << m_NavigationSteps.at(index)->GetTitle().toStdString() <<").";

  if (index > m_CurrentTabIndex)
  {
    this->UpdateFilterPipeline();

    // finish all previous steps to make sure that all data is valid
    for (int n = m_CurrentTabIndex; n < index; ++n)
    {
      m_NavigationSteps.at(n)->FinishStep();
    }
  }

  // deactivate the previously active step
  if ( m_CurrentTabIndex > 0 && m_NavigationSteps.size() > static_cast<unsigned int>(m_CurrentTabIndex) )
  {
    m_NavigationSteps.at(m_CurrentTabIndex)->DeactivateStep();
  }

  // start step of the current tab if it wasn't started before
  if ( m_NavigationSteps.at(index)->GetNavigationStepState() == QmitkUSAbstractNavigationStep::State_Stopped )
  {
    m_NavigationSteps.at(index)->StartStep();
  }

  m_NavigationSteps.at(index)->ActivateStep();

  if (static_cast<unsigned int>(index) < m_NavigationSteps.size())
    ui->restartStepButton->setEnabled(m_NavigationSteps.at(index)->GetIsRestartable());

  this->UpdatePrevNextButtons();

  m_CurrentTabIndex = index;
  emit SignalActiveNavigationStepChanged(index);
}

void QmitkUSNavigationProcessWidget::OnSettingsButtonClicked()
{
  this->SetSettingsWidgetVisible(true);
}

void QmitkUSNavigationProcessWidget::OnSettingsWidgetReturned()
{
  this->SetSettingsWidgetVisible(false);
}

void QmitkUSNavigationProcessWidget::OnSettingsNodeChanged(itk::SmartPointer<mitk::DataNode> dataNode)
{
  if ( m_SettingsWidget ) m_SettingsWidget->SetSettingsNode(dataNode);
}

void QmitkUSNavigationProcessWidget::OnStepReady(int index)
{
  if (m_CurrentMaxStep <= index)
  {
    m_CurrentMaxStep = index + 1;
    this->UpdatePrevNextButtons();
    for (int n = 0; n <= m_CurrentMaxStep; ++n)
    {
      ui->stepsToolBox->setItemEnabled(n, true);
    }
  }

  emit SignalNavigationStepFinished(index, true);
}

void QmitkUSNavigationProcessWidget::OnStepNoLongerReady(int index)
{
  if (m_CurrentMaxStep > index)
  {
    m_CurrentMaxStep = index;
    this->UpdatePrevNextButtons();
    this->UpdateFilterPipeline();
    for (int n = m_CurrentMaxStep+1; n < ui->stepsToolBox->count(); ++n)
    {
      ui->stepsToolBox->setItemEnabled(n, false);
      m_NavigationSteps.at(n)->StopStep();
    }
  }

  emit SignalNavigationStepFinished(index, false);
}

void QmitkUSNavigationProcessWidget::OnCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality> combinedModality)
{
  m_CombinedModality = combinedModality;
  m_ImageAlreadySetToNode = false;

  if ( combinedModality.IsNotNull() )
  {
    if ( combinedModality->GetNavigationDataSource().IsNull() )
    {
      MITK_WARN << "There is no navigation data source set for the given combined modality.";
      return;
    }

    this->UpdateFilterPipeline();
  }

  for (NavigationStepIterator it = m_NavigationSteps.begin(); it != m_NavigationSteps.end(); ++it)
  {
    (*it)->SetCombinedModality(combinedModality);
  }

  emit SignalCombinedModalityChanged(combinedModality);
}

void QmitkUSNavigationProcessWidget::OnSettingsChanged(const mitk::DataNode::Pointer dataNode)
{
  static bool methodEntered = false;
  if ( methodEntered )
  {
    MITK_WARN("QmitkUSNavigationProcessWidget") << "Ignoring recursive call to 'OnSettingsChanged()'. "
      << "Make sure to no emit 'SignalSettingsNodeChanged' in an 'OnSettingsChanged()' method.";
    return;
  }
  methodEntered = true;

  std::string application;
  if ( dataNode->GetStringProperty("settings.application", application) )
  {
    QString applicationQString = QString::fromStdString(application);
    if ( applicationQString != ui->titleLabel->text() )
    {
      ui->titleLabel->setText(applicationQString);
    }
  }

  // notify all navigation step widgets about the changed settings
  for (NavigationStepIterator it = m_NavigationSteps.begin(); it != m_NavigationSteps.end(); ++it)
  {
    (*it)->OnSettingsChanged(dataNode);
  }

  emit SignalSettingsChanged(dataNode);

  methodEntered = false;
}

void QmitkUSNavigationProcessWidget::InitializeNavigationStepWidgets()
{
  // do not listen for steps tool box signal during insertion of items into tool box
  disconnect( ui->stepsToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)) );

  m_CurrentMaxStep = 0;

  mitk::DataStorage::Pointer dataStorage = m_DataStorage;

  for (unsigned int n = 0; n < m_NavigationSteps.size(); ++n)
  {
    QmitkUSAbstractNavigationStep* curNavigationStep = m_NavigationSteps.at(n);

    curNavigationStep->SetDataStorage(dataStorage);

    connect( curNavigationStep, SIGNAL(SignalReadyForNextStep()), m_ReadySignalMapper, SLOT(map()));
    connect( curNavigationStep, SIGNAL(SignalNoLongerReadyForNextStep()), m_NoLongerReadySignalMapper, SLOT(map()) );
    connect( curNavigationStep, SIGNAL(SignalCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>)), this, SLOT(OnCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>)) );
    connect( curNavigationStep, SIGNAL(SignalIntermediateResult(const itk::SmartPointer<mitk::DataNode>)), this, SIGNAL(SignalIntermediateResult(const itk::SmartPointer<mitk::DataNode>)) );
    connect( curNavigationStep, SIGNAL(SignalSettingsNodeChanged(itk::SmartPointer<mitk::DataNode>)), this, SLOT(OnSettingsNodeChanged(itk::SmartPointer<mitk::DataNode>)) );

    m_ReadySignalMapper->setMapping(curNavigationStep, n);
    m_NoLongerReadySignalMapper->setMapping(curNavigationStep, n);

    ui->stepsToolBox->insertItem(n, curNavigationStep, QString("Step ") + QString::number(n+1) + ": " + curNavigationStep->GetTitle());
    if ( n > 0 ) { ui->stepsToolBox->setItemEnabled(n, false); }
  }

  ui->restartStepButton->setEnabled(m_NavigationSteps.at(0)->GetIsRestartable());
  ui->stepsToolBox->setCurrentIndex(0);

  // activate the first navigation step widgets
  if ( ! m_NavigationSteps.empty() ) { m_NavigationSteps.at(0)->ActivateStep(); }

  // after filling the steps tool box the signal is interesting again
  connect( ui->stepsToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)) );

  this->UpdateFilterPipeline();
}

void QmitkUSNavigationProcessWidget::UpdatePrevNextButtons()
{
  int currentIndex = ui->stepsToolBox->currentIndex();

  ui->previousButton->setEnabled(currentIndex > 0);
  ui->nextButton->setEnabled(currentIndex < m_CurrentMaxStep);
}

void QmitkUSNavigationProcessWidget::UpdateFilterPipeline()
{
  if ( m_CombinedModality.IsNull() ) { return; }

  std::vector<mitk::NavigationDataToNavigationDataFilter::Pointer> filterList;

  mitk::NavigationDataSource::Pointer navigationDataSource = m_CombinedModality->GetNavigationDataSource();

  for (unsigned int n = 0; n <= m_CurrentMaxStep && n < m_NavigationSteps.size(); ++n)
  {
    QmitkUSAbstractNavigationStep::FilterVector filter = m_NavigationSteps.at(n)->GetFilter();
    if ( ! filter.empty() ) { filterList.insert(filterList.end(), filter.begin(), filter.end()); }
  }

  if ( ! filterList.empty() )
  {
    for (unsigned int n = 0; n < navigationDataSource->GetNumberOfOutputs(); ++n)
    {
      filterList.at(0)->SetInput(n, navigationDataSource->GetOutput(n));
    }

    for (std::vector<mitk::NavigationDataToNavigationDataFilter::Pointer>::iterator it = filterList.begin()+1;
      it != filterList.end(); ++it)
    {
      std::vector<mitk::NavigationDataToNavigationDataFilter::Pointer>::iterator prevIt = it-1;

      for (unsigned int n = 0; n < (*prevIt)->GetNumberOfOutputs(); ++n)
      {
        (*it)->SetInput(n, (*prevIt)->GetOutput(n));
      }
    }

    m_LastNavigationDataFilter = filterList.at(filterList.size()-1);
  }
  else
  {
    m_LastNavigationDataFilter = navigationDataSource.GetPointer();
  }
}

void QmitkUSNavigationProcessWidget::SetSettingsWidgetVisible(bool visible)
{
 ui->settingsFrameWidget->setVisible(visible);
 ui->stepsToolBox->setHidden(visible);
 ui->settingsButton->setHidden(visible);
 ui->restartStepButton->setHidden(visible);
 ui->previousButton->setHidden(visible);
 ui->nextButton->setHidden(visible);
}

void QmitkUSNavigationProcessWidget::FinishCurrentNavigationStep()
{
  int currentIndex = ui->stepsToolBox->currentIndex();
  QmitkUSAbstractNavigationStep* curNavigationStep = m_NavigationSteps.at(currentIndex);
  curNavigationStep->FinishStep();
}
