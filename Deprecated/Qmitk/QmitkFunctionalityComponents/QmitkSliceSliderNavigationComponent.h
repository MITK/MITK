

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
#ifndef MITK_SLICESLIDERNAVIGATIONCOMPONENT_H
#define MITK_SLICESLIDERNAVIGATIONCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
class QmitkSliceSliderNavigationComponentGUI;
class QmitkStdMultiWidget;

/**
* \brief ComponentClass to find an adequate threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkSliceSliderNavigationComponent Overview
* 
* The ThresholdComponent is a class to easy find an adequate threshold. On two ways the user can insert 
* a possible threshold: as a number in a textfield or by moving a scrollbar. The selected threshold will 
* be shown on the image as an new binary layer that contains those areas above the theshold.
* Like all other componentes the QmitkSliceSliderNavigationComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QMITK_EXPORT QmitkSliceSliderNavigationComponent : public QmitkFunctionalityComponentContainer
{

  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkSliceSliderNavigationComponent(QObject *parent=0, const char *parentName=0, bool updateSelector = true, bool showSelector = false, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkSliceSliderNavigationComponent();

  /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();


  /***************      SET AND GET       ***************/

  /** \brief Method to set the Iterator to the DataTree */
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to return the TreeNodeSelector-QmitkDataTreeComboBox */
  virtual QmitkDataTreeComboBox* GetTreeNodeSelector();

      /** \brief Method to return the ComboBox that includes all GUI-elements instead of the outermost checkable CheckBox and that can be set visible or not*/
  virtual QGroupBox * GetContentContainer();

    /** \brief Method to return the outermost checkable ComboBox that is to decide whether the content shall be shown or not */
  virtual QGroupBox * GetMainCheckBoxContainer();

/** \brief Method to return the group-box that contains the tree-node-selector */
virtual QGroupBox* GetImageContent();


public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

  /** \brief Method to hide or show specific GroupBoxes*/
  void HideOrShow();

protected:

  /***************        ATTRIBUTES      ***************/

  /*!
  a reference to a data tree iterator object
  */
  mitk::DataTreeIteratorClone m_DataTreeIteratorClone;

  /*!
  a reference to the current ImageNode
  */
  mitk::DataTreeNode::Pointer m_Node;

    /*!
  a reference to the MultiWidget
  */
  QmitkStdMultiWidget * m_MultiWidget;


private:

  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is  obligatory*/
  QmitkSliceSliderNavigationComponentGUI * m_SliceSliderComponentGUI;

  /** \brief check if the selected image has a fourth-dimension an than show or hide the temporal slider*/
  void FourDimensionsOrNot();

};

#endif

