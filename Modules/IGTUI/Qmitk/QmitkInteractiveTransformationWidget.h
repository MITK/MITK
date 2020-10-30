/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInteractiveTransformationWidget_H
#define QmitkInteractiveTransformationWidget_H

//QT headers
#include <QDialog>

//Mitk headers
#include "MitkIGTUIExports.h"
#include "mitkVector.h"
#include "mitkGeometry3D.h"
#include "mitkNavigationTool.h"

//ui header
#include "ui_QmitkInteractiveTransformationWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to create a widget to access the advanced tool creation options.
  *
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkInteractiveTransformationWidget : public QDialog
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkInteractiveTransformationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkInteractiveTransformationWidget() override;

    /** This tool will be copied to m_ToolToEdit. It will not be changed.
        To apply any changes made by this widget, you will need to connect to the signal
        EditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip) and set this transfrom
        as calibrated tool tip.
        We do not directly modify the tool to allow to cancel/exit this widget without doing
        any harm.
     */
    void SetToolToEdit(const mitk::NavigationTool::Pointer _tool);

    /** The sliders and spinboxes will be set to these values.
        When clicking "Revert Changes", sliders will be reseted to these values.
    */
    void SetDefaultOffset(const mitk::Point3D _defaultValues);
    void SetDefaultRotation(const mitk::Quaternion _defaultValues);

  protected slots:
    void OnZTranslationValueChanged(double v);
    void OnYTranslationValueChanged(double v);
    void OnXTranslationValueChanged(double v);
    void OnZRotationValueChanged(double v);
    void OnYRotationValueChanged(double v);
    void OnXRotationValueChanged(double v);
    void OnResetGeometryToIdentity();
    void OnRevertChanges();
    void OnApplyManipulatedToolTip();
    void OnCancel();

signals:
    void EditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip);

  protected:

    void reject() override;

    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    /*! \brief Method performs the rotation.
    \param rotateVector New rotation to be combined with geometry. */
    void Rotate(mitk::Vector3D rotateVector);

    // Member variables
    Ui::QmitkInteractiveTransformationWidgetControls* m_Controls;

    mitk::NavigationTool::Pointer m_ToolToEdit;     ///< \brief this member holds a copy of the tool that should be edited for visualization
    mitk::BaseGeometry::Pointer m_Geometry;         ///< \brief The geometry that is manipulated
    mitk::BaseGeometry::Pointer m_ResetGeometry;    ///< \brief Lifeline to reset to the original geometry

private:
  void SetValuesToGUI(const mitk::AffineTransform3D::Pointer _defaultValues);
  void SetSynchronizedValuesToSliderAndSpinbox(QDoubleSpinBox* _spinbox, QSlider* _slider, double _value);

};
#endif // QmitkInteractiveTransformationWidget_H
