/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkFunctionalityComponentContainer.h"
//#include "QmitkBaseFunctionalityComponent.h"
#include "ui_QmitkFunctionalityComponentContainerControls.h"

#include <QmitkDataStorageComboBox.h>
#include "mitkDataTreeFilterFunctions.h"
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include "mitkProperties.h"

/*****Qt-Elements***/
#include <qgroupbox.h>
#include <qlayout.h>
#include <itkCommand.h>
//#include <qobjectlist.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtabwidget.h>
//#include <qlayout.h>

#include <vector>
#include <qwidget.h>

#include <QBoxLayout>


const QSizePolicy preferred(QSizePolicy::Preferred, QSizePolicy::Preferred);

/***************       CONSTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::QmitkFunctionalityComponentContainer(QObject *parent, const char *parentName, bool updateSelector, bool showSelector)
: QmitkBaseFunctionalityComponent(parent, parentName),
m_UpdateSelector(updateSelector), 
m_ShowSelector(showSelector),
m_GUI(NULL),
//m_ParentMitkImage(NULL),
//m_ParentMitkImageIterator(NULL),
m_Active(false),
m_SelectedItem(NULL),
m_FunctionalityComponentContainerGUI(NULL),
m_Parent(parent), 
//m_ParentName(parentName), 
m_ComponentName("ComponentContainer"),
//m_MultiWidget(mitkStdMultiWidget),
m_Spacer(NULL),
//m_MulitWidget(this->GetActiveStdMultiWidget()),
m_BackButton(NULL),
m_NextButton(NULL),
m_MaximumWidgedStackSize(-1)
{
  SetAvailability(true);
}

/***************        DESTRUCTOR      ***************/
QmitkFunctionalityComponentContainer::~QmitkFunctionalityComponentContainer()
{

}

/***************   SET COMPONENT NAME   ***************/
void QmitkFunctionalityComponentContainer::SetComponentName(QString name)
{
  m_ComponentName = name;
}

/***************   GET COMPONENT NAME   ***************/
QString QmitkFunctionalityComponentContainer::GetComponentName()
{
  return m_ComponentName;
}


/************* SET SHOW TREE NODE SELECTOR ***********/
void QmitkFunctionalityComponentContainer::SetShowTreeNodeSelector(bool show)
{
  GetImageContent()->setShown(show);
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkFunctionalityComponentContainer::GetImageContent()
{
  return (QGroupBox*) m_FunctionalityComponentContainerGUI->m_ImageContent;
}

/***************         GET GUI        ***************/
QWidget* QmitkFunctionalityComponentContainer::GetGUI()
{
  return m_GUI;
}

/*************** GET PARTENT MITK IMAGE ***************/
mitk::Image* QmitkFunctionalityComponentContainer::GetParentMitkImage()
{
  return m_ParentMitkImage;
}
/******** ******* GET TREE NODE SELECTOR ***************/
QmitkDataStorageComboBox * QmitkFunctionalityComponentContainer::GetTreeNodeSelector()
{
  if(m_FunctionalityComponentContainerGUI)
  {
    return m_FunctionalityComponentContainerGUI->m_TreeNodeSelector;
  }
  else return NULL;
}

/******** *******    GET MULTI WIDGET    ***************/
QmitkStdMultiWidget * QmitkFunctionalityComponentContainer::GetMultiWidget()
{
  return m_MulitWidget;
}

/*************** TREE CHANGED ( EVENT ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged(const itk::EventObject & /*treeChangedEvent*/)
{
  if(IsActivated())
  {
    TreeChanged();
  }
  else
    TreeChanged();
}

/*************** TREE CHANGED (       ) ***************/
void QmitkFunctionalityComponentContainer::TreeChanged()
{
  UpdateDataTreeComboBoxes();

  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->TreeChanged();
  } 
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkFunctionalityComponentContainer::UpdateDataTreeComboBoxes()
{
  //if(GetTreeNodeSelector() != NULL)
  //{
  //  m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->Update();
  //}
}

/***************       CONNECTIONS      ***************/
void QmitkFunctionalityComponentContainer::CreateConnections()
{
  if ( m_FunctionalityComponentContainerGUI )
  {
    connect( (QObject*)(m_FunctionalityComponentContainerGUI->m_TreeNodeSelector), SIGNAL(OnSelectionChanged (const mitk::DataTreeNode *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeNode *)));
    connect( (QObject*)(m_FunctionalityComponentContainerGUI->m_ContainerBorder),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool)));    
  }
}

/** \brief Method to set the DataStorage*/
void QmitkFunctionalityComponentContainer::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

/** \brief Method to get the DataStorage*/
mitk::DataStorage::Pointer QmitkFunctionalityComponentContainer::GetDataStorage()
{
  return m_DataStorage;
}

/***************     DATA STORAGE CHANGED     ***************/
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
  //TreeChanged(;)
}

/***************     IMAGE SELECTED     ***************/
void QmitkFunctionalityComponentContainer::ImageSelected(const mitk::DataTreeNode* item)
{

  if(m_FunctionalityComponentContainerGUI != NULL)
  {
    mitk::DataTreeNode::Pointer selectedItem = const_cast< mitk::DataTreeNode*>(item);
    GetTreeNodeSelector()->SetSelectedNode(selectedItem);

//    m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->changeItem();
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        //functionalityComponent->GetTreeNodeSelector()->SetSelectedNode(selectedItem);
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

/*************** CREATE CONTAINER WIDGET **************/
void QmitkFunctionalityComponentContainer::CreateQtPartControl(QWidget * /*parent*/, mitk::DataStorage::Pointer dataStorage)
{
  if (m_FunctionalityComponentContainerGUI == NULL)
  {
    m_GUI = new QWidget;
    m_FunctionalityComponentContainerGUI = new Ui::QmitkFunctionalityComponentContainerGUI;
    m_FunctionalityComponentContainerGUI->setupUi(m_GUI);
    //m_GUI = parent;

     this->SetDataStorage(dataStorage);

    m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->SetDataStorage(dataStorage);
    m_FunctionalityComponentContainerGUI->m_TreeNodeSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));
      
m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(0);
    
    //m_FunctionalityComponentContainerGUI->m_TreeNodeSelector()->SetDataTree(GetDataTreeIterator());
    //m_FunctionalityComponentContainerGUI->GetContainerBorder()->setTitle("<bold>Select Image<\bold>");
    //m_FunctionalityComponentContainerGUI->GetContainerBorder()->setLineWidth(0);
  }
  this->CreateConnections();
  //m_MulitWidget = parent->GetActiveMultiWidget();
  //m_FunctionalityComponentContainerGUI->m_TreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkFunctionalityComponentContainer::GetContentContainer()
{
  return m_FunctionalityComponentContainerGUI->m_ImageContent;
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkFunctionalityComponentContainer::GetMainCheckBoxContainer()
{
  return m_FunctionalityComponentContainerGUI->m_ContainerBorder;
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
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

/************** SET SELECTOR VISIBILITY ***************/
void QmitkFunctionalityComponentContainer::SetSelectorVisibility(bool visibility)
{
  if(m_GUI)
  {
    m_FunctionalityComponentContainerGUI->m_ImageContent->setShown(visibility);
  }
  m_ShowSelector = visibility;
}

/***************        ACTIVATED       ***************/
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

/***************      DEACTIVATED       ***************/
void QmitkFunctionalityComponentContainer::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 
}

/***************      ADD COMPONENT     ***************/
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
/** \brief Method to return the NextButton to switch to the next widgetStackPage*/
QPushButton* QmitkFunctionalityComponentContainer::GetNextButton()
{
  return m_NextButton;
}

/** \brief Method to return the BackButton to switch to the last widgetStackPage*/
QPushButton* QmitkFunctionalityComponentContainer::GetBackButton()
{
  return m_BackButton;
}


/***************      ADD COMPONENT     ***************/
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
	  // QT3: new QVBoxLayout(visibleWidget, QBoxLayout::TopToBottom);
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
      //CreateControlWidget(m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget());
    AddComponentListener(component);
    //QTabWidget* myTabWidget = 
    //m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(stackPage); 
    m_FunctionalityComponentContainerGUI->m_WidgetStack->setCurrentIndex(stackPage);
    QWidget* theCurrentWidget=m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget();
    QLayout* theCurrentLayout = theCurrentWidget->layout();
    theCurrentLayout->addWidget(componentWidget);

    //m_FunctionalityComponentContainerGUI->m_WidgetStack->currentWidget()->layout()->addWidget(componentWidget);
//    QWidget* myWidget = myTabWidget->page(stackPage);
  //  QLayout* myLayout = myWidget->layout();
   // myWidget->layout()->add(componentWidget);

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
  //QBoxLayout * buttonLayout = new QHBoxLayout(GetImageContent()->layout());
  QWidget* funcWidget = (QWidget*)m_FunctionalityComponentContainerGUI;
  QLayout *functionalityLayout = funcWidget->layout();
  QBoxLayout * buttonLayout = new QHBoxLayout(funcWidget);
  if ( QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(functionalityLayout) )
  {
    boxLayout->addLayout( buttonLayout );
  }
  //if(m_BackButton==NULL)
  //{
  //  m_BackButton = new QPushButton("<<", GetImageContent());
  //}
  //if(m_NextButton==NULL)
  //{
  //  m_NextButton = new QPushButton(">>", GetImageContent());
  //}
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
  //m_GUI->layout()->addChildLayout(buttonLayout);
  m_BackButton->setShown(true);
  m_NextButton->setShown(true);
  //m_GUI->layout()->AlignTop;
  m_GUI->layout()->activate();
  m_GUI->repaint();

//  connect( (QObject*)(m_NextButton),  SIGNAL(pressed()), (QObject*) this, SLOT(NextButtonPressed())); 
//  connect( (QObject*)(m_BackButton),  SIGNAL(pressed()), (QObject*) this, SLOT(BackButtonPressed())); 

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

void QmitkFunctionalityComponentContainer::SetWizardText(const QString& /*text*/)
{
  //m_FunctionalityComponentContainerGUI->GetWizardTextLabel()->setText(text);
  //m_FunctionalityComponentContainerGUI->GetWizardTextLabel()->setAlignment(Qt::WordBreak);
  GetImageContent()->updateGeometry();
  GetImageContent()->repaint();
  m_GUI->updateGeometry();
  //m_GUI->layout()->AlignTop;
  m_GUI->layout()->activate();
  m_GUI->repaint();

}

//void QmitkFunctionalityComponentContainer::ChooseWizardText(int page)
//{
//  switch(page)
//  {
//  case 1:
//    SetWizardText("<b>Step 1   Minimize the dataset:</b><br><b>a)</b> If you have a 4D-Dataset export one time step with the TimeStepExporter.<br><b>b)</b> Crop the image by selecting a shape and placing it (STRG + Mouse) around the relevant areas. Everything around the shape will be cut off.");
//    break;
//  case 2:
//    SetWizardText("<b>Step 2   Create an STL-Model:</b><br><b>a)</b> Choose a threshold with the Threshold Finder where all areas that shall be used for the model are marked (green).<br><b>b)</b> If there are special areas that have to be manipulated use the PixelGreyValueManipulator and do not forget to select the new created image.><br><b>c)</b> Create a Surface with the SurfaceCreator.");
//    break;
//  case 3:
//    SetWizardText("<b>Step 3 Surface Finish:</b><br><b>a)</b> If you have your surface you can use the connectivity filter where you can mark all connected areas in different colours or delete alle areas instead of the biggest.");
//    break;
//  default:
//    SetWizardText("<b>Step 1   Minimize the dataset:</b><br><b>a)</b> If you have a 4D-Dataset export one time step with the TimeStepExporter.<br><b>b)</b> Crop the image by selecting a shape and placing it (STRG + Mouse) around the relevant areas. Everything around the shape will be cut off.");
//    break;
//  };
//}

//void QmitkFunctionalityComponentContainer::NextButtonPressed()
//{
//  int actualPage = m_FunctionalityComponentContainerGUI->m_WidgetStack->id(m_FunctionalityComponentContainerGUI->m_WidgetStack->visibleWidget());
//  switch(actualPage)
//  {
//  case 1:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(2);
//    ChooseWizardText(2);
//    break;
//  case 2:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(3);
//    ChooseWizardText(3);
//    break;
//  case 3:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(1);
//    ChooseWizardText(1);
//    break;
//  default:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(1);
//    ChooseWizardText(1);
//    break;
//  };
//
//}
//void QmitkFunctionalityComponentContainer::BackButtonPressed()
//{
//  int actualPage = m_FunctionalityComponentContainerGUI->m_WidgetStack->id(m_FunctionalityComponentContainerGUI->m_WidgetStack->visibleWidget());
//  switch(actualPage)
//  {
//  case 1:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(3);
//    ChooseWizardText(3);
//    break;
//  case 2:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(1);
//    ChooseWizardText(1);
//    break;
//  case 3:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(2);
//    ChooseWizardText(2);
//    break;
//  default:
//    m_FunctionalityComponentContainerGUI->m_WidgetStack->raiseWidget(1);
//    ChooseWizardText(1);
//    break;
//  };
//}


Ui::QmitkFunctionalityComponentContainerGUI* QmitkFunctionalityComponentContainer::GetFunctionalityComponentContainerGUI()
{
  return m_FunctionalityComponentContainerGUI;
}
