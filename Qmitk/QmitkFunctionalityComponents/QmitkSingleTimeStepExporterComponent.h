

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
#ifndef MITK_SINGLETIMESTEPEXPORTERCOMPONENT_H
#define MITK_SINGLETIMESTEPEXPORTERCOMPONENT_H

#include "QmitkInteractionFunctionalityComponent.h"
#include "QmitkSingleTimeStepExporterComponentGUI.h"

#include <mitkStepper.h>
#include <mitkImageTimeSelector.h>

#include "itkObject.h"
#include "itkCommand.h"

//class QmitkSingleTimeStepExporterComponentGUI;

//class QmitkStdMultiWidget;
//class mitkStepper;
//class mitkImageTimeSelector;

/**
* \brief ComponentClass to set Seed-Points in the MPR-Widgets
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkInteractionFunctionalityComponent

* 
* \section QmitkSingleTimeStepExporterComponent Overview
* 
* The PointSetComponent is a class to set two or three Seed-Points with a combination 
* of the key "shift" and a left-mouse-click on that position in the widget where the 
* point shall be placed. The point is visible in all fout widget, that also includes 
* the 3D-view. That component has an point-list-widget that shows the points
* whith their coordinates. By clicking on of these points in the point-list-widget or
* on one of the four MPR-widgets it will be marked. A marked point can be moved with
* the mouse or removed by using the "delete"-key.
* 
*/


class QMITK_EXPORT QmitkSingleTimeStepExporterComponent : public QmitkInteractionFunctionalityComponent
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkSingleTimeStepExporterComponent(QObject *parent=0, const char *parentName=0,  bool updateSelector = true, bool showSelector = true, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkSingleTimeStepExporterComponent();

  /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();


  /***************      SET AND GET       ***************/

  /** \brief Method to return the group-box that contains the tree-node-selector */
  virtual QGroupBox* GetImageContent();

  /*************** SET DATA TREE ITERATOR ***************/
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to return the TreeNodeSelector-QmitkDataTreeComboBox */
  virtual QmitkDataTreeComboBox* GetTreeNodeSelector();

  /** \brief Method to return the Node that includes the points */
  mitk::DataTreeNode::Pointer GetPointSetNode();

  /** \brief Method to return the ComboBox that includes all GUI-elements instead of the outermost checkable CheckBox and that can be set visible or not*/
  virtual QGroupBox * GetContentContainer();

  /** \brief Method to return the outermost checkable ComboBox that is to decide whether the content shall be shown or not */
  virtual QGroupBox * GetMainCheckBoxContainer();

  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

  void Refetch();

public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method to expand or collapse the TreeNodeWidget. */
  void ShowTreeNodeSelector();

  /** \brief Slot method to expand or collapse the main component widget. */
  void ShowComponentContent();

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /** \brief Method to set the Image Selector visible or invisible */
  void SetContentContainerVisibility(bool visible);

  /** \brief Method to export a 3D-image at the selected timestep of a 4D-image dataset */
  void AddSliceImage();

protected:

  /************ Update DATATREECOMBOBOX(ES) *************/
  virtual void UpdateDataTreeComboBoxes();

  mitk::DataTreeFilter::Item* m_CurrentItem;
  bool m_4D;

private:

  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkSingleTimeStepExporterComponentGUI * m_SingleTimeStepExporterComponentGUI;

  /*!
  * Node for the connectivity-surface
  */
  mitk::DataTreeNode::Pointer m_SingleTimeStepExporterNode;

  /*!
  * PointSetPointer for the seed-points for threshold-gradient
  */
  //mitk::PointSet::Pointer m_Seeds;

  mitk::DataTreeIteratorBase* m_DataIt;
  mitk::Stepper::Pointer m_Stepper;
  mitk::ImageTimeSelector::Pointer m_ImageTimeSelector;
  // ItkEventListener::Pointer m_ItkEventListener;

};

#endif


