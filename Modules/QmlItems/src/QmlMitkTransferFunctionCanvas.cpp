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

#include "QmlMitkTransferFunctionCanvas.h"

#include <QColorDialog>
#include <QPainter>

QmlMitkTransferFunctionCanvas::QmlMitkTransferFunctionCanvas(QQuickPaintedItem * parent)
: QQuickPaintedItem(parent),
m_GrabbedHandle(-1),
m_Lower(0.0),
m_Upper(1.0),
m_Min(0.0),
m_Max(1.0),
m_Histogram(nullptr),
m_ImmediateUpdate(false),
m_Range(0.0f),
m_LineEditAvailable(false)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAntialiasing(true);
}


std::pair<int,int> QmlMitkTransferFunctionCanvas::FunctionToCanvas(
                                                                   std::pair<double,double> functionPoint)
{
    return std::make_pair((int) ((functionPoint.first - m_Lower) / (m_Upper
                                                                    - m_Lower) * boundingRect().width()) + boundingRect().x(), (int) (boundingRect().height() * (1 - functionPoint.second)) + boundingRect().y());
}

std::pair<double,double> QmlMitkTransferFunctionCanvas::CanvasToFunction(
                                                                         std::pair<int,int> canvasPoint)
{
    return std::make_pair((canvasPoint.first - boundingRect().x()) * (m_Upper - m_Lower) / boundingRect().width()
                          + m_Lower, 1.0 - (double) (canvasPoint.second - boundingRect().y()) / boundingRect().height());
}

void QmlMitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    int nearHandle = GetNearHandle(mouseEvent->pos().x(), mouseEvent->pos().y());
    if (nearHandle != -1)
    {
        this->DoubleClickOnHandle(nearHandle);
    }
}

/** returns index of a near handle or -1 if none is near
 */
int QmlMitkTransferFunctionCanvas::GetNearHandle(int, int, unsigned int)
{
    return -1;
}


void QmlMitkTransferFunctionCanvas::mousePressEvent(QMouseEvent* mouseEvent)
{
    forceActiveFocus();

    if(mouseEvent->button() == Qt::RightButton)
        mouseEvent->setAccepted(false);

    m_GrabbedHandle = GetNearHandle(mouseEvent->pos().x(), mouseEvent->pos().y());

    if ( (mouseEvent->button() & Qt::LeftButton) && m_GrabbedHandle == -1)
    {
        this->AddFunctionPoint(
                               this->CanvasToFunction(std::make_pair(mouseEvent->pos().x(),
                                                                     mouseEvent->pos().y())).first,
                               this->CanvasToFunction(std::make_pair(mouseEvent->x(), mouseEvent->y())).second);
        m_GrabbedHandle = GetNearHandle(mouseEvent->pos().x(),
                                        mouseEvent->pos().y());
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    update();
}

void QmlMitkTransferFunctionCanvas::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if (m_GrabbedHandle != -1)
    {
        std::pair<double,double>
        newPos = this->CanvasToFunction(std::make_pair(mouseEvent->x(),
                                                       mouseEvent->y()));

        // X Clamping
        {
            // Check with predecessor
            if( m_GrabbedHandle > 0 )
                if (newPos.first <= this->GetFunctionX(m_GrabbedHandle - 1))
                    newPos.first = this->GetFunctionX(m_GrabbedHandle);

            // Check with sucessor
            if( m_GrabbedHandle < this->GetFunctionSize()-1 )
                if (newPos.first >= this->GetFunctionX(m_GrabbedHandle + 1))
                    newPos.first = this->GetFunctionX(m_GrabbedHandle);

            // Clamping to histogramm
            if (newPos.first < m_Min) newPos.first = m_Min;
            else if (newPos.first > m_Max) newPos.first = m_Max;
        }

        // Y Clamping
        {
            if (newPos.second < 0.0) newPos.second = 0.0;
            else if (newPos.second > 1.0) newPos.second = 1.0;
        }

        // Move selected point
        this->MoveFunctionPoint(m_GrabbedHandle, newPos);

        update();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmlMitkTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent*)
{
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmlMitkTransferFunctionCanvas::PaintHistogram(QPainter* p)
{
    if(m_Histogram)
    {
        p->save();

        p->setPen(Qt::gray);

        int displayWidth = boundingRect().width();
        int displayHeight = boundingRect().height();

        double windowLeft = m_Lower;
        double windowRight = m_Upper;

        double step = (windowRight-windowLeft)/double(displayWidth);

        double pos = windowLeft;

        for (int x = 0; x < displayWidth; x++)
        {
            double left = pos;
            double right = pos + step;

            float height = m_Histogram->GetRelativeBin( left , right );

            if (height >= 0)
                p->drawLine(x, displayHeight*(1-height), x, displayHeight);

            pos += step;
        }

        p->restore();
    }
}


void QmlMitkTransferFunctionCanvas::keyPressEvent(QKeyEvent * e)
{
    if( m_GrabbedHandle == -1)
        return;

    switch(e->key())
    {
        case Qt::Key_Backspace:
            if(this->GetFunctionSize() > 1)
            {
                this->RemoveFunctionPoint(GetFunctionX(m_GrabbedHandle));
                m_GrabbedHandle = -1;
            }
            break;

        case Qt::Key_Left:
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle)-1 , GetFunctionY(m_GrabbedHandle))));
            break;

        case Qt::Key_Right:
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle)+1 , GetFunctionY(m_GrabbedHandle))));
            break;

        case Qt::Key_Up:
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle) , GetFunctionY(m_GrabbedHandle)+0.001)));
            break;

        case Qt::Key_Down:
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle) , GetFunctionY(m_GrabbedHandle)-0.001)));
            break;
    }

    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

// Update immediatly while changing the transfer function
void QmlMitkTransferFunctionCanvas::SetImmediateUpdate(bool state)
{
    m_ImmediateUpdate = state;
}
