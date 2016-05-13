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

#include "QmlMitkPiecewiseFunctionCanvas.h"

#include <QPainter>

QmlMitkPiecewiseFunctionCanvas* QmlMitkPiecewiseFunctionCanvas::instance = nullptr;

QmlMitkPiecewiseFunctionCanvas::QmlMitkPiecewiseFunctionCanvas(QQuickPaintedItem* parent) :
QmlMitkTransferFunctionCanvas(parent), m_PiecewiseFunction(nullptr)
{
    instance = this;
    setAcceptedMouseButtons(Qt::AllButtons);
}

void QmlMitkPiecewiseFunctionCanvas::setGreyValue(double value)
{
    this->m_GreyValue = value;
    this->SetX(value);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

double QmlMitkPiecewiseFunctionCanvas::getGreyValue()
{
    return this->m_GreyValue;
}

void QmlMitkPiecewiseFunctionCanvas::setOpacity(double opacity)
{
    this->m_Opacity = opacity;
    this->SetY(opacity);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

double QmlMitkPiecewiseFunctionCanvas::getOpacity()
{
    return this->m_Opacity;
}

void QmlMitkPiecewiseFunctionCanvas::SetTitle(const QString& title)
{
    m_Title=title;
}

void QmlMitkPiecewiseFunctionCanvas::paint(QPainter* painter)
{
    PaintHistogram(painter);

    if (m_Title.size()>0)
    {
        painter->setPen(Qt::black);
        painter->drawText(QPoint(11,21),m_Title);
        painter->setPen(Qt::white);
        painter->drawText(QPoint(10,20),m_Title);
    }

    {

        QString qs_min = QString::number( m_Min, 'g', 4 );
        QString qs_max = QString::number( m_Max, 'g', 4 );

        QRect qr_min = painter->fontMetrics().boundingRect( qs_min );
        QRect qr_max = painter->fontMetrics().boundingRect( qs_max );

        int y,x;

        y=this->boundingRect().height()-qr_min.height()+5;
        //y = this->boundingRect().height()+5;
        x=10;

        // Fill the tf presets in the generator widget
        /*
        painter->setPen(Qt::black);
        painter->drawText(QPoint(x+1,y+1),qs_min);
        painter->setPen(Qt::white);
        painter->drawText(QPoint(x  ,y  ),qs_min);

        y=this->boundingRect().height()-qr_max.height()+5;
        x=this->boundingRect().width()-qr_max.width()-6;

        painter->setPen(Qt::black);
        painter->drawText(QPoint(x,y+1),qs_max);
        painter->setPen(Qt::white);
        painter->drawText(QPoint(x,y  ),qs_max);
         */
    }

    painter->setPen(Qt::gray);

    QRectF contentsRect = this->boundingRect();
    //painter->drawRect(0, 0, contentsRect.width()+1, contentsRect.height()+1);
    if (m_PiecewiseFunction && this->isEnabled())
    {
        double* dp = m_PiecewiseFunction->GetDataPointer();

        // Render lines

        painter->setPen(Qt::black);

        for (int i = -1; i < m_PiecewiseFunction->GetSize(); i++)
        {
            std::pair<int,int> left;
            std::pair<int,int> right;

            if(i < 0)
                left = this->FunctionToCanvas(std::make_pair(-32768, dp[0 * 2 + 1]));
            else
                left = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));

            if(i+1 >= m_PiecewiseFunction->GetSize())
                right = this->FunctionToCanvas(std::make_pair(32768, dp[(i  ) * 2 + 1]));
            else
                right = this->FunctionToCanvas(std::make_pair(dp[(i+1) * 2], dp[(i+1) * 2 + 1]));

            painter->drawLine(left.first, left.second, right.first, right.second);
        }

        // Render Points

        for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
        {
            std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(
                                                                             dp[i * 2], dp[i * 2 + 1]));

            if (i == m_GrabbedHandle)
            {
                painter->setBrush(QBrush(Qt::red));

                this->m_Opacity = QString::number(GetFunctionY(m_GrabbedHandle), 'g', 4).toFloat();
                this->m_GreyValue = QString::number(GetFunctionX(m_GrabbedHandle), 'g', 4).toFloat();

                emit this->sync();

            }
            else
            {
                painter->setBrush(QBrush(Qt::green));
            }
            painter->drawEllipse(point.first - 4, point.second - 4, 8, 8);
        }

        painter->setBrush(Qt::NoBrush);
    }
}

int QmlMitkPiecewiseFunctionCanvas::GetNearHandle(int x, int y,
                                                  unsigned int maxSquaredDistance)
{
    double* dp = m_PiecewiseFunction->GetDataPointer();
    for (int i = 0; i < m_PiecewiseFunction->GetSize(); i++)
    {
        std::pair<int,int> point = this->FunctionToCanvas(std::make_pair(dp[i * 2],
                                                                         dp[i * 2 + 1]));
        if ((unsigned int) ((point.first - x) * (point.first - x) + (point.second
                                                                     - y) * (point.second - y)) <= maxSquaredDistance)
        {
            return i;

        }
    }
    return -1;
}

void QmlMitkPiecewiseFunctionCanvas::MoveFunctionPoint(int index,
                                                       std::pair<double,double> pos)
{
    RemoveFunctionPoint(GetFunctionX(index));
    m_GrabbedHandle = AddFunctionPoint(pos.first, pos.second);
}

void QmlMitkPiecewiseFunctionCanvas::mousePressEvent( QMouseEvent* mouseEvent )
{
    QmlMitkTransferFunctionCanvas::mousePressEvent(mouseEvent);
}

void QmlMitkPiecewiseFunctionCanvas::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    QmlMitkTransferFunctionCanvas::mouseMoveEvent(mouseEvent);
}

void QmlMitkPiecewiseFunctionCanvas::mouseReleaseEvent( QMouseEvent* mouseEvent )
{
    QmlMitkTransferFunctionCanvas::mouseReleaseEvent(mouseEvent);
}

void QmlMitkPiecewiseFunctionCanvas::mouseDoubleClickEvent( QMouseEvent* mouseEvent )
{
    QmlMitkTransferFunctionCanvas::mouseDoubleClickEvent(mouseEvent);
}

void QmlMitkPiecewiseFunctionCanvas::keyPressEvent(QKeyEvent *keyEvent)
{
    QmlMitkTransferFunctionCanvas::keyPressEvent(keyEvent);
}

void QmlMitkPiecewiseFunctionCanvas::create()
{
    qmlRegisterType<QmlMitkPiecewiseFunctionCanvas>("Mitk.Views", 1, 0, "PiecewiseItem");
}
