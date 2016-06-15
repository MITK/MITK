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

#ifndef __QmlMitkTransferFunctionCanvas_h
#define __QmlMitkTransferFunctionCanvas_h

#include "MitkQmlItemsExports.h"

#include <mitkSimpleHistogram.h>
#include <mitkRenderingManager.h>

#include <QQuickPaintedItem>

class MITKQMLITEMS_EXPORT QmlMitkTransferFunctionCanvas : public QQuickPaintedItem
{

    Q_OBJECT

public:

    QmlMitkTransferFunctionCanvas(QQuickPaintedItem* parent = nullptr);

    mitk::SimpleHistogram* GetHistogram()
    {
        return m_Histogram;
    }

    void SetHistogram(mitk::SimpleHistogram *histogram)
    {
        m_Histogram = histogram;
    }

    double GetMin()
    {
        return m_Min;
    }

    void SetMin(double min)
    {
        this->m_Min = min;
        SetLower(min);
    }

    double GetMax()
    {
        return m_Max;
    }

    void SetMax(double max)
    {
        this->m_Max = max;
        SetUpper(max);
    }

    double GetLower()
    {
        return m_Lower;
    }

    void SetLower(double lower)
    {
        this->m_Lower = lower;
    }

    double GetUpper()
    {
        return m_Upper;
    }

    void SetUpper(double upper)
    {
        this->m_Upper = upper;
    }

    virtual void paint(QPainter* painter) = 0;

    void mousePressEvent( QMouseEvent* mouseEvent ) override;
    virtual void DoubleClickOnHandle(int handle) = 0;

    void mouseMoveEvent( QMouseEvent* mouseEvent ) override;
    void mouseReleaseEvent( QMouseEvent* mouseEvent ) override;
    void mouseDoubleClickEvent( QMouseEvent* mouseEvent ) override;
    void keyPressEvent(QKeyEvent * event) override;
    void PaintHistogram(QPainter* painter);

    virtual int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32) = 0;
    virtual int AddFunctionPoint(double x,double val) = 0;
    virtual void RemoveFunctionPoint(double x) = 0;
    virtual void MoveFunctionPoint(int index, std::pair<double,double> pos) = 0;
    virtual double GetFunctionX(int index) = 0;
    virtual double GetFunctionY(int index) = 0;
    virtual int GetFunctionSize() = 0;
    int m_GrabbedHandle;

    double m_Lower, m_Upper, m_Min, m_Max;

    std::pair<int,int> FunctionToCanvas(std::pair<double,double>);
    std::pair<double,double> CanvasToFunction(std::pair<int,int>);

    mitk::SimpleHistogram *m_Histogram;

    void SetImmediateUpdate(bool state);

    std::pair<double,double> ValidateCoord( std::pair<double,double> x )
    {
        double max = m_Histogram->GetMax();
        double min = m_Histogram->GetMin();
        if( x.first < min ) x.first = min;
        if( x.first >  max ) x.first =  max;
        if( x.second < 0 ) x.second = 0;
        if( x.second > 1 ) x.second = 1;
        return x;
    }

    void SetX(float x)
    {
        if (m_GrabbedHandle != -1)
        {
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(x,GetFunctionY(m_GrabbedHandle))));
            update();
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }

    void SetY(float y)
    {
        if (m_GrabbedHandle != -1)
        {
            this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(GetFunctionX(m_GrabbedHandle),y)));
            update();
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }

protected:
    bool m_ImmediateUpdate;
    float m_Range;

    double m_GreyValue;
    double m_Opacity;

    bool m_LineEditAvailable;
};

#endif
