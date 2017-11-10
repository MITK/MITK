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

#ifndef QmitkInteractiveTransformationWidget_H
#define QmitkInteractiveTransformationWidget_H

//QT headers
#include <QWidget>

//Mitk headers
#include "MitkIGTUIExports.h"
#include "mitkVector.h"
#include "mitkGeometry3D.h"

//ui header
#include "ui_QmitkInteractiveTransformationWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to create a widget to access the advance tool creation options.
  *
  *
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkInteractiveTransformationWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkInteractiveTransformationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkInteractiveTransformationWidget();

    /** Sets the geometry which will be modified by this widget. Default values may be
     *  provided by the second variable. These values will be applied to the geometry
     *  in the beginning and the UI will also hold these values.
     */
    void SetGeometryPointer(mitk::BaseGeometry::Pointer geometry);

    void SetValues(const mitk::AffineTransform3D::Pointer _defaultValues);

    mitk::BaseGeometry::Pointer GetGeometry();

  protected slots:
    void OnZTranslationValueChanged( int v );
    void OnYTranslationValueChanged( int v );
    void OnXTranslationValueChanged( int v );
    void OnZRotationValueChanged( int v );
    void OnYRotationValueChanged( int v );
    void OnXRotationValueChanged( int v );
    void OnResetGeometryToIdentity();
    void OnRevertChanges();
    void OnApplyManipulatedToolTip();

signals:
    void ApplyManipulatedToolTip(mitk::BaseGeometry::Pointer geometry);

  protected:

    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    /*! \brief Method performs the rotation.
    \params rotateVector New rotation to be combined with geometry. */
    void Rotate(mitk::Vector3D rotateVector);

    // Member variables
    Ui::QmitkInteractiveTransformationWidgetControls* m_Controls;

    mitk::BaseGeometry::Pointer m_Geometry;         ///< \brief The geometry that is manipulated
    mitk::BaseGeometry::Pointer m_ResetGeometry;    ///< \brief Lifeline to reset to the original geometry

};
#endif // QmitkInteractiveTransformationWidget_H
