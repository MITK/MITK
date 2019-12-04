/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// qt
#include <QApplication>

// mitk chart
#include <mitkChartExampleTestHelper.h>

// mitk core
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkPropertyNameHelper.h>

// std includes
#include <string>

class mitkChartExampleTestSuite : public mitk::TestFixture
{
    QApplication a();
    CPPUNIT_TEST_SUITE(mitkChartExampleTestSuite);
    MITK_TEST(AddingDataTest);
    MITK_TEST(CheckingDataTest);
    MITK_TEST(ClearingDataTest);
    CPPUNIT_TEST_SUITE_END();

private:

    mitk::ChartExampleTestHelper* helper;

public:

    void setUp() override
    {
        helper = new mitk::ChartExampleTestHelper;
    }

    void tearDown() override
    {
        delete helper;
    }

    void AddingDataTest()
    {
        MITK_INFO << "=== AddingDataTest start ===";
        AddNewData();
        CheckData();
        ClearData();
        MITK_INFO << "=== AddingDataTest end ===";
    }

    void CheckingDataTest()
    {
        MITK_INFO << "=== CheckingDataTest start ===";
        CheckData();
        MITK_INFO << "=== CheckingDataTest end ===";
    }

    void ClearingDataTest()
    {
        MITK_INFO << "=== ClearingDataTest start ===";
        ClearData();
        MITK_INFO << "=== ClearingDataTest end ===";
    }


    void AddNewData()
    {
        MITK_INFO << "=== AddNewData";

        // Adding data
        //size_t size;
        for (size_t i = 1; i < 6; i++)
        {
            //helper.Add(i);
            //size = helper.qmitkChartWidget.ReturnSizeOfMemory();
            //CPPUNIT_ASSERT_MESSAGE("Data storage does not contain the right amount of items!", size == i);
        }
    }

    void CheckData()
    {
        auto myDataOne = helper->GetDataOne();

        auto xDataOne = myDataOne->GetXData();
        auto yDataOne = myDataOne->GetYData();
        auto labelOne = myDataOne->GetLabel();
        auto typeOne = myDataOne->GetChartType();
        auto colorOne = myDataOne->GetColor();
        auto styleOne = myDataOne->GetLineStyle();

        auto dataToCheckOne = helper->qmitkChartWidget.GetDataElementByLabel(labelOne.toString().toStdString());

        CPPUNIT_ASSERT_MESSAGE("Dataset one was not saved correctly", dataToCheckOne->GetXData() == xDataOne &&
                                                                      dataToCheckOne->GetYData() == yDataOne &&
                                                                      dataToCheckOne->GetLabel() == labelOne &&
                                                                      dataToCheckOne->GetChartType() == typeOne &&
                                                                      dataToCheckOne->GetColor() == colorOne &&
                                                                      dataToCheckOne->GetLineStyle() == styleOne);

        auto myDataTwo = helper->GetDataTwo();

        auto xDataTwo = myDataTwo->GetXData();
        auto yDataTwo = myDataTwo->GetYData();
        auto labelTwo = myDataTwo->GetLabel();
        auto typeTwo = myDataTwo->GetChartType();
        auto colorTwo = myDataTwo->GetColor();
        auto styleTwo = myDataTwo->GetLineStyle();

        auto dataToCheckTwo = helper->qmitkChartWidget.GetDataElementByLabel(labelTwo.toString().toStdString());

        CPPUNIT_ASSERT_MESSAGE("Dataset two was not saved correctly", dataToCheckTwo->GetXData() == xDataTwo &&
                                                                      dataToCheckTwo->GetYData() == yDataTwo &&
                                                                      dataToCheckTwo->GetLabel() == labelTwo &&
                                                                      dataToCheckTwo->GetChartType() == typeTwo &&
                                                                      dataToCheckTwo->GetColor() == colorTwo &&
                                                                      dataToCheckTwo->GetLineStyle() == styleTwo);

        auto myDataThree = helper->GetDataThree();

        auto xDataThree = myDataThree->GetXData();
        auto yDataThree = myDataThree->GetYData();
        auto labelThree = myDataThree->GetLabel();
        auto typeThree = myDataThree->GetChartType();
        auto colorThree = myDataThree->GetColor();
        auto styleThree = myDataThree->GetLineStyle();

        auto dataToCheckThree = helper->qmitkChartWidget.GetDataElementByLabel(labelThree.toString().toStdString());

        CPPUNIT_ASSERT_MESSAGE("Dataset three was not saved correctly", dataToCheckThree->GetXData() == xDataThree &&
                                                                        dataToCheckThree->GetYData() == yDataThree &&
                                                                        dataToCheckThree->GetLabel() == labelThree &&
                                                                        dataToCheckThree->GetChartType() == typeThree &&
                                                                        dataToCheckThree->GetColor() == colorThree &&
                                                                        dataToCheckThree->GetLineStyle() == styleThree);

        auto myDataFour = helper->GetDataFour();

        auto xDataFour = myDataFour->GetXData();
        auto yDataFour = myDataFour->GetYData();
        auto labelFour = myDataFour->GetLabel();
        auto typeFour = myDataFour->GetChartType();
        auto colorFour = myDataFour->GetColor();
        auto styleFour = myDataFour->GetLineStyle();

        auto dataToCheckFour = helper->qmitkChartWidget.GetDataElementByLabel(labelFour.toString().toStdString());

        CPPUNIT_ASSERT_MESSAGE("Dataset one was not saved correctly", dataToCheckFour->GetXData() == xDataFour &&
                                                                      dataToCheckFour->GetYData() == yDataFour &&
                                                                      dataToCheckFour->GetLabel() == labelFour &&
                                                                      dataToCheckFour->GetChartType() == typeFour &&
                                                                      dataToCheckFour->GetColor() == colorFour &&
                                                                      dataToCheckFour->GetLineStyle() == styleFour);

        auto myDataFive = helper->GetDataFive();

        auto xDataFive = myDataFive->GetXData();
        auto yDataFive = myDataFive->GetYData();
        auto labelFive = myDataFive->GetLabel();
        auto typeFive = myDataFive->GetChartType();
        auto colorFive = myDataFive->GetColor();
        auto styleFive = myDataFive->GetLineStyle();

        auto dataToCheckFive = helper->qmitkChartWidget.GetDataElementByLabel(labelFive.toString().toStdString());

        CPPUNIT_ASSERT_MESSAGE("Dataset one was not saved correctly", dataToCheckFive->GetXData() == xDataFive &&
                                                                      dataToCheckFive->GetYData() == yDataFive &&
                                                                      dataToCheckFive->GetLabel() == labelFive &&
                                                                      dataToCheckFive->GetChartType() == typeFive &&
                                                                      dataToCheckFive->GetColor() == colorFive &&
                                                                      dataToCheckFive->GetLineStyle() == styleFive);
    }

    void ClearData()
    {
        MITK_INFO << "=== ClearData";

        // Claering data
        mitk::ChartExampleTestHelper helper;
        helper.qmitkChartWidget.Clear();
        //int size = helper.qmitkChartWidget.ReturnSizeOfMemory();
        //CPPUNIT_ASSERT_MESSAGE("Data storage was not cleared completly!", size == 0);
    }
};

MITK_TEST_SUITE_REGISTRATION(mitkChartExample)
