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



class mitkChartWidgetTestSuite : public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkChartWidgetTestSuite);

	// Test the dicom property parsing
	MITK_TEST(AddOnce_Test_Success);
	MITK_TEST(RemoveNonexistingData_Success);
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
		
		//TODO Check internals
	}

	void RemoveNonexistingData_Success()
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