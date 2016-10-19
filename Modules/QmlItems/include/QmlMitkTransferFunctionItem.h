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

#ifndef __QmlmMitkTransferFunctionItem_h
#define __QmlmMitkTransferFunctionItem_h

#include "MitkQmlItemsExports.h"

#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

#include <QmlMitkPiecewiseFunctionCanvas.h>
#include <QmlMitkColorTransferFunctionCanvas.h>

#include <QQuickItem>
#include <QSlider>
#include <QPushButton>

namespace mitk
{
    class BaseRenderer;
}

class MITKQMLITEMS_EXPORT QmlMitkTransferFunctionItem: public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QmlMitkPiecewiseFunctionCanvas* opacityItem READ getOpacityItem WRITE setOpactityItem);
    Q_PROPERTY(QmlMitkColorTransferFunctionCanvas* colorItem READ getColorItem WRITE setColorItem);

    Q_PROPERTY(double min READ getMin WRITE setMin NOTIFY minChanged);
    Q_PROPERTY(double max READ getMax WRITE setMax NOTIFY maxChanged);
    Q_PROPERTY(double currentMin READ getCurrentMin WRITE setCurrentMin NOTIFY currentMinChanged);
    Q_PROPERTY(double currentMax READ getCurrentMax WRITE setCurrentMax NOTIFY currentMaxChanged);
public:
    static QmlMitkTransferFunctionItem* instance;

    QmlMitkTransferFunctionItem(QQuickItem* parent = nullptr);
    ~QmlMitkTransferFunctionItem () ;

    void SetDataNode(mitk::DataNode* node, const mitk::BaseRenderer* renderer = nullptr);

public slots:
    void OnUpdateCanvas();
    void UpdateRanges();
    void OnResetSlider();

    void wheelEvent(QWheelEvent* event) override;

    void OnSpanChanged (int lower, int upper);

    void setOpactityItem(QmlMitkPiecewiseFunctionCanvas* item);
    void setColorItem(QmlMitkColorTransferFunctionCanvas* item);

    QmlMitkPiecewiseFunctionCanvas* getOpacityItem();
    QmlMitkColorTransferFunctionCanvas* getColorItem();

    void setMin(double min);
    void setMax(double max);
    void setCurrentMin(double currentMin);
    void setCurrentMax(double currentMax);

    double getMin();
    double getMax();
    double getCurrentMin();
    double getCurrentMax();

    static void create();

protected:
    mitk::TransferFunctionProperty::Pointer tfpToChange;

    int m_Min;
    int m_Max;

    int m_CurrentMin;
    int m_CurrentMax;

    QmlMitkColorTransferFunctionCanvas* m_ColorItem;
    QmlMitkPiecewiseFunctionCanvas* m_OpacityItem;

    mitk::SimpleHistogramCache histogramCache;

signals:
    void currentMinChanged();
    void currentMaxChanged();
    void minChanged();
    void maxChanged();
    void sync();
};

#endif
