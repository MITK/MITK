

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
#ifndef MITK_CONNECTIVITYFILTERCOMPONENT_H
#define MITK_CONNECTIVITYFILTERCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"

#include "mitkPointSetInteractor.h"
#include "mitkPointSet.h"


class QmitkConnectivityFilterComponentGUI;
class QmitkSeedPointSetComponent;
class QmitkStdMultiWidget;

/**
* \brief ComponentClass to set Seed-Points in the MPR-Widgets
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkInteractionFunctionalityComponent

* 
* \section QmitkConnectivityFilterComponent Overview
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


class QmitkConnectivityFilterComponent : public QmitkFunctionalityComponentContainer
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkConnectivityFilterComponent(QObject *parent=0, const char *parentName=0, bool updateSelector = true, bool showSelector = true, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkConnectivityFilterComponent();

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

  void SetPointSet(QmitkSeedPointSetComponent* pointSet);

  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

public slots:  

  void StartConnectivityFilter();
  /***************      OHTER METHODS     ***************/

  void ShowTreeNodeSelector();
  void ShowComponentContent();

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /** \brief Method to set the Image Selector visible or invisible */
  void SetContentContainerVisibility(bool visible);

   signals:

    // Emitted when the property changes. The parameter gives you the new value of the property.
    void nodeExported(mitk::DataTreeNode::Pointer);


protected:

  /************ Update DATATREECOMBOBOX(ES) *************/
  virtual void UpdateDataTreeComboBoxes();
  //  /*!
  //* image which is used to create the surface
  //*/
  //mitk::Image* m_MitkImage;

  ///*!
  //* iterator on current image
  //*/
  //mitk::DataTreeIteratorClone m_MitkImageIterator;




private:

  /** \brief Method to create the pointset-object that is necessary for some connectivity-options. See also  - \ref QmitkSeedPointSetComponent*/
  //void CreatePointSet();

  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkConnectivityFilterComponentGUI * m_ConnectivityFilterComponentGUI;


  /*!
  * Node for the connectivity-surface
  */
  mitk::DataTreeNode::Pointer m_ConnectivityNode;

  /*!
  * Node for the seed-points 
  */
  //mitk::DataTreeNode::Pointer m_PointSetNode;


  /*!
  * Interactor for the seed-points for threshold-gradient
  */
  mitk::PointSetInteractor::Pointer m_PointSetInteractor;

  /*!
  * PointSetPointer for the seed-points for threshold-gradient
  */
  //mitk::PointSet::Pointer m_Seeds;

   /** \brief Member iterator on the Dataset*/
  mitk::DataTreeIteratorBase* m_DataIt;

   /** \brief Member of a point Set that can be used to set Points in the visualisation widgets*/
  QmitkSeedPointSetComponent* m_PointSet;
  
   /** \brief Member how many connectivities were created yet*/
  int m_ConnectivityCounter;

  /*!
  a reference to the visible multiwidget
  */
  QmitkStdMultiWidget* m_MultiWidget;




};

#endif

