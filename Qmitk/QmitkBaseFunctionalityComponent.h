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

#include "mitkDataTree.h"
#include <qstring.h>



//namespace mitk {
class QmitkBaseFunctionalityComponent 
{
  bool m_Available;
  bool m_Activated;
public:


  /** \brief Constructor. */
  QmitkBaseFunctionalityComponent(mitk::DataTreeIteratorBase* dataIt = NULL);

  /** \brief Destructor. */
  ~QmitkBaseFunctionalityComponent();

  /*!
  \brief setter method for data tree attribute
  @param it the reference to a data tree ieterator object
  */
  void SetDataTree(mitk::DataTreeIteratorBase* it);

  /*!
  \brief getter for dataTree attribute. It returns the 
  reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /*!
  \brief  Slot that will be called if DataTree changes to inform
  addicted methods
  */
  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);

  /*!
  \brief  Slot that will be called if DataTree changes to inform
  addicted methods
  */
  virtual void TreeChanged();

  /*!
  \brief method for defining the name of the functionality
  */
  virtual QString GetFunctionalityName();

  void SetFunctionalityName(QString name);

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

  virtual bool IsAvailable();

protected:

  virtual void SetAvailability(bool available);

  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIterator;

  /*!
  a Name of the FunctionalityComponent
  */
  QString m_Name;

  unsigned long m_ObserverTag;

  bool m_TreeChangedWhileInActive;

private:



};
//}//namespace mitk
#endif

