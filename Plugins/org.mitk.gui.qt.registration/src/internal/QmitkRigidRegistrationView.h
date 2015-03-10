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

#ifndef QMITKRIGIDREGISTRATION_H
#define QMITKRIGIDREGISTRATION_H

#include "QmitkFunctionality.h"
#include "ui_QmitkRigidRegistrationViewControls.h"

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"

#include <org_mitk_gui_qt_registration_Export.h>

// Time Slider related
#include <QmitkStepperAdapter.h>

/*!
\brief This functionality allows you to register 2D as well as 3D images in a rigid manner.

Register means to align two images, so that they become as similar as possible.
Therefore you can select from different transforms, metrics and optimizers.
Registration results will directly be applied to the Moving Image.

\sa QmitkFunctionality
\ingroup Functionalities
\ingroup RigidRegistration

\author Daniel Stein
*/

class REGISTRATION_EXPORT QmitkRigidRegistrationView : public QmitkFunctionality
{

  friend struct SelListenerRigidRegistration;

  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    /*!
    \brief default constructor
    */
    QmitkRigidRegistrationView(QObject *parent=0, const char *name=0);

    /*!
    \brief default destructor
    */
    virtual ~QmitkRigidRegistrationView();

    /*!
    \brief method for creating the applications main widget
    */
    virtual void CreateQtPartControl(QWidget *parent);

    /*!
    \brief Sets the StdMultiWidget and connects it to the functionality.
    */
    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);

    /*!
    \brief Removes the StdMultiWidget and disconnects it from the functionality.
    */
    virtual void StdMultiWidgetNotAvailable();

    /*!
    \brief method for creating the connections of main and control widget
    */
    virtual void CreateConnections();

    /*!
    \brief Method which is called when this functionality is selected in MITK
    */
    virtual void Activated();

    /*!
    \brief Method which is called whenever the functionality is deselected in MITK
    */
    virtual void Deactivated();

    virtual void Visible();
    virtual void Hidden();

    void DataNodeHasBeenRemoved(const mitk::DataNode* node);

  signals:

  protected slots:

    /*!
    * sets the fixed Image according to TreeNodeSelector widget
    */
    void FixedSelected(mitk::DataNode::Pointer fixedImage);

    /*!
    * sets the moving Image according to TreeNodeSelector widget
    */
    void MovingSelected(mitk::DataNode::Pointer movingImage);

    /*!
    * checks if registration is possible
    */
    bool CheckCalculate();

    /*!
    * \brief Undo the last registration.
    */
    void UndoTransformation();

    /*!
    * \brief Redo the last registration
    */
    void RedoTransformation();

    /*!
    * \brief Adds a new Transformation to the undo list and enables the undo button.
    */
    void AddNewTransformationToUndoList();

    /*!
    * \brief Translates the moving image in x, y and z direction given by translateVector
    *
    * @param translateVector Contains the translation in x, y and z direction.
    */
    void Translate(int* translateVector);

    /*!
    * \brief Rotates the moving image in x, y and z direction given by rotateVector
    *
    * @param rotateVector Contains the rotation around x, y and z axis.
    */
    void Rotate(int* rotateVector);

    /*!
    * \brief Scales the moving image in x, y and z direction given by scaleVector
    *
    * @param scaleVector Contains the scaling around x, y and z axis.
    */
    void Scale(int* scaleVector);

    /*!
    * \brief Automatically aligns the image centers.
    */
    void AlignCenters();

    /*!
    * \brief Stores whether the image will be shown in gray values or in red for fixed image and green for moving image
    * @param show if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*!
    * \brief Draws the contour of the fixed image according to a threshold
    * @param show if true, then images will be shown in red and green
    */
    void ShowContour();

    /*!
    * \brief Stores whether the contour of the fixed image will be shhown
    * @param show if true, the contour of the fixed image is shown
    */
    void EnableContour(bool show);

    /*!
    * \brief Changes the visibility of the manual registration methods accordingly to the checkbox "Manual Registration" in GUI
    * @param show if true, then manual registration methods will be shown
    */
    void ShowManualRegistrationFrame(bool show);

    /*!
    * \brief Sets the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(int opacity);

    /*!
    * \brief Sets the images to grayvalues or fixed image to red and moving image to green
    * @param redGreen if true, then images will be shown in red and green
    */
    void SetImageColor(bool redGreen);

    /*!
    * \brief Clears the undo and redo lists and sets the undo and redo buttons to disabled.
    */
    void ClearTransformationLists();

    void SetUndoEnabled( bool enable );

    void SetRedoEnabled( bool enable );

    void CheckCalculateEnabled();

    void xTrans_valueChanged( int v );

    void yTrans_valueChanged( int v );

    void zTrans_valueChanged( int v );

    void xRot_valueChanged( int v );

    void yRot_valueChanged( int v );

    void zRot_valueChanged( int v );

    void xScale_valueChanged( int v );

    void yScale_valueChanged( int v );

    void zScale_valueChanged( int v );

    void MovingImageChanged();

    /*!
    * \brief Starts the registration process.
    */
    void Calculate();

    void SetOptimizerValue( double value );

    void StopOptimizationClicked();

    void UpdateTimestep();

    void SetImagesVisible(berry::ISelection::ConstPointer /*selection*/);

    //void CheckForMaskImages();

    //void UseFixedMaskImageChecked(bool checked);

    //void UseMovingMaskImageChecked(bool checked);

    void TabChanged(int index);

    void SwitchImages();

    /*!
    * indicates if the masking option shall be used.
    */
    void OnUseMaskingChanged(int state);

    /*!
    * method called if the fixed mask image selector changed.
    */
    void OnFixedMaskImageChanged(const mitk::DataNode* node);
    /*!
    * method called if the moving mask image selector changed.
    */
    void OnMovingMaskImageChanged(const mitk::DataNode* node);

  protected:

    QScopedPointer<berry::ISelectionListener> m_SelListener;
    berry::IStructuredSelection::ConstPointer m_CurrentSelection;

    /*!
    * default main widget containing 4 windows showing 3
    * orthogonal slices of the volume and a 3d render window
    */
    QmitkStdMultiWidget * m_MultiWidget;

    /*!
    * control widget to make all changes for Deformable registration
    */
    Ui::QmitkRigidRegistrationViewControls m_Controls;
    mitk::DataNode::Pointer m_MovingNode;
    mitk::DataNode::Pointer m_MovingMaskNode;
    mitk::DataNode::Pointer m_FixedNode;
    mitk::DataNode::Pointer m_FixedMaskNode;



    // A node to store the contour of the fixed image in
    mitk::DataNode::Pointer m_ContourHelperNode;

    typedef std::map<mitk::DataNode::Pointer, mitk::Geometry3D::Pointer> GeometryMapType;
    typedef std::list< GeometryMapType > GeometryMapListType;
    typedef std::list<mitk::Geometry3D::Pointer> GeometryListType;

    GeometryListType m_UndoGeometryList;
    GeometryMapListType m_UndoChildGeometryList;
    GeometryListType m_RedoGeometryList;
    GeometryMapListType m_RedoChildGeometryList;
    bool m_ShowRedGreen;
    float m_Opacity;
    float m_OriginalOpacity;
    bool m_Deactivated;
    int m_FixedDimension;
    int m_MovingDimension;
    int * translationParams;
    int * rotationParams;
    int * scalingParams;
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    int m_TranslateSliderPos[3];
    int m_RotateSliderPos[3];
    int m_ScaleSliderPos[3];

    QmitkStepperAdapter*      m_TimeStepperAdapter;
};

#endif //QMITKRigidREGISTRATION_H
