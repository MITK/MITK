/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H
#define QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H

#include "QmitkBaseFunctionalityComponent.h"

#include "mitkDataTree.h"
#include <qaction.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include "mitkDataTree.h"
#include <vector>
#include <qstring.h>
#include <qobject.h>
#include <mitkDataTreeFilter.h>
#include <qlayout.h>
#include "QmitkFunctionalityComponentContainerGUI.h"

class QmitkStdMultiWidget;


/**
* \brief ContainerClass for components 
* \ingroup QmitkFunctionalityComponent
* 
* 
* - \ref QmitkBaseFunctionalityComponent

* 
* \section QmitkFunctionalityComponentContainer Overview
* 
* The FunctionalityComponentContainer is a containerclass for several components inherit from it.
* All Components including to one FunctionalityComponentContainer can be used together as one applied 
* functionality. I.e. in particular that new functionalities can be combined with different components.
* As common ground for all inherit classes the FunctionalityComponentContainer includes a selector for
* marked data like image(s), segmentation(s) or model(s). 
*/


class QmitkFunctionalityComponentContainer : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT

public:

  /***************       CONSTRUCTOR      ***************/
  /** \brief Standard-Constructor. */
  QmitkFunctionalityComponentContainer(QObject *parent=0, const char * parentName = 0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true);
  
  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkFunctionalityComponentContainer();

  /***************        CREATE          ***************/
  /*!
  \brief Method to create all Signale-Slot-Connections 
  */
  virtual void CreateConnections();

  /*!
  \brief Method to create the GUI-Object
  */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  

  /***************        SET AND GET     ***************/

  /*!
  \brief  set it the reference to a data tree ieterator object
  */
  virtual void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);
  
  /** \brief Method to set the Name of the FunctionalityComponent */
  virtual void SetComponentName(QString name);

  /** \brief Method to get the Name of the FunctionalityComponent */
  virtual QString GetComponentName();

  /** \brief Method to get the GUI of this component.*/
  QWidget* GetGUI();

  virtual QmitkDataTreeComboBox* GetTreeNodeSelector();

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  QmitkStdMultiWidget * GetMulitWidget();

  

  /***************     ADD COMPONENTS     ***************/

  /** \brief method to add components into this component. */
  virtual void AddComponent(QmitkBaseFunctionalityComponent* componentContainer);

  /*************** TREE CHANGED (       ) ***************/
  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);


  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

protected slots:  


public slots:

  /*************** TREE CHANGED (       ) ***************/
  // /** \brief The TreeChanged-slot-method updates the TreeNodeSelector if the datatree changes. */
  virtual void TreeChanged();

  /***************      OHTER METHODS     ***************/
  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

protected:

  QmitkStdMultiWidget *GetMultiWidget();

  /** \brief Method to update the content of all DataTreeComboBoxes. */
  virtual void UpdateDataTreeComboBoxes();

  /***************        ATTRIBUTES      ***************/
  /** \brief ObserverTag */
  unsigned long m_ObserverTag;

  /** \brief Graphical User Interface for the component in general QWidget-Type */
  QWidget* m_GUI;

  /** \brief Vector with all added components */
  std::vector<QmitkBaseFunctionalityComponent*> m_AddedChildList;      

  /** \brief Attribute whether the component is active or not */
  bool m_Active;

  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;

  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;

    /** \brief Item on the actual selected Image in the TreeNodeSelector */
  const mitk::DataTreeFilter::Item * m_SelectedItem;


private:

  /***************        ATTRIBUTES      ***************/
  /** \brief also the Graphical User Interface for the component, like m_GUI, but with its specific type */
  QmitkFunctionalityComponentContainerGUI * m_FunctionalityComponentContainerGUI;

  /** \brief parent of the component */
  QObject *m_Parent;

  /** \brief name of this the component */
  QString m_ComponentName;

//  QmitkStdMultiWidget * m_MultiWidget;
//  QmitkFunctionalityComponentContainerGUI * m_GUI;

  /** \brief Spacer added at the end of the component */
  QSpacerItem* m_Spacer;

  QmitkStdMultiWidget *m_MulitWidget;
};

#endif

