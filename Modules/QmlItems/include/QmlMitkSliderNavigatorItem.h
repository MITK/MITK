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

#ifndef __QmlMitkSliderNavigatorItem_h
#define __QmlMitkSliderNavigatorItem_h

#include "MitkQmlItemsExports.h"
#include <mitkStepper.h>
#include <QQuickItem>
#include <QString>

class MITKQMLITEMS_EXPORT QmlMitkSliderNavigatorItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(double min READ getMin WRITE setMin NOTIFY minChanged);
    Q_PROPERTY(double max READ getMax WRITE setMax NOTIFY maxChanged);
    Q_PROPERTY(double value READ getValue WRITE setValue NOTIFY valueChanged);

private:
    double m_Min;
    double m_Max;
    double m_Value;

public:
    QmlMitkSliderNavigatorItem(QQuickItem* parent = nullptr);

    void setMin(double min);
    void setMax(double max);
    void setValue(double value);

    double getMin();
    double getMax();
    double getValue();

    QString GetLabelUnit();

    /**
     * \brief Converts the passed value to a QString representation.
     *
     * If the value exceeds a certain maximum, "INF" (for "infinity") is displayed
     * instead.
     */
    QString ClippedValueToString( float value );

    /**
     * \brief Returns range-minimum (displayed as label left of slider if enabled)
     */
    QString GetMinValueLabel();
    QString GetMaxValueLabel();

    int GetPos();
    static void create();

public slots:
    /**
     * \brief Updates the slider with the recent changes applied to the navigator.
     *
     * Intended to be called via event mechanism, e.g. if the connected
     * mitk::Stepper is modified.
     */
    void Refetch();
    void SetStepper( mitk::Stepper * stepper);
    void ShowLabels( bool show );

    /**
     * \brief En-/disables displaying of the unit label (range will be displayed
     * without unit if enabled).
     */
    void ShowLabelUnit( bool show );
    void SetPos(int val);
    void SetInverseDirection (bool inverseDirection);

protected slots:
    /**
     * \brief Set range minimum and maximum (displayed as labels left and right
     * of slider if enabled)
     */
    void SetLabelValues( float min, float max );
    void SetLabelValuesValid( bool minValid, bool maxValid );

    /**
     * \brief Set range unit (e.g. mm or ms) which will be displayed below range
     * labels if enabled.
     */
    void SetLabelUnit( const char *unit );

protected:
    bool m_HasLabelUnit;
    bool m_MaxValueValid;
    bool m_MinValueValid;
    QString m_LabelUnit;
    mitk::Stepper::Pointer m_Stepper;
    bool m_InRefetch;
    bool m_HasLabels;
    float m_MinValue;
    float m_MaxValue;
    bool m_InverseDirection;
signals:
    void minChanged();
    void maxChanged();
    void valueChanged();
    void sync();
};

#endif
