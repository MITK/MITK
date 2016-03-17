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

#ifndef __QmlMitkPieceWiseFunctionCanvas_h
#define __QmlMitkPieceWiseFunctionCanvas_h

#include "QmlMitkTransferFunctionCanvas.h"
#include <MitkQmlItemsExports.h>

#include <vtkPiecewiseFunction.h>

#include <mitkTransferFunctionProperty.h>
#include <mitkDataNode.h>

class MITKQMLITEMS_EXPORT QmlMitkPiecewiseFunctionCanvas: public QmlMitkTransferFunctionCanvas
{
    Q_OBJECT
    Q_PROPERTY(double greyValue READ getGreyValue WRITE setGreyValue)
    Q_PROPERTY(double opacity READ getOpacity WRITE setOpacity)

public:

    QmlMitkPiecewiseFunctionCanvas(QQuickPaintedItem* parent = nullptr);
    void paint(QPainter* painter ) override;
    void SetTitle(const QString& title);
    int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32) override;

    static QmlMitkPiecewiseFunctionCanvas* instance;

    void setGreyValue(double value);
    double getGreyValue();

    void setOpacity(double opacity);
    double getOpacity();

    void SetPiecewiseFunction(vtkPiecewiseFunction* piecewiseFunction)
    {
        this->m_PiecewiseFunction = piecewiseFunction;
        this->SetMin(m_PiecewiseFunction->GetRange()[0]);
        this->SetMax(m_PiecewiseFunction->GetRange()[1]);
        setEnabled(true);

        update();
    }

    int AddFunctionPoint(double x,double val) override
    {
        return m_PiecewiseFunction->AddPoint(x,val);
    }

    void RemoveFunctionPoint(double x) override
    {
        int old_size = GetFunctionSize();
        m_PiecewiseFunction->RemovePoint(x);
        if (GetFunctionSize() + 1 != old_size)
        {
            std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
            std::cout << "called with x=" << x << std::endl;
        }
    }

    double GetFunctionX(int index) override
    {
        return m_PiecewiseFunction->GetDataPointer()[index*2];
    }

    double GetFunctionY(int index) override
    {
        return m_PiecewiseFunction->GetValue(m_PiecewiseFunction->GetDataPointer()[index*2]);
    }

    int GetFunctionSize() override
    {
        return m_PiecewiseFunction->GetSize();
    }

    void DoubleClickOnHandle(int) override
    {
    }

    void MoveFunctionPoint(int index, std::pair<double,double> pos) override;

    double GetFunctionMax()
    {
        return m_PiecewiseFunction->GetRange()[1];
    }

    double GetFunctionMin()
    {
        return m_PiecewiseFunction->GetRange()[0];
    }

    double GetFunctionRange()
    {
        double range;
        if((m_PiecewiseFunction->GetRange()[0])<0)
        {
            range = (m_PiecewiseFunction->GetRange()[1])-(m_PiecewiseFunction->GetRange()[0]);
            return range;
        }
        else
        {
            range = m_PiecewiseFunction->GetRange()[1];
            return range;
        }
    }

    void RemoveAllFunctionPoints()
    {
        m_PiecewiseFunction->AddSegment(this->GetFunctionMin(),0,this->GetFunctionMax(),1);
        m_PiecewiseFunction->AddPoint(0.0,0.0);
    }

    void ResetGO()
    {
        //Gradient Opacity
        m_PiecewiseFunction->AddSegment(this->GetFunctionMin(),0,0,1);
        m_PiecewiseFunction->AddSegment(0,1,((this->GetFunctionRange())*0.125),1);
        m_PiecewiseFunction->AddSegment(((this->GetFunctionRange())*0.125),1,((this->GetFunctionRange())*0.2),1);
        m_PiecewiseFunction->AddSegment(((this->GetFunctionRange())*0.2),1,((this->GetFunctionRange())*0.25),1);
    }

    void mousePressEvent( QMouseEvent* mouseEvent ) override;
    void mouseMoveEvent( QMouseEvent* mouseEvent ) override;
    void mouseReleaseEvent( QMouseEvent* mouseEvent ) override;
    void mouseDoubleClickEvent( QMouseEvent* mouseEvent ) override;
    void keyPressEvent(QKeyEvent* keyEvent) override;

    static void create();

protected:
    vtkPiecewiseFunction* m_PiecewiseFunction;
    QString m_Title;
    mitk::SimpleHistogramCache histogramCache;

    double m_GreyValue;
    double m_Opacity;

signals:
    void sync();
};

#endif
