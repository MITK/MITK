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

#ifndef __QmlMitkImageNavigator_h
#define __QmlMitkImageNavigator_h

#include "MitkQmlItemsExports.h"
#include <QmlMitkRenderWindowItem.h>
#include <QmlMitkSliderNavigatorItem.h>

#include <mitkVector.h>
#include <QQuickitem>
#include <QQmlEngine>

class QmitkStepperAdapter;

/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \class QmlMitkImageNavigator
 *
 * \brief Provides a means to scan quickly through a dataset via Axial,
 * Coronal and Sagittal sliders, displaying millimetre location and stepper position.
 *
 * For images, the stepper position corresponds to a voxel index. For other datasets
 * such as a surface, it corresponds to a sub-division of the bounding box.
 *
 * \sa QmitkAbstractView
 */
class MITKQMLITEMS_EXPORT QmlMitkImageNavigator : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QmlMitkSliderNavigatorItem* navigatorAxial READ getNavigatorAxial WRITE setNavigatorAxial)
    Q_PROPERTY(QmlMitkSliderNavigatorItem* navigatorSagittal READ getNavigatorSagittal WRITE setNavigatorSagittal)
    Q_PROPERTY(QmlMitkSliderNavigatorItem* navigatorCoronal READ getNavigatorCoronal WRITE setNavigatorCoronal)
    Q_PROPERTY(QmlMitkSliderNavigatorItem* navigatorTime READ getNavigatorTime WRITE setNavigatorTime)

    Q_PROPERTY(double worldCoordinateX READ getWorldCoordinateX WRITE setWorldCoordinateX)
    Q_PROPERTY(double worldCoordinateY READ getWorldCoordinateY WRITE setWorldCoordinateY)
    Q_PROPERTY(double worldCoordinateZ READ getWorldCoordinateZ WRITE setWorldCoordinateZ)

    Q_PROPERTY(double worldCoordinateXMin READ getWorldCoordinateXMin WRITE setWorldCoordinateXMin)
    Q_PROPERTY(double worldCoordinateYMin READ getWorldCoordinateYMin WRITE setWorldCoordinateYMin)
    Q_PROPERTY(double worldCoordinateZMin READ getWorldCoordinateZMin WRITE setWorldCoordinateZMin)

    Q_PROPERTY(double worldCoordinateXMax READ getWorldCoordinateXMax WRITE setWorldCoordinateXMax)
    Q_PROPERTY(double worldCoordinateYMax READ getWorldCoordinateYMax WRITE setWorldCoordinateYMax)
    Q_PROPERTY(double worldCoordinateZMax READ getWorldCoordinateZMax WRITE setWorldCoordinateZMax)

private:
    QmlMitkSliderNavigatorItem* m_NavigatorAxial;
    QmlMitkSliderNavigatorItem* m_NavigatorSagittal;
    QmlMitkSliderNavigatorItem* m_NavigatorCoronal;
    QmlMitkSliderNavigatorItem* m_NavigatorTime;

    double m_WorldCoordinateX;
    double m_WorldCoordinateY;
    double m_WorldCoordinateZ;

    double m_WorldCoordinateXMin;
    double m_WorldCoordinateYMin;
    double m_WorldCoordinateZMin;

    double m_WorldCoordinateXMax;
    double m_WorldCoordinateYMax;
    double m_WorldCoordinateZMax;

public:
    static QmlMitkImageNavigator* instance;

    QmlMitkImageNavigator();
    ~QmlMitkImageNavigator();

    static void create(QQmlEngine &engine);

    void setNavigatorAxial(QmlMitkSliderNavigatorItem* item);
    void setNavigatorSagittal(QmlMitkSliderNavigatorItem* item);
    void setNavigatorCoronal(QmlMitkSliderNavigatorItem* item);
    void setNavigatorTime(QmlMitkSliderNavigatorItem* item);

    QmlMitkSliderNavigatorItem* getNavigatorAxial();
    QmlMitkSliderNavigatorItem* getNavigatorSagittal();
    QmlMitkSliderNavigatorItem* getNavigatorCoronal();
    QmlMitkSliderNavigatorItem* getNavigatorTime();

    void setWorldCoordinateX(double coordinate);
    void setWorldCoordinateY(double coordinate);
    void setWorldCoordinateZ(double coordinate);

    void setWorldCoordinateXMin(double coordinate);
    void setWorldCoordinateYMin(double coordinate);
    void setWorldCoordinateZMin(double coordinate);

    void setWorldCoordinateXMax(double coordinate);
    void setWorldCoordinateYMax(double coordinate);
    void setWorldCoordinateZMax(double coordinate);

    double getWorldCoordinateX();
    double getWorldCoordinateY();
    double getWorldCoordinateZ();

    double getWorldCoordinateXMin();
    double getWorldCoordinateYMin();
    double getWorldCoordinateZMin();

    double getWorldCoordinateXMax();
    double getWorldCoordinateYMax();
    double getWorldCoordinateZMax();

protected slots:
    void initialize();
    void OnMillimetreCoordinateValueChanged();
    void OnRefetch();

protected:
    //void SetBorderColors();
    //void SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString);
    //void SetBorderColor(int axis, QString colorAsStyleSheetString);
    void SetStepSizes();
    void SetStepSize(int axis);
    void SetStepSize(int axis, double stepSize);
    int  GetClosestAxisIndex(mitk::Vector3D normal);

    QmitkStepperAdapter* m_AxialStepper;
    QmitkStepperAdapter* m_SagittalStepper;
    QmitkStepperAdapter* m_FrontalStepper;
    QmitkStepperAdapter* m_TimeStepper;

    /**
     * @brief GetDecorationColorOfGeometry helper method to get the color of a helper geometry node.
     * @param renderWindow The renderwindow of the geometry
     * @return the color for decoration in QString format (#RRGGBB).
     */
    //QString GetDecorationColorOfGeometry(QmlMitkRenderWindowItem *renderWindow);
signals:
    void sync();
};

#endif
