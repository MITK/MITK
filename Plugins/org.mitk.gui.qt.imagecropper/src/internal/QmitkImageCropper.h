/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#if !defined(QMITK_IMAGECROPPER_H__INCLUDED)
#define QMITK_IMAGECROPPER_H__INCLUDED

#ifdef WIN32
#pragma warning( disable : 4250 )
#endif

#include "QmitkFunctionality.h"
#include <mitkCuboid.h>
#include <mitkOperationActor.h>
#include <mitkOperation.h>
#include <mitkAffineInteractor.h>
#include "mitkWeakPointer.h"
#include <itkImage.h>
#include <QProgressDialog>

#include "mitkImageCropperEventInterface.h"

#include "ui_QmitkImageCropperControls.h"


/*!
\ingroup org_mitk_gui_qt_imagecropper_internal
\brief Functionality for cropping images with a cuboid

This functionality lets the user select an image from the data tree, select an area of interest by placing
a cuboid object, and then crop the image, so that pixels from outside the cuboid will remove.

The image size is automatically reduced, if the cuboid is not rotated but parallel to the image axes.

\b Implementation

The functionality owns a cuboid (m_CroppingObject) and the associated interactor (m_AffineInteractor),
which implements moving and scaling the cuboid.

*/

class QmitkImageCropper : public QmitkFunctionality, public mitk::OperationActor
{

  /// Operation base class, which holds pointers to a node of the data tree (mitk::DataNode)
  /// and to two data sets (mitk::BaseData) instances
  class opExchangeNodes: public mitk::Operation
  {
  public: opExchangeNodes( mitk::OperationType type,  mitk::DataNode* node,
            mitk::BaseData* oldData,
            mitk::BaseData* newData );
          ~opExchangeNodes();
          mitk::DataNode* GetNode() { return m_Node; }
          mitk::BaseData* GetOldData() { return m_OldData; }
          mitk::BaseData* GetNewData() { return m_NewData; }
  protected:
    void NodeDeleted(const itk::Object * /*caller*/, const itk::EventObject & /*event*/);
  private:
    mitk::DataNode* m_Node;
    mitk::BaseData::Pointer m_OldData;
    mitk::BaseData::Pointer m_NewData;
    long m_NodeDeletedObserverTag;
    long m_OldDataDeletedObserverTag;
    long m_NewDataDeletedObserverTag;
  };

private:

  Q_OBJECT

public:
  /*!
  \brief Constructor. Called by SampleApp (or other apps that use functionalities)
  */
  QmitkImageCropper(QObject *parent=0);

  /*!
  \brief Destructor
  */
  virtual ~QmitkImageCropper();

  /*!
  \brief Creates the Qt widget containing the functionality controls, like sliders, buttons etc.
  */
  virtual void CreateQtPartControl(QWidget* parent);

  /*!
  \brief Creates the Qt connections needed
  */
  virtual void CreateConnections();

  /*!
  \brief Invoked when this functionality is selected by the application
  */
  virtual void Activated();

  /*!
  \brief Invoked when the user leaves this functionality
  */
  virtual void Deactivated();

  ///
  /// Called when a StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when no StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetNotAvailable();


  /*
  \brief Interface of a mitk::StateMachine (for undo/redo)
  */
  virtual void  ExecuteOperation (mitk::Operation*);

  QWidget* GetControls();


  public slots:

    virtual void CropImage();
    virtual void SurroundingCheck(bool value);
    virtual void CreateNewBoundingObject();
    virtual void ChkInformationToggled( bool on );

protected:

  /*!
  * Default main widget containing 4 windows showing 3
  * orthogonal slices of the volume and a 3d render window
  */
  QmitkStdMultiWidget* m_MultiWidget;

  /*!
  * Controls containing an image selection drop down, some usage information and a "crop" button
  */
  Ui::QmitkImageCropperControls * m_Controls;

  /*!
  * The parent QWidget
  */
  QWidget* m_ParentWidget;

  /*!
  * \brief A pointer to the node of the image to be croped.
  */
  mitk::WeakPointer<mitk::DataNode> m_ImageNode;

  /*!
  * \brief A pointer to the image to be cropped.
  */
  mitk::WeakPointer<mitk::Image> m_ImageToCrop;

  /*!
  * \brief The cuboid used for cropping.
  */
  mitk::BoundingObject::Pointer m_CroppingObject;

  /*!
  * \brief Tree node of the cuboid used for cropping.
  */
  mitk::DataNode::Pointer m_CroppingObjectNode;

  /*!
  * \brief Interactor for moving and scaling the cuboid
  */
  mitk::AffineInteractor::Pointer m_AffineInteractor;

  /*!
  * \brief Creates the cuboid and its data tree node.
  */
  virtual void CreateBoundingObject();

  /*!
  * \brief Called from superclass, handles selection changes.
  */
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

  /*!
  * \brief Finds the given node in the data tree and optionally fits the cuboid to it
  */
  virtual void AddBoundingObjectToNode(mitk::DataNode* node, bool fit);

  /*!
  * \brief Removes the cuboid from any node and hides it from the user.
  */
  virtual void RemoveBoundingObjectFromNode();

  virtual void NodeRemoved(const mitk::DataNode* node);

private:

  // operation constant
  static const mitk::OperationType OP_EXCHANGE;

  //Interface class for undo redo
  mitk::ImageCropperEventInterface* m_Interface;
};
#endif // !defined(QMITK_IMAGECROPPER_H__INCLUDED)
