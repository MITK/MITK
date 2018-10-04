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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// qt includes
#include "QApplication"

// std includes
#include <string>

// MITK includes
#include "QmitkChartWidget.h"
#include "QmitkChartxyData.h"
#include "QmitkChartData.h"

class mitkChartWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkChartWidgetTestSuite);

  // Test the dicom property parsing
  MITK_TEST(AddOnce_Test_Success);
  MITK_TEST(AddSameLabelTwice_Test_Success);
  MITK_TEST(RemoveNonexistingData_Failure);
  MITK_TEST(RemoveData_Success);
  MITK_TEST(SetandGet_Success);
  MITK_TEST(SetC3DataAndGet_Success);
  MITK_TEST(AddAndRemoveData_Sucess);
  CPPUNIT_TEST_SUITE_END();

private:
  std::vector<double> data1D;
  std::string label;

public:
  void setUp() override
  {
    data1D = {2, 4, 7, 8, 21, 5, 12, 65, 41, 9};
    label = "testLabel";

    int argc = 1;
    char *argv[] = {(char *)"AppName"};
    if (QApplication::instance() == nullptr)
    {
      new QApplication(argc, argv);
    }
  }

  void tearDown() override {}

  void AddOnce_Test_Success()
  {
    //use QmitkChartWidget without QWebEngineView
    QmitkChartWidget widget(nullptr, true);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Adding data caused an exception", widget.AddData1D(data1D, label));

    std::vector<std::unique_ptr<QmitkChartxyData>> *dataVector = widget.GetData();

    CPPUNIT_ASSERT_MESSAGE("Adding data failed.", dataVector->size() == 1 && dataVector != nullptr);
    QmitkChartxyData *dataPtr = dataVector->at(0).get();

    CPPUNIT_ASSERT_MESSAGE("Label differs for no obvious reason",
                           dataPtr->GetLabel().toString().toStdString() == label);

    std::vector<QVariant> insertedYData = dataPtr->GetYData().toVector().toStdVector();
    CPPUNIT_ASSERT_MESSAGE("Data differs in size", insertedYData.size() == data1D.size());

    for (size_t i = 0; i < data1D.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D[i] == insertedYData[i]);
    }
  }

  void AddSameLabelTwice_Test_Success()
  {


    QmitkChartWidget widget(nullptr, true);
    widget.AddData1D(data1D, label);
    widget.AddData1D(data1D, label);

    auto dataVector = widget.GetData();

    QmitkChartxyData* xyData1 = dataVector->at(0).get();
    QmitkChartxyData* xyData2 = dataVector->at(1).get();

    QVariant dataLabel1 = xyData1->GetLabel();
    QVariant dataLabel2 = xyData2->GetLabel();
    CPPUNIT_ASSERT_MESSAGE("The two dataLabel are the same", dataLabel1 != dataLabel2);

    QString string1 = dataLabel1.toString();
    QString string2 = dataLabel2.toString();

    std::string stdString1 = string1.toStdString();
    std::string stdString2 = string2.toStdString();
    CPPUNIT_ASSERT_MESSAGE("The first dataLabel isn't still the same", stdString1 == label);
    CPPUNIT_ASSERT_MESSAGE("The second dataLabel is still the same", stdString2 != label);
  }

  void AddAndRemoveData_Sucess()
  {
    std::vector<double> data1D2 = {2, 2};
    std::string label2 = "testData2";
    std::vector<double> data1D3 = {3, 3, 3};
    std::string label3 = "testData3";
    QmitkChartWidget widget(nullptr, true);
    widget.AddData1D(data1D, label);
	widget.AddData1D(data1D2, label2);
    widget.AddData1D(data1D3, label3);
	auto dataVector = widget.GetData();
    // data with {data1D0, data1D2, data1D3}
	CPPUNIT_ASSERT_MESSAGE("Adding data failed.", dataVector->size() == 3 && dataVector != nullptr);
	widget.RemoveData(label2);
  // data with {data1D0, data1D3}
	CPPUNIT_ASSERT_MESSAGE("Removing data failed.", dataVector->size() == 2 && dataVector != nullptr);
	    QmitkChartxyData *xyData1 = dataVector->at(0).get();
    std::vector<QVariant> insertedYData = xyData1->GetYData().toVector().toStdVector();
      for (size_t i = 0; i < data1D.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D[i] == insertedYData[i]);
    }
    QmitkChartxyData *xyData2 = dataVector->at(1).get();
    insertedYData = xyData2->GetYData().toVector().toStdVector();
    for (size_t i = 0; i < data1D3.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D3[i] == insertedYData[i]);
    }
    widget.RemoveData(label);
    // data with {data1D3}
    CPPUNIT_ASSERT_MESSAGE("Removing data failed.", dataVector->size() == 1 && dataVector != nullptr);
    widget.AddData1D(data1D2, label2);
    CPPUNIT_ASSERT_MESSAGE("Adding data failed.", dataVector->size() == 2 && dataVector != nullptr);
	
	//data with {data1D3, data1D2}
    xyData1 = dataVector->at(0).get();
    insertedYData = xyData1->GetYData().toVector().toStdVector();
    for (size_t i = 0; i < data1D3.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D3[i] == insertedYData[i]);
    }
    xyData2 = dataVector->at(1).get();
    insertedYData = xyData2->GetYData().toVector().toStdVector();
    for (size_t i = 0; i < data1D2.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D2[i] == insertedYData[i]);
    }

  }

  void RemoveNonexistingData_Failure()
  {
    QmitkChartWidget widget(nullptr, true);

    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Removin nonexistend label did not throw exception", widget.RemoveData(label), std::invalid_argument);
  }

  void RemoveData_Success()
  {
    QmitkChartWidget widget(nullptr, true);
    widget.AddData1D(data1D, label);

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Removin nonexistend label did not throw exception", widget.RemoveData(label));
  }

  void SetandGet_Success()
  {
    QmitkChartWidget widget(nullptr, true);
    widget.AddData1D(data1D, label);
    std::string colorName = "green";

    auto mitkChartxyData = widget.GetData();
    QmitkChartxyData *xyData1 = mitkChartxyData->at(0).get();
    
	
	//set color test
	widget.SetColor(label, colorName);
	auto qVariantColor = xyData1->GetColor();
	QString qStringColor = qVariantColor.toString();
	CPPUNIT_ASSERT_MESSAGE("The color isn't the assigned color", colorName == qStringColor.toStdString());

	//set line style test
    QVariant defaultLineStyle = xyData1->GetLineStyle();

    widget.SetLineStyle(label, QmitkChartWidget::LineStyle::dashed);
	QVariant lineStyle = xyData1->GetLineStyle();

	QVariant defaultChartType = xyData1->GetChartType();
    auto line= std::make_pair("line", QmitkChartWidget::ChartType::line);
  if (defaultChartType.toString().toStdString() != line.first)
	{
		CPPUNIT_ASSERT_MESSAGE("The line style could be changed without ChartType line", 
			defaultLineStyle == lineStyle);
	}

	//set ChartType
  widget.SetChartType(label, QmitkChartWidget::ChartType::line);
  QVariant chartType = xyData1->GetChartType();


  CPPUNIT_ASSERT_MESSAGE("The chart type could not be changed to line", 
	  chartType.toString().toStdString() == line.first);

  //set line style with chart type line
  widget.SetLineStyle(label, QmitkChartWidget::LineStyle::dashed);
  lineStyle = xyData1->GetLineStyle();
  CPPUNIT_ASSERT_MESSAGE("The line style could not be changed", "dashed" == lineStyle.toString().toStdString());

  
  }

  void SetC3DataAndGet_Success()
   {
    QmitkChartWidget widget(nullptr, true);
    widget.AddData1D(data1D, label);

    //set YAxisScale
    widget.SetYAxisScale(QmitkChartWidget::AxisScale::log);
    QmitkChartData *c3Data = widget.GetC3Data();
    QVariant yAxisScale = c3Data->GetYAxisScale();
    CPPUNIT_ASSERT_MESSAGE("The YAxisScale could not be changed", "log" == yAxisScale.toString().toStdString());

    //set Title
    std::string testTitle = "testTitle";
    widget.SetTitle(testTitle);
    QVariant title = c3Data->GetTitle();
    CPPUNIT_ASSERT_MESSAGE("The title could not be set", testTitle == title.toString().toStdString());

    //set LegendPosition
    widget.SetLegendPosition(QmitkChartWidget::LegendPosition::right);
    QVariant legendPosition = c3Data->GetLegendPosition();
    CPPUNIT_ASSERT_MESSAGE("The LegendPosition could not be changed", "right" == legendPosition.toString().toStdString());

    //show legend
    QVariant isShowLegend = c3Data->GetShowLegend();
    widget.SetShowLegend(!isShowLegend.toBool());
    QVariant isShowLegendInverted = c3Data->GetShowLegend();
    CPPUNIT_ASSERT_MESSAGE("The ShowLegend could not be changed",
                           isShowLegend.toBool() != isShowLegendInverted.toBool());

    //show dataPoints
	QVariant dataPointSize = c3Data->GetDataPointSize();
    bool showDataPoints = dataPointSize.toInt() > 0;
	widget.SetShowDataPoints(!showDataPoints);
    dataPointSize = c3Data->GetDataPointSize();
	bool showDataPointsInvert = dataPointSize.toInt() > 0;
    CPPUNIT_ASSERT_MESSAGE("The DataPoints could not be changed",
                         showDataPoints != showDataPointsInvert);
   }
};

MITK_TEST_SUITE_REGISTRATION(mitkChartWidget)