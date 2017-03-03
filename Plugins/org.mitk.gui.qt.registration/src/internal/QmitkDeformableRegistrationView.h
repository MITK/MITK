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


#ifndef QMITKDEFORMABLEREGISTRATION_H
#define QMITKDEFORMABLEREGISTRATION_H

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkDeformableRegistrationViewControls.h"

#include <org_mitk_gui_qt_registration_Export.h>

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"
#include <QmitkDataStorageComboBox.h>
#include "itkImageFileReader.h"

/*!
\brief The DeformableRegistration view is used to perform deformable registration.

This view allows you to register two 2D as well as two 3D images in a non rigid manner.
Register means to align two images, so that they become as similar as possible.
Therefore you can select from different deformable registration methods.
Registration results will directly be applied to the Moving Image. The result is shown in the multi-widget.

For more informations see: \ref QmitkDeformableRegistrationUserManual

\ingroup DeformableRegistration
*/

class REGISTRATION_EXPORT QmitkDeformableRegistrationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart, public mitk::IRenderWindowPartListener
{

  friend struct SelListenerDeformableRegistration;

  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    typedef itk::Vector< float, 3 >       VectorType;
    typedef itk::Image< VectorType, 3 >   DeformationFieldType;
    typedef itk::ImageFileReader< DeformationFieldType > ImageReaderType;

    /*!
    \brief default constructor
    */
    QmitkDeformableRegistrationView(QObject *parent=0, const char *name=0);

    /*!
    \brief default destructor
    */
    virtual ~QmitkDeformableRegistrationView();

    /*!
    \brief method for creating the applications main widget
    */
    virtual void CreateQtPartControl(QWidget *parent) override;

    ///
    /// Sets the focus to an internal widget.
    ///
    virtual void SetFocus() override;

    virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;

    virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

    /*!
    \brief method for creating the connections of main and control widget
    */
    virtual void CreateConnections();

    virtual void Activated() override;

    virtual void Deactivated() override;

    virtual void Visible() override;

    virtual void Hidden() override;

    void DataNodeHasBeenRemoved(const mitk::DataNode* node);



  signals:

    /*!
    \brief Signal that informs about that the fixed image should be reinitialized in the multi-widget.
    */
    void reinitFixed(const mitk::Geometry3D *);

    /*!
    \brief Signal that informs about that the moving image should be reinitialized in the multi-widget.
    */
    void reinitMoving(const mitk::Geometry3D *);

    /*!
    \brief Signal that informs about that the BSpline registration should be performed.
    */
    void calculateBSplineRegistration();

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
    * stores whether the image will be shown in grayvalues or in red for fixed image and green for moving image
    * @param show if true, then images will be shown in red and green
    */
    void ShowRedGreen(bool show);

    /*!
    * set the selected opacity for moving image
    * @param opacity the selected opacity
    */
    void OpacityUpdate(float opacity);

    /*!
    \brief Sets the selected opacity for moving image

    @param opacity the selected opacity
    */
    void OpacityUpdate(int opacity);

    /*!
    * sets the images to grayvalues or fixed image to red and moving image to green
    * @param redGreen if true, then images will be shown in red and green
    */
    void SetImageColor(bool redGreen);

    /*!
    \brief Checks whether the registration can be performed.
    */
    void CheckCalculateEnabled();

    /*!
    \brief Performs the registration.
    */
    void Calculate();

     /*!
     * Prints the values of the deformationfield
     */
    void ApplyDeformationField();

    void SetImagesVisible(berry::ISelection::ConstPointer selection);

    void TabChanged(int index);

    void SwitchImages();

  protected:

    QWidget* m_Parent;

    QScopedPointer<berry::ISelectionListener> m_SelListener;
    berry::IStructuredSelection::ConstPointer m_CurrentSelection;

    /*!
    * control widget to make all changes for Deformable registration
    */
    Ui::QmitkDeformableRegistrationViewControls m_Controls;
    mitk::DataNode::Pointer m_MovingNode;
    mitk::DataNode::Pointer m_FixedNode;
    bool m_ShowRedGreen;
    float m_Opacity;
    float m_OriginalOpacity;
    mitk::Color m_FixedColor;
    mitk::Color m_MovingColor;
    bool m_Deactivated;
};

#endif //QMITKDEFORMABLEREGISTRATION_H
