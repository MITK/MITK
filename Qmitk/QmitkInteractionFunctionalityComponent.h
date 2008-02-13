/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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
 * \brief Class for components that interact with the image 
 * \ingroup QmitkFunctionalityComponent
 * 
 * 
 * - \ref QmitkBaseFunctionalityComponent
 * - \ref QmitkBaseAlgorithmComponent

 * 
 * \section QmitkInteractionFunctionalityComponent Overview
 * 
 * The InteractionFunctionalityComponent is a class for components that interact with the image. It  
 * inherits the DataTreeIterator an the name from QmitkBaseFunctionalityComponent and has an own state
 * machine and an interactor (switch on/of)

 */

#ifndef QMITK_INTERACTIONFUNCTIONALITYCOMPONENT_H
#define QMITK_INTERACTIONFUNCTIONALITYCOMPONENT_H

#include "QmitkBaseFunctionalityComponent.h"
#include <qstring.h>
#include <qobject.h>

class QmitkDataTreeComboBox;
class QmitkStdMultiWidget;

class QMITK_EXPORT QmitkInteractionFunctionalityComponent : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT

  public:
         /** \brief Constructor. */
    QmitkInteractionFunctionalityComponent(QObject *parent = 0, const char * parentName = 0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true);


     /** \brief Destructor. */
    ~QmitkInteractionFunctionalityComponent();

/***************      SET AND GET       ***************/


    void SetComponentName(QString name);
      /** \brief Method to get the GUI of this component.*/
    QWidget* GetGUI();

/***************   GET COMPONENT NAME   ***************/
QString GetComponentName();

QmitkStdMultiWidget * GetMultiWidget();


  


      /*************** TREE CHANGED (       ) ***************/
virtual void TreeChanged();

/************ Update DATATREECOMBOBOX(ES) *************/
virtual void UpdateDataTreeComboBoxes();




  protected:

    /******** ******* GET TREE NODE SELECTOR ***************/
/** \brief Method to return the TreeNodeSelector-QmitkDataTreeComboBox */
virtual QmitkDataTreeComboBox* GetTreeNodeSelector();


  /** \brief Method to set the Iterator to the DataTree */
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);


  /** \brief Graphical User Interface for the component in general QWidget-Type */
  QWidget* m_GUI;

  QmitkStdMultiWidget * m_MultiWidget;

  /** \brief Attribute whether the component is active or not */
  bool m_Active;

  /** \brief Item on the actual selected Image in the TreeNodeSelector */
  const mitk::DataTreeFilter::Item * m_SelectedItem;

  mitk::DataTreeIteratorClone m_DataTreeIterator;

  private:
  /*!
  Name of the Component
  */
  QString m_Name;



};

#endif

