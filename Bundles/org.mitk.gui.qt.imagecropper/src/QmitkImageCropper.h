/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13136 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QMITK_IMAGECROPPER_H__INCLUDED)
#define QMITK_IMAGECROPPER_H__INCLUDED

#pragma warning( disable : 4250 )

#include "QmitkFunctionality.h"
#include <mitkBoundingObject.h>
#include <mitkOperationActor.h>
#include <mitkOperation.h>
#include <mitkAffineInteractor.h>
#include <itkImage.h>
#include <qprogressdialog.h>

//!class QmitkStdMultiWidget;
//!class QmitkImageCropperControls;
#include "ui_QmitkImageCropperControls.h"
#include "QmitkImageCropperDll.h"

/*!
\brief Functionality for cropping images with a cuboid 

This functionality lets the user select an image from the data tree, select an area of interest by placing 
a cuboid object, and then crop the image, so that pixels from outside the cuboid will remove.

The image size is automatically reduced, if the cuboid is not rotated but parallel to the image axes.

\b Implementation

The functionality owns a cuboid (m_CroppingObject) and the associated interactor (m_AffineInteractor), 
which implements moving and scaling the cuboid.

\ingroup Functionalities
*/

//!class QmitkImageCropper : public QmitkFunctionality, public mitk::OperationActor
class MITK_QT_IMAGECROPPER QmitkImageCropper : public QObject, public QmitkFunctionality, public mitk::OperationActor
{  

  /// Operation base class, which holds pointers to a node of the data tree (mitk::DataTreeNode) 
  /// and to two data sets (mitk::BaseData) instances
  class opExchangeNodes: public mitk::Operation
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

private:
  
  Q_OBJECT
  
public:  
  /*!  
  \brief Constructor. Called by SampleApp (or other apps that use functionalities)  
  */  
  //!QmitkImageCropper(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);
  QmitkImageCropper(QObject *parent=0);

  /*!  
  \brief Destructor  
  */  
  virtual ~QmitkImageCropper();

  /*!  
  \brief Creates the Qt widget containing the functionality controls, like sliders, buttons etc.  
  */  
  //!virtual QWidget * CreateControlWidget(QWidget *parent);
  virtual void CreateQtPartControl(QWidget* parent);

  /*!  
  \brief Should create the applications main widget. Not used here (maybe not clever).
  */  
  //!virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief Creates the Qt connections needed
  */  
  virtual void CreateConnections();

  /*!  
  \brief Method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  //!virtual QAction * CreateAction(QActionGroup *parent);

  /*!  
  \brief Invoked when this functionality is selected by the application 
  */  
  virtual void Activated();

  /*!  
  \brief Invoked when the user leaves this functionality 
  */  
  virtual void Deactivated();

  /*!  
  \brief Invoked when the data tree changes
  */  
  //!virtual void TreeChanged();
  virtual void DataStorageChanged();

  /*!  
  \brief Filter function, used with the QmitkTreeNodeSelector widget to ensure, only images can be selected
  */  
  static bool DataTreeNodeFilter(mitk::DataTreeNode*);

  /*
  \brief Interface of a mitk::StateMachine (for undo/redo)
  */
  virtual void  ExecuteOperation (mitk::Operation*);

   QWidget* GetControls();

   const mitk::DataTreeNode::Pointer selectedImage();

public slots:

  virtual void CropImage();
  virtual void SurroundingCheck(bool value);

  // from QmitkImageCropperControls
  virtual void ImageSelectionChanged();
  virtual void ImageNodeChanged( const mitk::DataTreeNode::Pointer item );
  virtual void ChkInformationToggled( bool on );

protected:
  //mitk::DataTree::Pointer m_DataTree;
  //mitk::DataTreeIteratorClone 	m_DataTreeIterator;

  /*!  
  * Default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  //!QmitkStdMultiWidget* m_MultiWidget;

  /*!  
  * Controls containing an image selection drop down, some usage information and a "crop" button
  */  
  //!QmitkImageCropperControls* m_Controls;
  Ui::QmitkImageCropperControls * m_Controls;
  
  /*!
  * \brief A pointer to the node of the image to be croped.
  */
  mitk::DataTreeNode::Pointer m_ImageNode;

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
  virtual void CreateBoundingObject();

  /*!
   * \brief Finds the given node in the data tree and fits the cuboid to it
   */
  virtual void AddBoundingObjectToNode(mitk::DataTreeNode::Pointer node);
  
  /*!
   * \brief Removes the cuboid from any node and hides it from the user.
   */
  virtual void RemoveBoundingObjectFromNode();

  template < typename TPixel, unsigned int VImageDimension >
  void AddSurrounding( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image::Pointer image);

private:

  // operation constant
  static const mitk::OperationType OP_EXCHANGE;

  QProgressDialog *progress;
  mitk::Image::Pointer m_surrImage;
};
#endif // !defined(QMITK_IMAGECROPPER_H__INCLUDED)
