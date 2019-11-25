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

#include "mitkChartExampleTestHelper.h"

// std includes
#include <string>

std::unique_ptr<QmitkChartxyData> mitk::ChartExampleTestHelper::GetDataOne()
{
    QmitkChartxyData myDataOne;

    QMap<QVariant, QVariant> data;

    for (int i = 0; i < 10; i++)
    {
        data.insert(i, i);
    }

    myDataOne.SetData(data);
    myDataOne.SetLabel("DataOne");
    myDataOne.SetChartType("bar");
    myDataOne.SetColor("red");
    myDataOne.SetLineStyle("solid");

    return std::make_unique<QmitkChartxyData>(myDataOne);
}

std::unique_ptr<QmitkChartxyData> mitk::ChartExampleTestHelper::GetDataTwo()
{
    QmitkChartxyData myDataTwo;

    QMap<QVariant, QVariant> data;

    for (int i = 10; i < 20; i++)
    {
        data.insert(i, i);
    }

    myDataTwo.SetData(data);
    myDataTwo.SetLabel("DataTwo");
    myDataTwo.SetChartType("bar");
    myDataTwo.SetColor("green");
    myDataTwo.SetLineStyle("solid");

    return std::make_unique<QmitkChartxyData>(myDataTwo);
}

std::unique_ptr<QmitkChartxyData> mitk::ChartExampleTestHelper::GetDataThree()
{
    QmitkChartxyData myDataThree;

    QMap<QVariant, QVariant> data;

    for (int i = 20; i < 30; i++)
    {
        data.insert(i, i);
    }

    myDataThree.SetData(data);
    myDataThree.SetLabel("DataThree");
    myDataThree.SetChartType("bar");
    myDataThree.SetColor("blue");
    myDataThree.SetLineStyle("solid");

    return std::make_unique<QmitkChartxyData>(myDataThree);
}

std::unique_ptr<QmitkChartxyData> mitk::ChartExampleTestHelper::GetDataFour()
{
    QmitkChartxyData myDataFour;

    QMap<QVariant, QVariant> data;

    for (int i = 30; i < 40; i++)
    {
        data.insert(i, i);
    }

    myDataFour.SetData(data);
    myDataFour.SetLabel("DataFour");
    myDataFour.SetChartType("bar");
    myDataFour.SetColor("yellow");
    myDataFour.SetLineStyle("solid");

    return std::make_unique<QmitkChartxyData>(myDataFour);
}

std::unique_ptr<QmitkChartxyData> mitk::ChartExampleTestHelper::GetDataFive()
{
    QmitkChartxyData myDataFive;

    QMap<QVariant, QVariant> data;

    for (int i = 40; i < 50; i++)
    {
        data.insert(i, i);
    }

    myDataFive.SetData(data);
    myDataFive.SetLabel("DataFive");
    myDataFive.SetChartType("bar");
    myDataFive.SetColor("black");
    myDataFive.SetLineStyle("solid");

    return std::make_unique<QmitkChartxyData>(myDataFive);
}

QmitkChartWidget::ChartType mitk::ChartExampleTestHelper::ReturnChartTypeByString(std::string chartTypeString)
{
    if (chartTypeString == "bar")
    {
        return QmitkChartWidget::ChartType::bar;
    }

    if (chartTypeString == "line")
    {
        return QmitkChartWidget::ChartType::line;
    }

    if (chartTypeString == "spline")
    {
        return QmitkChartWidget::ChartType::spline;
    }

    if (chartTypeString == "pie")
    {
        return QmitkChartWidget::ChartType::pie;
    }

    if (chartTypeString == "area")
    {
        return QmitkChartWidget::ChartType::area;
    }

    if (chartTypeString == "area_spline")
    {
        return QmitkChartWidget::ChartType::area_spline;
    }

    if (chartTypeString == "scatter")
    {
        return QmitkChartWidget::ChartType::scatter;
    }

    return QmitkChartWidget::ChartType::bar;
}

QmitkChartWidget::ChartColor mitk::ChartExampleTestHelper::ReturnChartColorByString(std::string chartColorString)
{
    if (chartColorString == "red")
    {
        return QmitkChartWidget::ChartColor::red;
    }

    if (chartColorString == "orange")
    {
        return QmitkChartWidget::ChartColor::orange;
    }

    if (chartColorString == "yellow")
    {
        return QmitkChartWidget::ChartColor::yellow;
    }

    if (chartColorString == "green")
    {
        return QmitkChartWidget::ChartColor::green;
    }

    if (chartColorString == "blue")
    {
        return QmitkChartWidget::ChartColor::blue;
    }

    if (chartColorString == "purple")
    {
        return QmitkChartWidget::ChartColor::purple;
    }

    if (chartColorString == "brown")
    {
        return QmitkChartWidget::ChartColor::brown;
    }

    if (chartColorString == "magenta")
    {
        return QmitkChartWidget::ChartColor::magenta;
    }

    if (chartColorString == "tan")
    {
        return QmitkChartWidget::ChartColor::tan;
    }

    if (chartColorString == "cyan")
    {
        return QmitkChartWidget::ChartColor::cyan;
    }

    if (chartColorString == "olive")
    {
        return QmitkChartWidget::ChartColor::olive;
    }

    if (chartColorString == "maroon")
    {
        return QmitkChartWidget::ChartColor::maroon;
    }

    if (chartColorString == "navy")
    {
        return QmitkChartWidget::ChartColor::navy;
    }

    if (chartColorString == "aquamarine")
    {
        return QmitkChartWidget::ChartColor::aquamarine;
    }

    if (chartColorString == "turqouise")
    {
        return QmitkChartWidget::ChartColor::turqouise;
    }

    if (chartColorString == "silver")
    {
        return QmitkChartWidget::ChartColor::silver;
    }

    if (chartColorString == "lime")
    {
        return QmitkChartWidget::ChartColor::lime;
    }

    if (chartColorString == "teal")
    {
        return QmitkChartWidget::ChartColor::teal;
    }

    if (chartColorString == "indigo")
    {
        return QmitkChartWidget::ChartColor::indigo;
    }

    if (chartColorString == "violet")
    {
        return QmitkChartWidget::ChartColor::violet;
    }

    if (chartColorString == "pink")
    {
        return QmitkChartWidget::ChartColor::pink;
    }

    if (chartColorString == "black")
    {
        return QmitkChartWidget::ChartColor::black;
    }

    if (chartColorString == "white")
    {
        return QmitkChartWidget::ChartColor::white;
    }

    if (chartColorString == "grey")
    {
        return QmitkChartWidget::ChartColor::grey;
    }

    return QmitkChartWidget::ChartColor::red;
}

QmitkChartWidget::LineStyle mitk::ChartExampleTestHelper::ReturnChartStyleByString(std::string chartStyleString)
{
    if (chartStyleString == "solid")
    {
        return QmitkChartWidget::LineStyle::solid;
    }

    if (chartStyleString == "dashed")
    {
        return QmitkChartWidget::LineStyle::dashed;
    }

    return QmitkChartWidget::LineStyle::solid;
}

void mitk::ChartExampleTestHelper::Add(int dataSet)
{
    if (dataSet == 1)
    {
        auto myDataOne = mitk::ChartExampleTestHelper::GetDataOne();

        std::map<double, double> data = mitk::ChartExampleTestHelper::ToStdMap(myDataOne->GetXData(), myDataOne->GetYData());
        auto label = myDataOne->GetLabel().toString().toStdString();
        auto type = mitk::ChartExampleTestHelper::ReturnChartTypeByString((myDataOne->GetChartType().toString().toStdString()));
        auto color = mitk::ChartExampleTestHelper::ReturnChartColorByString((myDataOne->GetColor().toString().toStdString()));
        auto style = mitk::ChartExampleTestHelper::ReturnChartStyleByString((myDataOne->GetLineStyle().toString().toStdString()));

        qmitkChartWidget.AddChartExampleData(data, label, type, color, style);
    }

    if (dataSet == 2)
    {
        auto myDataTwo = mitk::ChartExampleTestHelper::GetDataTwo();

        std::map<double, double> data = mitk::ChartExampleTestHelper::ToStdMap(myDataTwo->GetXData(), myDataTwo->GetYData());
        auto label = myDataTwo->GetLabel().toString().toStdString();
        auto type = mitk::ChartExampleTestHelper::ReturnChartTypeByString((myDataTwo->GetChartType().toString().toStdString()));
        auto color = mitk::ChartExampleTestHelper::ReturnChartColorByString((myDataTwo->GetColor().toString().toStdString()));
        auto style = mitk::ChartExampleTestHelper::ReturnChartStyleByString((myDataTwo->GetLineStyle().toString().toStdString()));

        qmitkChartWidget.AddChartExampleData(data, label, type, color, style);
    }

    if (dataSet == 3)
    {
        auto myDataThree = mitk::ChartExampleTestHelper::GetDataThree();

        std::map<double, double> data = mitk::ChartExampleTestHelper::ToStdMap(myDataThree->GetXData(), myDataThree->GetYData());
        auto label = myDataThree->GetLabel().toString().toStdString();
        auto type = mitk::ChartExampleTestHelper::ReturnChartTypeByString((myDataThree->GetChartType().toString().toStdString()));
        auto color = mitk::ChartExampleTestHelper::ReturnChartColorByString((myDataThree->GetColor().toString().toStdString()));
        auto style = mitk::ChartExampleTestHelper::ReturnChartStyleByString((myDataThree->GetLineStyle().toString().toStdString()));

        qmitkChartWidget.AddChartExampleData(data, label, type, color, style);
    }

    if (dataSet == 4)
    {
        auto myDataFour = mitk::ChartExampleTestHelper::GetDataFour();

        std::map<double, double> data = mitk::ChartExampleTestHelper::ToStdMap(myDataFour->GetXData(), myDataFour->GetYData());
        auto label = myDataFour->GetLabel().toString().toStdString();
        auto type = mitk::ChartExampleTestHelper::ReturnChartTypeByString((myDataFour->GetChartType().toString().toStdString()));
        auto color = mitk::ChartExampleTestHelper::ReturnChartColorByString((myDataFour->GetColor().toString().toStdString()));
        auto style = mitk::ChartExampleTestHelper::ReturnChartStyleByString((myDataFour->GetLineStyle().toString().toStdString()));

        qmitkChartWidget.AddChartExampleData(data, label, type, color, style);
    }

    if (dataSet == 5)
    {
        auto myDataFive = mitk::ChartExampleTestHelper::GetDataFive();

        std::map<double, double> data = mitk::ChartExampleTestHelper::ToStdMap(myDataFive->GetXData(), myDataFive->GetYData());
        auto label = myDataFive->GetLabel().toString().toStdString();
        auto type = mitk::ChartExampleTestHelper::ReturnChartTypeByString((myDataFive->GetChartType().toString().toStdString()));
        auto color = mitk::ChartExampleTestHelper::ReturnChartColorByString((myDataFive->GetColor().toString().toStdString()));
        auto style = mitk::ChartExampleTestHelper::ReturnChartStyleByString((myDataFive->GetLineStyle().toString().toStdString()));

        qmitkChartWidget.AddChartExampleData(data, label, type, color, style);
    }
}

std::map<double, double> mitk::ChartExampleTestHelper::ToStdMap(QVariantList xData, QVariantList yData)
{
    auto xDataConverted = xData.toVector().toStdVector();
    auto yDataConverted = yData.toVector().toStdVector();

    std::map<double, double> data;
    for (int i = 0; i < xDataConverted.size(); i++)
    {
        data.insert_or_assign(xDataConverted[i].toDouble(), yDataConverted[i].toDouble());
    }

    return data;
}

void mitk::ChartExampleTestHelper::ClearMemory()
{
    // Clear the vector
    qmitkChartWidget.Clear();
}
