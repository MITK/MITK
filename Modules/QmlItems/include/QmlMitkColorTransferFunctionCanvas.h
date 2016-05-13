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

#ifndef __QmlMitkColorTransferFunctionCanvas_h
#define __QmlMitkColorTransferFunctionCanvas_h

#include "QmlMitkTransferFunctionCanvas.h"
#include <MitkQmlItemsExports.h>

#include <vtkColorTransferFunction.h>

class MITKQMLITEMS_EXPORT QmlMitkColorTransferFunctionCanvas: public QmlMitkTransferFunctionCanvas
{
    Q_OBJECT
    Q_PROPERTY(double greyValue READ getGreyValue WRITE setGreyValue)

public:

    QmlMitkColorTransferFunctionCanvas(QQuickPaintedItem* parent = nullptr);
    virtual void paint(QPainter* painter ) override;
    int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32) override;
    void SetTitle(const QString& title);

    void setGreyValue(double value);
    double getGreyValue();

    void SetColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
    {
        this->m_ColorTransferFunction = colorTransferFunction;
        this->SetMin(colorTransferFunction->GetRange()[0]);
        this->SetMax(colorTransferFunction->GetRange()[1]);
        setEnabled(true);
        update();
    }

    int AddFunctionPoint(double x, double) override
    {
        return m_ColorTransferFunction->AddRGBPoint(x,m_ColorTransferFunction->GetRedValue(x),m_ColorTransferFunction->GetGreenValue(x),m_ColorTransferFunction->GetBlueValue(x));
    }

    void RemoveFunctionPoint(double x) override
    {
        int old_size = GetFunctionSize();
        m_ColorTransferFunction->RemovePoint(x);
        if (GetFunctionSize() + 1 != old_size)
        {
            std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
            std::cout << "called with x=" << x << std::endl;
        }
    }

    double GetFunctionX(int index) override
    {
        return m_ColorTransferFunction->GetDataPointer()[index*4];
    }

    int GetFunctionSize() override
    {
        return m_ColorTransferFunction->GetSize();
    }

    void DoubleClickOnHandle(int handle) override;
    void MoveFunctionPoint(int index, std::pair<double,double> pos) override;

    void AddRGB(double x, double r, double g, double b);

    double GetFunctionMax()
    {
        return m_ColorTransferFunction->GetRange()[1];
    }

    double GetFunctionMin()
    {
        return m_ColorTransferFunction->GetRange()[0];
    }

    double GetFunctionRange()
    {
        double range;
        if((m_ColorTransferFunction->GetRange()[0])==0)
        {
            range = m_ColorTransferFunction->GetRange()[1];
            return range;
        }
        else
        {
            range = (m_ColorTransferFunction->GetRange()[1])-(m_ColorTransferFunction->GetRange()[0]);
            return range;
        }
    }

    void RemoveAllFunctionPoints()
    {
        m_ColorTransferFunction->AddRGBSegment(this->GetFunctionMin(),1,0,0,this->GetFunctionMax(),1,1,0);
    }

    double GetFunctionY(int) override
    {
        return 0.0;
    }

    static void create();

protected:
    vtkColorTransferFunction* m_ColorTransferFunction;
    QString m_Title;

    double m_GreyValue;

signals:
    void sync();

};

#endif
