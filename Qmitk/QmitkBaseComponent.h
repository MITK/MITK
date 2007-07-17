#ifndef QMITK_BASECOMPONENT_H
#define QMITK_BASECOMPONENT_H

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-06-06 10:05:10 +0200 (Mi, 06 Jun 2007) $
Version:   $Revision: 10646 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


/**
* \brief Base class for components of different kinds.
* \ingroup QmitkFunctionalityComponent
*
*
*

*
* \section BaseComponent Overview
*
* The BaseComponent ist the root-class for all components.
* It contains all necessary items a component uses.
* Inherit classes are:
*
* - \ref QmitkBaseComponent
* - \ref QmitkBaseAlgorithmComponent
* - \ref QmitkBaseTrackingComponent
*
*/

#include <qstring.h>
#include <qobject.h>
#include <vector>

class QGroupBox;

class QmitkBaseComponent : public QObject
{
  Q_OBJECT

public:

  /***************       CONSTRUCTOR      ***************/
  /** 
  * @brief Constructor
  */
  QmitkBaseComponent(QObject *parent=0, const char *name=0);

  /***************        DESTRUCTOR      ***************/
  /** 
  * @brief Destructor 
  */
  virtual  ~QmitkBaseComponent();

  /** 
  * @brief Method to create the GUI for the component from the .ui-File. This Method is obligatory 
  */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  /** 
  * @brief Method to create the connections for the component. This Method is obligatory even if no connections is needed
  */
  virtual void CreateConnections();

  /** 
  * @brief The TreeChanged-slot-method if the component needs this information then reimplement
  */
  virtual void TreeChanged();

  /** 
  * @brief Method to set the Name of the Functionality 
  */
  void SetFunctionalityName(QString name);

  virtual QWidget* GetGUIControls();

  /*!
  * @brief method for defining the name of the functionality
  */
  virtual QString GetFunctionalityName();


  /*************** GET CONTENT CONTAINER  ***************/
  virtual QGroupBox * GetContentContainer();

  /************ GET MAIN CHECK BOX CONTAINER ************/
  virtual QGroupBox * GetMainCheckBoxContainer();

  /** 
  * @brief Method to set the "GetContentContainer"-visible or not, addicted to the visibility of a parent-component and the status of the checkable ComboBox from "GetMainCheckBoxContainer()" 
  */
  virtual void SetContentContainerVisibility(bool);

  /***************      OHTER METHODS     ***************/
  /*!
  @brief called when a functionality becomes active/visible. Often, event-handlers are connected (e.g.,
  GlobalStateMachine::AddInteractor() or AddListener()) in Activated() and the connection is removed in Deactivated()
  (e.g., GlobalStateMachine::RemoveInteractor() or RemoveListener()).
  */
  virtual void Activated();

  /*!
  @brief called when a functionality is Deactivated, i.e., is no longer active/visible. Often, event-handlers are connected (e.g.,
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
  * @biref To set whether this component can be used or not. Thats why some components can only be used  in connexion with other components.
  */
  virtual void SetAvailability(bool available);

  QObject* GetParent();

  /** 
  * @brief Vector with all added components 
  */
  std::vector<QmitkBaseComponent*> m_AddedChildList;


  /********************SIGN ON / OFF ************************************/

  void AddComponentListener(QmitkBaseComponent* component);

  void RemoveComponentListener(QmitkBaseComponent* component);


  /*!
  * @brief A Name of the FunctionalityComponent
  */
  QString m_FuncName;

  /*!
  * @brief The parent of the FunctionalityComponent
  */
  QObject * m_Parent;


private:
  /*!
  * @brief A boolean attribute to hold if the Component is available
  */
  bool m_Available;

  /*!
  * @brief A boolean attribute to hold if the Component is currently active
  */
  bool m_Activated;

};

#endif

