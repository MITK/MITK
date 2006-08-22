

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
#ifndef MITK_THRESHOLDCOMPONENT_H
#define MITK_THRESHOLDCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
class QmitkThresholdComponentGUI;
class QmitkStdMultiWidget;

/**
* \brief ComponentClass to find an adequate threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkThresholdComponent Overview
* 
* The ThresholdComponent is a class to easy find an adequate threshold. On two ways the user can insert 
* a possible threshold: as a number in a textfield or by moving a scrollbar. The selected threshold will 
* be shown on the image as an new binary layer that contains those areas above the theshold.
* Like all other componentes the QmitkThresholdComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QmitkThresholdComponent : public QmitkFunctionalityComponentContainer
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkThresholdComponent(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  ~QmitkThresholdComponent();

  /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  QWidget* CreateContainerWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();


  /***************      SET AND GET       ***************/

  /** \brief Method to set the Iterator to the DataTree */
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to get the Iterator to the DataTree */
  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  /** \brief Method to set the Name of the Functionality */
  void SetFunctionalityName(QString name);

  /** \brief Method to get the Name of the Functionality */
  virtual QString GetFunctionalityName();

  /** \brief Method to get the Name of the FunctionalityComponent */
  virtual QString GetFunctionalityComponentName();
  
  /** \brief Method to get the GUI of this component. This Method is obligatory */
  virtual QWidget* GetGUI();

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  /***************      (DE)ACTIVATED     ***************/
   
  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

  /***************        ATTRIBUTES      ***************/

  /** \brief Vector with all added components */
  std::vector<QmitkFunctionalityComponentContainer*> m_AddedChildList; 


public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

    /** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
  void ShowThreshold(bool show = true);

  /** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
  void ShowThresholdFinderContent(bool show = true);

   /** \brief Slot method that will be called if the CheckBox at the TreeNodeSelector-Group-Box was toggled to show the TreeNodeSelector or not. */ 
  void ShowImageContent(bool show = true);

  /** \brief Slot method that will be called if the Thresholdslider was moved to update the shown image and the ThresholdLineEdit. */ 
  void ThresholdSliderChanged(int moved = 0);

  /** \brief Slot method that will be called if the ThresholdLineEdit was changed to update the shown image and the ThresholdSlider. */
  void ThresholdValueChanged( );

  /** \brief Method is called when the DataTree was changed. This Method is 	self-evident obligatory */
  virtual void TreeChanged();

  /** \brief Method is called when the DataTree was changed. This Method is 	self-evident obligatory */
  virtual void TreeChanged(mitk::DataTreeIteratorBase* it);

protected:
  /***************        ATTRIBUTES      ***************/

  /*!
  the name of the component
  */
  QString m_Name;

  /*!
  a reference to the MultiWidget
  */
  QmitkStdMultiWidget * m_MultiWidget;

  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIteratorClone;

  /*!
  a reference to the current ImageNode
  */
  mitk::DataTreeNode::Pointer m_Node;

  /*!
  a reference to the node with the thresholdImage and adjusted preferences to show the threshold
  */
  mitk::DataTreeNode::Pointer m_ThresholdImageNode;

  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;

  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;
 
  /** \brief Attribute to that will be set to true by the first activation of the functionality */
  bool m_Active;


private:

  /** \brief Method to create an Node that will hold the ThresholdImage and to set that preferences*/
  void CreateThresholdImageNode();

  /** \brief Method to set the Range of the ThresholdSlider*/
  void SetSliderRange();

  /** \brief Method to to delete ThresholdNode if Component is deactivated*/
  void DeleteThresholdNode();


  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkThresholdComponentGUI * m_GUI;

  /** \brief This Attribute holds the current ImageItem from the TreeNodeSelector if his selection was changed*/
  const mitk::DataTreeFilter::Item * m_SelectedImage; 

  /** \brief This Attribute holds the information if a thresholdnode is already existing or not*/
  bool m_ThresholdNodeExisting;

};

#endif

