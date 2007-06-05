#ifndef QMITK_BASEFUNCTIONALITYCOMPONENT_H
#define QMITK_BASEFUNCTIONALITYCOMPONENT_H

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


/**
* \brief Base class for hierarchical components. This class contains the DataTreeIterator, access by the usual set-and get-methods, and the name-attribut of a component
* \ingroup QmitkFunctionalityComponent
* 
* 
* 

* 
* \section BaseFunctionalityComponent Overview
* 
* The BaseFunctionalityComponent ist the root-class for all component-based functionalities.
* As common ground for all inherit classes it contains the DataTreeIterator, i.e an iterator
* on the current node of the data tree, and the name-attribute of the component. 
* Inherit classes are:
* 
* - \ref QmitkAlgorithmFunctionalityComponent
* - \ref QmitkInteractionFunctionalityComponent
* - \ref QmitkFunctionalityComponentContainer
* 
*/

//#include "mitkDataTree.h"
#include <qstring.h>
#include <qobject.h>
#include <vector>

class QGroupBox;

#include "mitkDataTreeFilter.h"

//namespace mitk {
class QmitkBaseFunctionalityComponent : public QObject
{
  Q_OBJECT

public:

  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkBaseFunctionalityComponent(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual  ~QmitkBaseFunctionalityComponent();

   /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();

  /*************** TREE CHANGED (       ) ***************/
  // /** \brief The TreeChanged-slot-method updates the TreeNodeSelector if the datatree changes. */
  virtual void TreeChanged();

  /***************      OHTER METHODS     ***************/
  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

   /***************        SET AND GET     ***************/
  /*!
  \brief setter method for data tree attribute

  @param it the reference to a data tree ieterator object
  */
  virtual void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

 /** \brief Method to set the Name of the Functionality */
  void SetFunctionalityName(QString name);

  virtual QWidget* GetGUIControls();

  /*!
  \brief getter for dataTree attribute. It returns the 
  reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /*!
  \brief method for defining the name of the functionality
  */
  virtual QString GetFunctionalityName();

     /*!
  * image selected in MainSelector
  */
  mitk::Image* m_ParentMitkImage;


  /*!
  * iterator on current image
  */
  mitk::DataTreeIteratorClone m_ParentMitkImageIterator;

    /*************** GET CONTENT CONTAINER  ***************/
virtual QGroupBox * GetContentContainer();

/************ GET MAIN CHECK BOX CONTAINER ************/
virtual QGroupBox * GetMainCheckBoxContainer();

 /** \brief Method to set the "GetContentContainer"-visible or not, addicted to the visibility of a parent-component and the status of the checkable ComboBox from "GetMainCheckBoxContainer()" */
 virtual void SetContentContainerVisibility(bool);

  /***************      OHTER METHODS     ***************/
  /*!
  \brief called when a functionality becomes active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddInteractor() or AddListener()) in Activated() and the connection is removed in Deactivated()
  (e.g., GlobalStateMachine::RemoveInteractor() or RemoveListener()).
  */
  virtual void Activated();

  /*!
  \brief called when a functionality is Deactivated, i.e., is no longer active/visible. Often, event-handlers are connected (e.g., 
  GlobalStateMachine::AddInteractor() or AddListener()) in Activated() and the connection is removed in Deactivated()
  (e.g., GlobalStateMachine::RemoveInteractor() or RemoveListener()).
  */
  virtual void Deactivated();

  
  //##Documentation
  //## @brief Is the functionalityComponent currently active?
  //## 
  //## @warning Will only work if in sub-classes the superclass-methods are called in Activated() and Deactivated()!
  virtual bool IsActivated();

  //##Documentation
  //## @brief Is the functionalityComponent currently available? Some components can only be available if other components
  //## are active. That can be checked here
  //## 
  //## @warning Will only work if in sub-classes the superclass-methods are called in Activated() and Deactivated()!
  virtual bool IsAvailable();




protected:

  /***************        SET AND GET     ***************/
  /*!
  To set whether this component can be used or not. Thats why some components can only be used 	in connexion with other components.
  */
  virtual void SetAvailability(bool available);

  QObject* GetParent();

    /** \brief Vector with all added components */
  std::vector<QmitkBaseFunctionalityComponent*> m_AddedChildList;   

  
  /********************SIGN ON / OFF ************************************/
  
   
  void AddComponentListener(QmitkBaseFunctionalityComponent* component);

  void RemoveComponentListener(QmitkBaseFunctionalityComponent* component);


  /***************        ATTRIBUTES      ***************/
  
  /*!
  A Name of the FunctionalityComponent
  */
  QString m_FuncName;
  
  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIterator;

private:

  QObject * m_Parent;

  /***************        ATTRIBUTES      ***************/

  /*!
  A boolean attribute to hold if the Component is available
  */
  bool m_Available;

  /*!
  A boolean attribute to hold if the Component is currently active
  */
  bool m_Activated;

};
//}//namespace mitk
#endif

