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
#include <qapplication>

// std includes
#include <string>


// MITK includes
#include "QmitkChartWidget.h"
#include "QmitkChartxyData.h"



class mitkChartWidgetTestSuite : public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkChartWidgetTestSuite);

	// Test the dicom property parsing
  MITK_TEST(AddOnce_Test_Success);
  MITK_TEST(AddSameLabelTwice_Test_Success);
  MITK_TEST(RemoveNonexistingData_Failure);
  MITK_TEST(RemoveData_Success);
  CPPUNIT_TEST_SUITE_END();

private:

	std::vector<double> data1D;
	std::string label;

public:

	void setUp() override
	{
		data1D = { 2, 4, 7, 8, 21, 5, 12, 65, 41, 9 };
		label = "testLabel";

		int argc = 1;
		char* argv[] = { "AppName" };
		if (QApplication::instance() == nullptr)
		{
			new QApplication(argc, argv);
		}
	}

	void tearDown() override
	{
	}

	void AddOnce_Test_Success()
	{
    QmitkChartWidget widget;
		CPPUNIT_ASSERT_NO_THROW_MESSAGE("Adding data caused an exception", widget.AddData1D(data1D, label));

    std::vector<QmitkChartxyData*> dataVector = widget.GetData();
    CPPUNIT_ASSERT_MESSAGE("Adding data failed.", dataVector.size() == 1 && dataVector[0] != nullptr);
    QmitkChartxyData* dataPtr = dataVector[0];

    CPPUNIT_ASSERT_MESSAGE("Label differs for no obvious reason", dataPtr->GetLabel().toString().toStdString() == label);

    std::vector<QVariant> insertedYData = dataPtr->GetYData().toVector().toStdVector();
    CPPUNIT_ASSERT_MESSAGE("Data differs in size", insertedYData.size() == data1D.size());

    for (int i = 0; i < data1D.size(); ++i)
    {
      CPPUNIT_ASSERT_MESSAGE("The inserted data differs when checked", data1D[i] == insertedYData[i]);
    }
	}

  void AddSameLabelTwice_Test_Success()
  {
    QmitkChartWidget widget;
    widget.AddData1D(data1D, label);
    widget.AddData1D(data1D, label);

    std:: vector<QmitkChartxyData*>dataVector = widget.GetData();

    QmitkChartxyData* xyData1 = dataVector.at(0);
    QmitkChartxyData* xyData2 = dataVector.at(1);

    QVariant dataLabel1 = xyData1 -> GetLabel();
    QVariant dataLabel2 = xyData2->GetLabel();
    CPPUNIT_ASSERT_MESSAGE("The two dataLabel are the same", dataLabel1 != dataLabel2);

    QString string1 = dataLabel1.toString();
    QString string2 = dataLabel2.toString();

    std::string stdString1 = string1.toStdString();
    std::string stdString2 = string2.toStdString();
    CPPUNIT_ASSERT_MESSAGE("The first dataLabel isn't still the same", stdString1 == label);
    CPPUNIT_ASSERT_MESSAGE("The second dataLabel is still the same", stdString2 != label);
  }

  void RemoveNonexistingData_Failure()
	{
		QmitkChartWidget widget;

		CPPUNIT_ASSERT_THROW_MESSAGE("Removin nonexistend label did not throw exception", widget.RemoveData(label), std::invalid_argument);
	}

	void RemoveData_Success()
	{
		QmitkChartWidget widget;
		widget.AddData1D(data1D, label);

		CPPUNIT_ASSERT_NO_THROW_MESSAGE("Removin nonexistend label did not throw exception", widget.RemoveData(label));
	}
};

MITK_TEST_SUITE_REGISTRATION(mitkChartWidget)