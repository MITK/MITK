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

#include "QmlMitkTransferFunctionItem.h"

#include <QQuickWindow>

QmlMitkTransferFunctionItem* QmlMitkTransferFunctionItem::instance = nullptr;

QmlMitkTransferFunctionItem::QmlMitkTransferFunctionItem(QQuickItem* parent):
QQuickItem(parent)
{
    instance = this;

    this->m_Max = 0;
    this->m_Min = 0;

    this->m_CurrentMin = 0;
    this->m_CurrentMax = 0;

    this->setEnabled(false);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

QmlMitkTransferFunctionItem::~QmlMitkTransferFunctionItem()
{
}

void QmlMitkTransferFunctionItem::wheelEvent(QWheelEvent *event)
{
    int fact = -event->delta() * 0.15;

    if(event->orientation() == Qt::Horizontal)
    {
        int lower = this->m_CurrentMin + fact;
        int upper = this->m_CurrentMax + fact;

        if(lower < this->m_Min )
        {
            this->m_CurrentMin = this->m_Min;
        }
        else if (upper > this->m_Max)
        {
            this->m_CurrentMax = this->m_Max;
        }
        else
        {
            this->m_CurrentMin = lower;
            this->m_CurrentMax = upper;
        }
        emit this->sync();

        UpdateRanges();
    }
}

void QmlMitkTransferFunctionItem::SetDataNode(mitk::DataNode* node, const mitk::BaseRenderer* renderer)
{
    if (node)
    {
        tfpToChange = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction", renderer));

        if(!tfpToChange)
        {
            if (! dynamic_cast<mitk::Image*>(node->GetData()))
            {
                MITK_WARN << "QmlMitkTransferFunctionItem::SetDataNode called with non-image node";
                //  goto turnOff;
            }

            node->SetProperty("TransferFunction", tfpToChange = mitk::TransferFunctionProperty::New() );
        }

        mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

        if( mitk::BaseData* data = node->GetData() )
        {
            mitk::SimpleHistogram *h = histogramCache[data];

            this->setMin(h->GetMin());
            this->setMax(h->GetMax());

            this->m_CurrentMin = h->GetMin();
            this->m_CurrentMax = h->GetMax();

            this->m_ColorItem->SetHistogram( h );
            this->m_OpacityItem->SetHistogram( h );
            //    m_ColorTransferFunctionCanvas->SetHistogram( h );

            emit this->sync();
        }

        OnUpdateCanvas();

        return;
    }
}

void QmlMitkTransferFunctionItem::OnUpdateCanvas()
{

    if(tfpToChange.IsNull())
        return;

    mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

    if(tf.IsNull())
        return;

    this->m_OpacityItem->SetPiecewiseFunction( tf->GetScalarOpacityFunction() );
    //m_GradientOpacityCanvas->SetPiecewiseFunction( tf->GetGradientOpacityFunction() );
    this->m_ColorItem->SetColorTransferFunction( tf->GetColorTransferFunction() );

    UpdateRanges();

    this->m_ColorItem->update();
    //m_GradientOpacityCanvas->update();
    this->m_OpacityItem->update();
}

void QmlMitkTransferFunctionItem::setColorItem(QmlMitkColorTransferFunctionCanvas *item)
{
    this->m_ColorItem = item;
}

QmlMitkColorTransferFunctionCanvas* QmlMitkTransferFunctionItem::getColorItem()
{
    return this->m_ColorItem;
}

void QmlMitkTransferFunctionItem::setOpactityItem(QmlMitkPiecewiseFunctionCanvas *item)
{
    this->m_OpacityItem = item;
}

QmlMitkPiecewiseFunctionCanvas* QmlMitkTransferFunctionItem::getOpacityItem()
{
    return this->m_OpacityItem;
}

void QmlMitkTransferFunctionItem::UpdateRanges()
{
    int lower = this->m_CurrentMin;
    int upper = this->m_CurrentMax;

    this->m_OpacityItem->SetMin(lower);
    this->m_OpacityItem->SetMax(upper);

    // m_GradientOpacityCanvas->SetMin(lower);
    // m_GradientOpacityCanvas->SetMax(upper);

    this->m_ColorItem->SetMin(lower);
    this->m_ColorItem->SetMax(upper);

    this->m_OpacityItem->update();
    this->m_ColorItem->update();
}


void QmlMitkTransferFunctionItem::OnSpanChanged(int, int)
{
    UpdateRanges();

    //m_GradientOpacityCanvas->update();
    this->m_OpacityItem->update();
    this->m_ColorItem->update();
}

void QmlMitkTransferFunctionItem::OnResetSlider()
{
    this->m_CurrentMin = this->m_Min;
    this->m_CurrentMax = this->m_Max;

    UpdateRanges();
}

void QmlMitkTransferFunctionItem::setMin(double min)
{
    this->m_Min = min;
    emit this->minChanged();
}

void QmlMitkTransferFunctionItem::setMax(double max)
{
    this->m_Max = max;
    emit this->maxChanged();
}

void QmlMitkTransferFunctionItem::setCurrentMin(double currentMin)
{
    this->m_CurrentMin = currentMin;
    UpdateRanges();
}

void QmlMitkTransferFunctionItem::setCurrentMax(double currentMax)
{
    this->m_CurrentMax = currentMax;
    UpdateRanges();
}

double QmlMitkTransferFunctionItem::getMin()
{
    return this->m_Min;
}

double QmlMitkTransferFunctionItem::getMax()
{
    return this->m_Max;
}

double QmlMitkTransferFunctionItem::getCurrentMin()
{
    return this->m_CurrentMin;
}

double QmlMitkTransferFunctionItem::getCurrentMax()
{
    return this->m_CurrentMax;
}

void QmlMitkTransferFunctionItem::create()
{
    QmlMitkPiecewiseFunctionCanvas::create();
    QmlMitkColorTransferFunctionCanvas::create();
    qmlRegisterType<QmlMitkTransferFunctionItem>("Mitk.Views", 1, 0, "TransferItem");
}
