

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-10 15:14:14 +0200 (Di, 10 Jul 2007) $
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef MITK_SURFACETRANSFORMERCOMPONENT_H
#define MITK_SURFACETRANSFORMERCOMPONENT_H

#include "QmitkFunctionalityComponentContainer.h"
class QmitkSurfaceTransformerComponentGUI;
class QmitkStdMultiWidget;

//class vtkTransform;
//class qaccel;
#include <vtkTransform.h>

#include <qaccel.h>

/**
* \brief ComponentClass to find an adequate threshold for a selected image
* \ingroup QmitkFunctionalityComponent
* 
* This class inherits from
* - \ref QmitkFunctionalityComponentContainer

* 
* \section QmitkSurfaceTransformerComponent Overview
* 
* The SurfaceTransformerComponent is a class to translate, rotate and scale parts of surfaces
* Like all other componentes the QmitkSurfaceTransformerComponent inherits from QmitkFunctionalityComponentContainer.
* 
*/


class QmitkSurfaceTransformerComponent : public QmitkFunctionalityComponentContainer
{

	Q_OBJECT

public:
	/***************       CONSTRUCTOR      ***************/
	/** \brief Constructor. */
	QmitkSurfaceTransformerComponent(QObject *parent=0, const char *parentName=0, bool updateSelector = true, bool showSelector = true, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

	/***************        DESTRUCTOR      ***************/
	/** \brief Destructor. */
	virtual ~QmitkSurfaceTransformerComponent();

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

	/** \brief Method to return node that shall be transfomed */
	mitk::DataTreeNode::Pointer GetSurfaceNode();

	/** \brief Method to set the reference to the node that shall be transformed */
	void SetSurfaceNode(mitk::DataTreeNode::Pointer node);

	/** \brief Method to set the the distance that is used to move the object */
	void SetDistance(double distance);

    /** \brief Method to get the distance that is used to move the object */
	double GetDistance();

	/** \brief Method to set the the angle that is used to rotate the object */
	void SetAngle(double angle);

	/** \brief Method to get the angle that is used to rotate the object */
	double GetAngle();

	/***************      (DE)ACTIVATED     ***************/

	///** \brief Method to set m_Activated to true */
	virtual void Activated();

	///** \brief Method to set m_Activated to false */
	virtual void Deactivated();

public slots:  
	/***************      OHTER METHODS     ***************/

	/** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
	virtual void ImageSelected(const mitk::DataTreeFilter::Item * imageIt);

	/** \brief Slot method that will be called if the CheckBox at the Threshold-Group-Box was toggled to show the threshold image or not. */ 
	void ShowSurfaceTransformerContent(bool show = true);

	/** \brief Slot method that will be called if the CheckBox at the TreeNodeSelector-Group-Box was toggled to show the TreeNodeSelector or not. */ 
	void ShowImageContent(bool show = true);

	/** \brief Slot method that will be called if the move-operation is activated or deactivated by the move button. */ 
	void TransformationModeMove(bool toggleflag);

	/** \brief Slot method that will be called if the rotate-operation is activated or deactivated by the rotate button. */ 
	void TransformationModeRotate(bool toggleflag);

	/** \brief Slot method that will be called if the scale-operation is activated or deactivated by the rotate button. */ 
	void TransformationModeScale(bool toggleflag);

  /** \brief Slot method that will be called if the mirror-operation is activated or deactivated by the mirror button. */ 
	void TransformationModeMirror(bool toggleflag);

  void TransformXBox();
  void TransformYBox();
  void TransformZBox();


	/***************************** SLOTS TO TRANSFORM *************************************/

	/* TRANSLATION */

	/** \brief Slot method to move the surface to the left if the move button is activated and (Qt::Key_Left) ist pressed*/
	void MoveLeft();                     
	/** \brief Slot method to move the surface to the right if the move button is activated and (Qt::Key_Right) ist pressed*/
	void MoveRight();                     
	/** \brief Slot method to move the surface up if the move button is activated and (Qt::Key_Up) ist pressed*/
	void MoveUp();                     
	/** \brief Slot method to move the surface down if the move button is activated and (Qt::Key_Down) ist pressed*/
	void MoveDown(); 
	/** \brief Slot method to move the surface forward if the move button is activated and (Qt::Key_A) ist pressed*/
	void MoveForward();
	/** \brief Slot method to move the surface backward if the move button is activated and (Qt::Key_S) ist pressed*/
	void MoveBackward();

	/* ROTATION */  

	/** \brief Slot method to rotate the surface around the x-Axis if the rotate button is activated and (Qt::Key_Left) is pressed*/
	void RotX();  
	/** \brief Slot method to rotate the surface negative around the x-Axis if the rotate button is activated and (Qt::Key_Right) is pressed*/
	void RotInvX();
	/** \brief Slot method to rotate the surface around the y-Axis if the rotate button is activated and (Qt::Key_Up) is pressed*/
	void RotY(); 
	/** \brief Slot method to rotate the surface negative around the y-Axis if the rotate button is activated and (Qt::Key_Down) is pressed*/
	void RotInvY(); 
	/** \brief Slot method to rotate the surface around the z-Axis if the rotate button is activated and (Qt::Key_A) is pressed*/
	void RotZ();
	/** \brief Slot method to rotate the surface negative around the z-Axis if the rotate button is activated and (Qt::Key_S) is pressed*/
	void RotInvZ();   

	/* Scale */ 

	/** \brief Slot method to scale the surface up if the scale button is activated and (Qt::Key_Up) is pressed*/
	void ScaleUp();
	/** \brief Slot method to scale the surface down if the scale button is activated and (Qt::Key_Down) is pressed*/
	void ScaleDown();  

  /* Mirror */ 

  /** \brief Slot method to mirror the surface around the X-plane if the mirror-button is activated and (Qt::Key_X) is pressed*/
  void MirrorX();
  /** \brief Slot method to mirror the surface around the Y-plane if the mirror-button is activated and (Qt::Key_Y) is pressed*/
  void MirrorY(); 
  /** \brief Slot method to mirror the surface around the Z-plane if the mirror-button is activated and (Qt::Key_Z) is pressed*/
  void MirrorZ(); 

protected:


	///** \brief Method that controls everything what happend after an image was selected */
	//void DataObjectSelected();

	///** \brief Method to repaint all Renderwindows after transformation */
	void RepaintRenderWindows();

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
	a reference to the node with the thresholdImage and adjusted preferences to show the threshold
	*/
	mitk::DataTreeNode::Pointer m_SurfaceTransformerImageNode;

  double m_XTranslate;
  double m_YTranslate;
  double m_ZTranslate;

private:

	///** \brief Method to create an Node that will hold the ThresholdImage and to set that preferences*/
	//void CreateSurfaceTransformerImageNode();

	/** \brief Method to to delete SurfaceTransformerNode if Component is deactivated*/
	void DeleteSurfaceTransformerNode();

	/** \brief Method to adopt the transformation on the surface and put it into the datatree*/
	void Transform(vtkTransform* transform);

	/***************        ATTRIBUTES      ***************/

	/* MEMBER ACCELS FOR TRANSLATION */
	/** \brief Member for the connection between the left-arrow-key and the moveLeft slot*/
	QAccel* m_Left;
	/** \brief  Member for the connection between the right-arrow-key and the moveRight slot*/
	QAccel* m_Right;
	/** \brief  Member for the connection between the up-arrow-key and the moveUp slot*/
	QAccel* m_Up;
	/** \brief  Member for the connection between the down-arrow-key and the moveDown slot*/
	QAccel* m_Down;
	/** \brief  Member for the connection between the "A"-key and the moveForward slot*/
	QAccel* m_Forward;
	/** \brief  Member for the connection between the "S"-key and the moveBackward slot*/
	QAccel* m_Backward;

    /* MEMBER ACCELS FOR ROTATION */
	/** \brief Member for the connection between the left-arrow-key and the RotX slot*/
	QAccel* m_RotX;
	/** \brief  Member for the connection between the right-arrow-key and the RotInvX slot*/
	QAccel* m_RotInvX;
	/** \brief  Member for the connection between the up-arrow-key and the RotY slot*/
	QAccel* m_RotY;
	/** \brief  Member for the connection between the down-arrow-key and the RotInvY slot*/
	QAccel* m_RotInvY;
	/** \brief  Member for the connection between the "A"-key and the RotZ slot*/
	QAccel* m_RotZ;
	/** \brief  Member for the connection between the "S"-key and the RotInvZ slot*/
	QAccel* m_RotInvZ;

	/* MEMBER ACCELS FOR SCALE */
	/** \brief  Member for the connection between the up-arrow-key and the ScaleUp slot*/
	QAccel* m_ScaleUp;
	/** \brief  Member for the connection between the down-arrow-keyand the ScaleDown slot*/
	QAccel* m_ScaleDown;

  /* MEMBER ACCELS FOR SCALE */
	/** \brief  Member for the connection between the X-key and the MirrorX slot*/
	QAccel* m_MirrorX;
	/** \brief  Member for the connection between the Y-key and the MirrorY slot*/
	QAccel* m_MirrorY;
  /** \brief  Member for the connection between the Z-key and the MirrorZ slot*/
	QAccel* m_MirrorZ;



	/** \brief The created GUI from the .ui-File. This Attribute is	obligatory*/
	QmitkSurfaceTransformerComponentGUI * m_SurfaceTransformerComponentGUI;

	/** \brief This Attribute holds the information if a SurfaceTransformerNode is already existing or not*/
	bool m_SurfaceTransformerNodeExisting;

	/*!
	a reference to the node with the surface that shall be transformed
	*/
	mitk::DataTreeNode::Pointer m_SurfaceNode;

	/*!
	Member for the transformationmode that is active. 
	0 = none
	1 = move
	2 = rotate
	*/
	int m_TransformationMode;

	/*!
	a reference to the visible multiwidget
	*/
	QmitkStdMultiWidget* m_MultiWidget;

	/*!
	Member for the standard angle that is used to rotate the object
	*/
	double m_Angle;

    /*!
	Member for the standard distance that is used to move the object
	*/
    double m_Distance;

	/*!
	Member for the standard scale-factor that is used to scale the object
	*/
    double m_Scale;

	/*!
	Member for the scale-factors x, y, z that are used to scale the object
	*/
	double m_ScaleX;

	/*!
	Member for the scale-factors x, y, z that are used to scale the object
	*/
	double m_ScaleY;

	/*!
	Member for the scale-factors x, y, z that are used to scale the object
	*/
	double m_ScaleZ;


};

#endif

