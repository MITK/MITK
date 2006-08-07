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

class QmitkFunctionalityComponentContainerGUI;

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

  /** \brief Constructor. */
  QmitkFunctionalityComponentContainer(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = NULL);
  /** \brief Destructor. */
  ~QmitkFunctionalityComponentContainer();

  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_Qbfc;

  // Create
  //virtual QAction * CreateAction(QActionGroup* m_FunctionalityComponentActionGroup);
  virtual void CreateConnections();
  virtual QWidget* CreateContainerWidget(QWidget* parent);

  virtual void Activated();

  // SET and GET
  virtual QString GetFunctionalityName();
  void SetFunctionalityName(QString name);
  virtual QString GetFunctionalityComponentName();
    /*!
  \brief setter method for data tree attribute
  @param it the reference to a data tree ieterator object
  */
  virtual void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /*!
  \brief getter for dataTree attribute. It returns the 
  reference to a data tree iterator object
  */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  void QmitkFunctionalityComponentContainer::AddComponent(QmitkFunctionalityComponentContainer* componentContainer);

 // virtual void TreeChanged();
 // virtual void TreeChanged(mitk::DataTreeIteratorBase* it);

  QWidget* GetGUI();

    virtual void TreeChanged();

protected slots:  

  // /** \brief The TreeChanged-slot-method updates the TreeNodeSelector if the datatree changes. */
  //virtual void TreeChanged();
  virtual void TreeChanged(mitk::DataTreeIteratorBase* it);

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  void ImageSelected(mitk::DataTreeIteratorClone imageIt);

protected:
  QString m_Name;
  bool m_Activated;
    unsigned long m_ObserverTag;


private:
  QmitkFunctionalityComponentContainerGUI * m_GUI;


};

#endif

