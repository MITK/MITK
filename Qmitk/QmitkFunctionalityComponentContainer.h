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

class QmitkFunctionalityComponentContainerGUI;
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
  QmitkFunctionalityComponentContainer(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);
  
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor to set some preferences. */
  QmitkFunctionalityComponentContainer(QObject *parent, const char *name, mitk::DataTreeIteratorBase* dataIt, bool updateSelector, bool showSelector);
  

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  ~QmitkFunctionalityComponentContainer();

  /***************        CREATE          ***************/
  //virtual QAction * CreateAction(QActionGroup* m_FunctionalityComponentActionGroup);
  virtual void CreateConnections();
  virtual QWidget* CreateContainerWidget(QWidget* parent);

  

  /***************        SET AND GET     ***************/
  virtual QString GetFunctionalityName();
  void SetFunctionalityName(QString name);

  virtual QString GetFunctionalityComponentName();
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

  QWidget* GetGUI();

  /*************** TREE CHANGED (       ) ***************/
  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);

  /***************      OHTER METHODS     ***************/
  void AddComponent(QmitkFunctionalityComponentContainer* componentContainer);
  virtual void Activated();


  /***************        ATTRIBUTES      ***************/
  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList;      

protected slots:  
  /*************** TREE CHANGED (       ) ***************/
  // /** \brief The TreeChanged-slot-method updates the TreeNodeSelector if the datatree changes. */
  virtual void TreeChanged();
  virtual void TreeChanged(mitk::DataTreeIteratorBase* it);

public slots:
  /***************      OHTER METHODS     ***************/
  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

protected:
  /***************        ATTRIBUTES      ***************/

  unsigned long m_ObserverTag;
      
  bool m_Available;

  bool m_Activated;
  bool m_Active;
  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;
  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;


private:
  /***************        ATTRIBUTES      ***************/
  QObject *m_Parent;
  QString m_Name;
  QmitkStdMultiWidget * m_MultiWidget;
  QmitkFunctionalityComponentContainerGUI * m_GUI;
  const mitk::DataTreeFilter::Item * m_SelectedImage;


};

#endif

