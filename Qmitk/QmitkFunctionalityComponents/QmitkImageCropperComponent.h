

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
#ifndef MITK_IMAGECROPPERCOMPONENT_H
#define MITK_IMAGECROPPERCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
class QmitkImageCropperComponentGUI;
class QmitkStdMultiWidget;


#include <mitkBoundingObject.h>
#include <mitkOperationActor.h>
#include <mitkOperation.h>
#include <mitkAffineInteractor.h>

/**
* \brief ComponentClass to find an adequate threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkImageCropperComponent Overview
* 
* The ImageCropperComponent is a class to easy find an adequate threshold. On two ways the user can insert 
* a possible threshold: as a number in a textfield or by moving a scrollbar. The selected threshold will 
* be shown on the image as an new binary layer that contains those areas above the theshold.
* Like all other componentes the QmitkImageCropperComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QMITK_EXPORT QmitkImageCropperComponent : public QmitkFunctionalityComponentContainer, public mitk::OperationActor
{
	Q_OBJECT
    // Operation base class, which holds pointers to a node of the data tree (mitk::DataTreeNode) 
    // and to two data sets (mitk::BaseData) instances
  class QMITK_EXPORT opExchangeNodes: public mitk::Operation
  {
    public: opExchangeNodes( mitk::OperationType type,  mitk::DataTreeNode* node, 
                                                        mitk::BaseData* oldData, 
                                                        mitk::BaseData* newData );
            mitk::DataTreeNode* GetNode() { return m_Node; }
            mitk::BaseData* GetOldData() { return m_OldData; }
            mitk::BaseData* GetNewData() { return m_NewData; }
    private: mitk::DataTreeNode::Pointer m_Node;
             mitk::BaseData::Pointer m_OldData;
             mitk::BaseData::Pointer m_NewData;
  };
public:
  /***************       CONSTRUCTOR      ***************/
  /** \brief Constructor. */
  QmitkImageCropperComponent(QObject *parent=0, const char *parentName=0, bool updateSelector = true, bool showSelector = true, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL, QmitkFunctionalityComponentContainer * parentObject = NULL);

  /***************        DESTRUCTOR      ***************/
  /** \brief Destructor. */
  virtual ~QmitkImageCropperComponent();

  /***************        CREATE          ***************/

  /** \brief Method to create the GUI for the component from the .ui-File. This Method is obligatory */
  virtual QWidget* CreateControlWidget(QWidget* parent);

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();


  /***************      SET AND GET       ***************/

  /** \brief Method to set the Iterator to the DataTree */
  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  /** \brief Method to return the TreeNodeSelector-QmitkDataTreeComboBox */
  virtual QmitkDataTreeComboBox* GetTreeNodeSelector();

  
    /** \brief Method to return the ComboBox that includes all GUI-elements instead of the outermost checkable CheckBox and that can be set visible or not*/
  virtual QGroupBox * GetContentContainer();

    /** \brief Method to return the outermost checkable ComboBox that is to decide whether the content shall be shown or not */
  virtual QGroupBox * GetMainCheckBoxContainer();

/** \brief Method to return the group-box that contains the tree-node-selector */
virtual QGroupBox* GetImageContent();

  /***************      (DE)ACTIVATED     ***************/

  ///** \brief Method to set m_Activated to true */
  virtual void Activated();

  ///** \brief Method to set m_Activated to false */
  virtual void Deactivated();

signals:
void mitkImageChanged();

public slots:  
  /***************      OHTER METHODS     ***************/

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);


  /** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
  void ShowImageCropperContent(bool show = true);

  /** \brief Slot method that will be called if the CheckBox at the TreeNodeSelector-Group-Box was toggled to show the TreeNodeSelector or not. */ 
  void ShowImageContent(bool show = true);

  /** \brief Slot method that will be called if the cube-Button was pressed */ 
  void CubePressed();

  /** \brief Slot method that will be called if the Ellipsoid-Button was pressed */ 
  void EllipsoidPressed();

  /** \brief Slot method that will be called if the Cone-Button was pressed */ 
  void ConePressed();

  /** \brief Slot method that will be called if the Cylinder-Button was pressed */ 
  void CylinderPressed();

  /** \brief Slot method that will be called if the CropImage-Button was pressed to crop the image*/ 
  void CropImage();


protected:


	  /*!
  * image which is used to create the surface
  */
  mitk::Image* m_MitkImage;


  /*!
  * iterator on current image
  */
  mitk::DataTreeIteratorClone m_MitkImageIterator;

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
  a reference to the node with the ImageCropperImage and adjusted preferences to show the new Image
  */
  mitk::DataTreeNode::Pointer m_ImageCropperImageNode;

  
  /*!
  * \brief An iterator to the image to be croped.
  */
  mitk::DataTreeIteratorClone m_IteratorToImageToCrop;

  /*!
  * \brief A pointer to the image to be cropped.
  */
  mitk::Image::Pointer m_ImageToCrop;

  /*!
  * \brief The cuboid used for cropping.
  */
  mitk::BoundingObject::Pointer m_CroppingObject;

  /*!
  * \brief Tree node of the cuboid used for cropping.
  */
  mitk::DataTreeNode::Pointer m_CroppingObjectNode;
  
  /*!
  * \brief Interactor for moving and scaling the cuboid
  */
  mitk::AffineInteractor::Pointer m_AffineInteractor;
 
  /*!
   * \brief Creates the cuboid and its data tree node.
   */
  virtual void CreateBoundingObject(int type);

  /*!
   * \brief Finds the given node in the data tree and fits the cuboid to it
   */
  virtual void AddBoundingObjectToNode(mitk::DataTreeIteratorClone& iterToNode);
  
  /*!
   * \brief Removes the cuboid from any node and hides it from the user.
   */
  virtual void RemoveBoundingObjectFromNode();

    /*
  \brief Interface of a mitk::StateMachine (for undo/redo)
  */
  virtual void  ExecuteOperation (mitk::Operation*);

    /*!
  a reference to the MultiWidget
  */
  QmitkStdMultiWidget * m_MultiWidget;

  bool m_BoundingObjectExists;

private:

  QmitkFunctionalityComponentContainer* m_ParentObject;
  /***************        ATTRIBUTES      ***************/

  /** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
  QmitkImageCropperComponentGUI * m_ImageCropperComponentGUI;

  /** \brief This Attribute holds the information if a ImageCropperNode is already existing or not*/
  bool m_ImageCropperNodeExisting;

    // operation constant
  static const mitk::OperationType OP_EXCHANGE;


};

#endif

