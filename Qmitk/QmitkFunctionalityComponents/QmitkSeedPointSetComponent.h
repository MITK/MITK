

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
#ifndef MITK_SEEDPOINTSETCOMPONENT_H
#define MITK_SEEDPOINTSETCOMPONENT_H

#include "QmitkInteractionFunctionalityComponent.h"

#include "mitkPointSetInteractor.h"
#include "mitkPointSet.h"


class QmitkSeedPointSetComponentGUI;
class QmitkStdMultiWidget;

/**
* \brief ComponentClass to set Seed-Points in the MPR-Widgets
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkInteractionFunctionalityComponent

* 
* \section QmitkSeedPointSetComponent Overview
* 
* The PointSetComponent is a class to set two or three Seed-Points with a combination 
* of the key "shift" and a left-mouse-click on that position in the widget where the 
* point shall be placed. The point is visible in all four widgets, that also includes 
* the 3D-view. That component has an point-list-widget that shows the points
* whith their coordinates. By clicking on oine of these points in the point-list-widget or
* on one of the four MPR-widgets it will be marked. A marked point can be moved with
* the mouse or removed by using the "delete"-key.
* 
*/


class QMITK_EXPORT QmitkSeedPointSetComponent : public QmitkInteractionFunctionalityComponent
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkSeedPointSetComponent(QObject *parent=0, const char *parentName=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, bool updateSelector = true, bool showSelector = true);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkSeedPointSetComponent();

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

  /** \brief Method to that the name of the PointSetNode in the dataTree */
  void SetDataTreeName(std::string pointSetNodeName);

  QmitkSeedPointSetComponentGUI* GetSeedPointSetComponentGUI();

  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

public slots:  
  /***************      OHTER METHODS     ***************/

  void ShowTreeNodeSelector();
  void ShowComponentContent();

  ///** \brief Slot method to export the PointSet */
  //void PointsExported();

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

    /** \brief Method to set the Image Selector visible or invisible */
  void SetContentContainerVisibility(bool visible);

protected:

  /************ Update DATATREECOMBOBOX(ES) *************/
  virtual void UpdateDataTreeComboBoxes();

private:
 void CreatePointSetNode();
 void IsNodeExisting();
  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is  obligatory*/
  QmitkSeedPointSetComponentGUI * m_PointSetComponentGUI;
  bool m_PointSetNodeExisting;


  /*!
  * Node for the seed-points 
  */
  mitk::DataTreeNode::Pointer m_PointSetNode;

  mitk::DataTreeIteratorBase* m_It;

  /*!
  * Interactor for the seed-points
  */
  mitk::PointSetInteractor::Pointer m_PointSetInteractor;

  /*!
  * PointSetPointer for the seed-points 
  */
  mitk::PointSet::Pointer m_Seeds;

  /*!
  * Name of the PointSetNode that can be set by - \ref SetDataTreeName(QString * pointSetNodeName) 
  */
  std::string  m_PointSetNodeName;



};

#endif

