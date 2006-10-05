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
  ~QmitkFunctionalityComponentContainer();

  /***************        CREATE          ***************/
  //virtual QAction * CreateAction(QActionGroup* m_FunctionalityComponentActionGroup);
  virtual void CreateConnections();
  virtual QWidget* CreateContainerWidget(QWidget* parent);

  

  /***************        SET AND GET     ***************/
  virtual QString GetFunctionalityName();

  void SetFunctionalityName(QString parentName);

  virtual QString GetComponentName();
    /*!
  \brief getter for dataTree attribute. It returns the 
  reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /*!
  \brief setter method for data tree attribute
  @param it the reference to a data tree ieterator object
  */
  virtual void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to get the GUI of this component. This Method is obligatory */
  QWidget* GetGUI();

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);
  
  /***************     ADD COMPONENTS     ***************/
  /** \brief method to add components into this component. */
  virtual void AddComponent(QmitkFunctionalityComponentContainer* componentContainer);


  /*************** TREE CHANGED (       ) ***************/
  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);

  /***************      OHTER METHODS     ***************/

  virtual void Activated();
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
  /***************        ATTRIBUTES      ***************/

  unsigned long m_ObserverTag;

  QWidget* m_GUI;

    /***************        ATTRIBUTES      ***************/
  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList;      

  /** \brief Attribute whether the component is active or not */
  bool m_Active;

  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;
  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;


private:
  /***************        ATTRIBUTES      ***************/
  QmitkFunctionalityComponentContainerGUI * m_FunctionalityComponentContainerGUI;
  QObject *m_Parent;
  QString m_ParentName;
  QString m_ComponentName;
  QmitkStdMultiWidget * m_MultiWidget;
//  QmitkFunctionalityComponentContainerGUI * m_GUI;
  const mitk::DataTreeFilter::Item * m_SelectedImage;

  /** \brief Spacer added at the end of the component */
  QSpacerItem* m_Spacer;


};

#endif

