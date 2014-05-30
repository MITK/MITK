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

#include "QmitkFunctionalityComponentContainer.h"
#include "ui_QmitkFunctionalityComponentContainerControls.h"

#include <QmitkDataStorageComboBox.h>
#include "mitkDataTreeFilterFunctions.h"
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include "mitkProperties.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <itkCommand.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtabwidget.h>

#include <vector>
#include <qwidget.h>

#include <QBoxLayout>


const QSizePolicy preferred(QSizePolicy::Preferred, QSizePolicy::Preferred);

QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *parentName, bool updateSelector, bool showSelector)
: QmitkBaseFunctionalityComponent(parent, parentName),
m_UpdateSelector(updateSelector),
m_ShowSelector(showSelector),
m_GUI(NULL),
m_Active(false),
m_SelectedItem(NULL),
m_FunctionalityComponentContainerGUI(NULL),
m_Parent(parent),
m_ComponentName("ComponentContainer"),
m_Spacer(NULL),
m_BackButton(NULL),
m_NextButton(NULL),
m_MaximumWidgedStackSize(-1)
{
  SetAvailability(true);
}

QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{

}

void QmitkFunctionalityComponentContainer::SetComponentName(QString name)
{
  m_ComponentName = name;
}

QString QmitkFunctionalityComponentContainer::GetComponentName()
{
  return m_ComponentName;
}

void QmitkFunctionalityComponentContainer::SetShowTreeNodeSelector(bool show)
{
  GetImageContent()->setShown(show);
}

QGroupBox*  QmitkFunctionalityComponentContainer::GetImageContent()
{
  return (QGroupBox*) m_FunctionalityComponentContainerGUI->m_ImageContent;
}

QWidget* QmitkFunctionalityComponentContainer::GetGUI()
{
  return m_GUI;
}

mitk::Image* QmitkFunctionalityComponentContainer::GetParentMitkImage()
{
  return m_ParentMitkImage;
}

QmitkDataStorageComboBox * QmitkFunctionalityComponentContainer::GetTreeNodeSelector()
{
  if(m_FunctionalityComponentContainerGUI)
  {
    return m_FunctionalityComponentContainerGUI->m_TreeNodeSelector;
  }
  else return NULL;
}

QmitkStdMultiWidget * QmitkFunctionalityComponentContainer::GetMultiWidget()
{
  return m_MulitWidget;
}

void QmitkFunctionalityComponentContainer::TreeChanged(const itk::EventObject&)
{
  if(IsActivated())
  {
    TreeChanged();
  }
  else
    TreeChanged();
}

void QmitkFunctionalityComponentContainer::TreeChanged()
{
  UpdateDataTreeComboBoxes();

  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->TreeChanged();
  }
}

void QmitkFunctionalityComponentContainer::UpdateDataTreeComboBoxes()
{
}

void QmitkFunctionalityComponentContainer::CreateConnections()
{
  if ( m_FunctionalityComponentContainerGUI )
  {
    connect( (QObject*)(m_FunctionalityComponentContainerGUI->m_TreeNodeSelector), SIGNAL(OnSelectionChanged (const mitk::DataNode *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataNode *)));
    connect( (QObject*)(m_FunctionalityComponentContainerGUI->m_ContainerBorder),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool)));
  }
}

void QmitkFunctionalityComponentContainer::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

mitk::DataStorage::Pointer QmitkFunctionalityComponentContainer::GetDataStorage()
{
  return m_DataStorage;
}

void QmitkFunctionalityComponentContainer::DataStorageChanged(mitk::DataStorage::Pointer ds)
{
  if(m_FunctionalityComponentContainerGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
      {
        functionalityComponent->DataStorageChanged(ds);
      }
    }
  }

  if(m_FunctionalityComponentContainerGUI)
  {
    if(!m_FunctionalityComponentContainerGUI->m_TreeNodeSelector)
      return;
    if(!m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->GetSelectedNode())
      return;
    if(!m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->GetSelectedNode()->GetData())
      return;
    m_ParentMitkImage = static_cast<mitk::Image*> (m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->GetSelectedNode()->GetData());


    if(m_FunctionalityComponentContainerGUI != NULL)
    {
      for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
      {
        QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
        if (functionalityComponent != NULL)
        {
          if(!m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->GetSelectedNode()->GetData())
            return;
          functionalityComponent->m_ParentMitkImage = static_cast<mitk::Image*> (m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->GetSelectedNode()->GetData());

        }
      }
    }
  }
}

void QmitkFunctionalityComponentContainer::ImageSelected(const mitk::DataNode* item)
{

  if(m_FunctionalityComponentContainerGUI != NULL)
  {
    mitk::DataNode::Pointer selectedItem = const_cast< mitk::DataNode*>(item);
    GetTreeNodeSelector()->SetSelectedNode(selectedItem);

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(item);
    }
  }

  if(m_FunctionalityComponentContainerGUI)
  {
      m_ParentMitkImage = static_cast<mitk::Image*> (item->GetData());


    if(m_FunctionalityComponentContainerGUI != NULL)
    {
      for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
      {
        QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
        if (functionalityComponent != NULL)
        {
          functionalityComponent->m_ParentMitkImage = static_cast<mitk::Image*> (item->GetData());
        }
      }
    }
  }
  TreeChanged();
}

void QmitkFunctionalityComponentContainer::CreateQtPartControl(QWidget*, mitk::DataStorage::Pointer dataStorage)
{
  if (m_FunctionalityComponentContainerGUI == NULL)
  {
    m_GUI = new QWidget;
    m_FunctionalityComponentContainerGUI = new Ui::QmitkFunctionalityComponentContainerGUI;
    m_FunctionalityComponentContainerGUI->setupUi(m_GUI);

    this->SetDataStorage(dataStorage);

    m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->SetDataStorage(dataStorage);
    m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));

    m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(0);
  }
  this->CreateConnections();
}

QGroupBox * QmitkFunctionalityComponentContainer::GetContentContainer()
{
  return m_FunctionalityComponentContainerGUI->m_ImageContent;
}

QGroupBox * QmitkFunctionalityComponentContainer::GetMainCheckBoxContainer()
{
  return m_FunctionalityComponentContainerGUI->m_ContainerBorder;
}

void QmitkFunctionalityComponentContainer::SetContentContainerVisibility(bool)
{
  if(GetMainCheckBoxContainer() != NULL)
  {
    if(GetMainCheckBoxContainer()->isChecked())
    {
      Activated();
    }
    else
    {
      Deactivated();
    }
  }
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    if(m_AddedChildList[i]->GetContentContainer() != NULL)
    {
      m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
    }
    if(m_AddedChildList[i]->GetMainCheckBoxContainer() != NULL)
    {
      m_AddedChildList[i]->GetMainCheckBoxContainer()->setChecked(GetMainCheckBoxContainer()->isChecked());
    }
    m_AddedChildList[i]->SetContentContainerVisibility(GetMainCheckBoxContainer()->isChecked());
  }
}

void QmitkFunctionalityComponentContainer::SetSelectorVisibility(bool visibility)
{
  if(m_GUI)
  {
    m_FunctionalityComponentContainerGUI->m_ImageContent->setShown(visibility);
  }
  m_ShowSelector = visibility;
}

void QmitkFunctionalityComponentContainer::Activated()
{
  TreeChanged();
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  }
}

void QmitkFunctionalityComponentContainer::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  }
}

void QmitkFunctionalityComponentContainer::AddComponent(QmitkFunctionalityComponentContainer* component)
{
  if(component!=NULL)
  {
    QWidget* componentWidget = component->CreateControlWidget(m_GUI);
    AddComponentListener(component);
    m_GUI->layout()->addWidget(componentWidget);
    component->CreateConnections();
    if(m_Spacer != NULL)
    {
      m_GUI->layout()->removeItem(m_Spacer);
    }
    QSpacerItem*  spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_Spacer = spacer;
    m_GUI->layout()->addItem( m_Spacer );
    m_GUI->repaint();
  }
}

QPushButton* QmitkFunctionalityComponentContainer::GetNextButton()
{
  return m_NextButton;
}

QPushButton* QmitkFunctionalityComponentContainer::GetBackButton()
{
  return m_BackButton;
}

void QmitkFunctionalityComponentContainer::AddComponent(QmitkFunctionalityComponentContainer* component, QString label, int stackPage)
{
  if(component!=NULL)
  {
    QWidget* visibleWidget = m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget();
    int idVisibleWidget = m_FunctionalityComponentContainerGUI->m_WidgetStack->indexOf(visibleWidget);
    if(idVisibleWidget > m_MaximumWidgedStackSize)
    {
      m_MaximumWidgedStackSize = idVisibleWidget;
    }
    if(m_MaximumWidgedStackSize < stackPage)
    {
      QWidget* w = new QWidget(m_FunctionalityComponentContainerGUI->m_WidgetStack);
      m_FunctionalityComponentContainerGUI->m_WidgetStack->insertTab(stackPage, w, label);
      m_MaximumWidgedStackSize++;
      m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(stackPage);
      visibleWidget = m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget();
      idVisibleWidget = m_FunctionalityComponentContainerGUI->m_WidgetStack->indexOf(visibleWidget);
      new QVBoxLayout(visibleWidget);
    }

    QLayout* layout;
    if(m_FunctionalityComponentContainerGUI->m_WidgetStack->layout() == 0)
    {
    layout = new QVBoxLayout( (QWidget*)(m_FunctionalityComponentContainerGUI->m_WidgetStack));
    }
    else
    {
      layout = m_FunctionalityComponentContainerGUI->m_WidgetStack->layout();
    }

    component->CreateQtPartControl(m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget(), this->m_DataStorage);
    QWidget* componentWidget = component->GetGUI();
    AddComponentListener(component);
    m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(stackPage);
    QWidget* theCurrentWidget=m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget();
    QLayout* theCurrentLayout = theCurrentWidget->layout();
    theCurrentLayout->addWidget(componentWidget);

    m_FunctionalityComponentContainerGUI->m_WidgetStack->setShown(true);
    m_FunctionalityComponentContainerGUI->m_WidgetStack->updateGeometry();
    m_FunctionalityComponentContainerGUI->m_WidgetStack->layout()->activate();
    componentWidget->setShown(true);
    component->CreateConnections();

    m_GUI->repaint();
  }
}

void QmitkFunctionalityComponentContainer::CreateNavigationButtons()
{
  QWidget* funcWidget = (QWidget*)m_FunctionalityComponentContainerGUI;
  QLayout *functionalityLayout = funcWidget->layout();
  QBoxLayout * buttonLayout = new QHBoxLayout(funcWidget);
  if ( QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(functionalityLayout) )
  {
    boxLayout->addLayout( buttonLayout );
  }
  if(m_BackButton==NULL)
  {
    m_BackButton = new QPushButton("<<", (QWidget*)(m_FunctionalityComponentContainerGUI));
  }
  if(m_NextButton==NULL)
  {
    m_NextButton = new QPushButton(">>", (QWidget*)(m_FunctionalityComponentContainerGUI));
  }
  buttonLayout->addWidget(m_BackButton);
  buttonLayout->addWidget(m_NextButton);
  m_BackButton->setShown(true);
  m_NextButton->setShown(true);
  m_GUI->layout()->activate();
  m_GUI->repaint();

  m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(1);
  SetWizardText("");
  GetImageContent()->updateGeometry();
  if(m_Spacer != NULL)
  {
    m_GUI->layout()->removeItem(m_Spacer);
  }
  QSpacerItem*  spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  m_Spacer = spacer;
  m_GUI->layout()->addItem( m_Spacer );
  m_GUI->updateGeometry();
  m_GUI->repaint();
}

void QmitkFunctionalityComponentContainer::SetWizardText(const QString&)
{
  GetImageContent()->updateGeometry();
  GetImageContent()->repaint();
  m_GUI->updateGeometry();
  m_GUI->layout()->activate();
  m_GUI->repaint();

}

Ui::QmitkFunctionalityComponentContainerGUI* QmitkFunctionalityComponentContainer::GetFunctionalityComponentContainerGUI()
{
  return m_FunctionalityComponentContainerGUI;
}

