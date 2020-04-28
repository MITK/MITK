/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageStatisticsContainerNodeHelper.h"
#include "mitkImageStatisticsContainerManager.h"
#include "mitkImageStatisticsContainer.h"

class mitkImageStatisticsContainerTestSuite : public mitk::TestFixture
{
    CPPUNIT_TEST_SUITE(mitkImageStatisticsContainerTestSuite);
    MITK_TEST(StatisticNamesIO);
    MITK_TEST(InvalidKey);
    MITK_TEST(TimeSteps);
    MITK_TEST(PrintSelf);
    MITK_TEST(InternalClone);
    MITK_TEST(StatisticNames);
    MITK_TEST(OverwriteStatistic);
    MITK_TEST(AllStatisticNamesForContainer);
    MITK_TEST(Reset);
    CPPUNIT_TEST_SUITE_END();

private:
    mitk::ImageStatisticsContainer::Pointer m_StatisticsContainer;
    mitk::ImageStatisticsContainer::Pointer m_StatisticsContainer2;
    mitk::ImageStatisticsContainer::Pointer m_StatisticsContainer3;

    mitk::ImageStatisticsContainer::ImageStatisticsObject m_StatisticsObject;
    mitk::ImageStatisticsContainer::ImageStatisticsObject m_StatisticsObject2;
    mitk::ImageStatisticsContainer::ImageStatisticsObject m_StatisticsObject3;

    mitk::ArbitraryTimeGeometry::Pointer m_TimeGeometry;

    mitk::Geometry3D::Pointer m_Geometry1;
    mitk::Geometry3D::Pointer m_Geometry2;
    mitk::Geometry3D::Pointer m_Geometry3;
    mitk::Geometry3D::Pointer m_Geometry3_5;
    mitk::Geometry3D::Pointer m_Geometry4;
    mitk::Geometry3D::Pointer m_Geometry5;

    mitk::TimePointType m_Geometry1MinTP;
    mitk::TimePointType m_Geometry2MinTP;
    mitk::TimePointType m_Geometry3MinTP;
    mitk::TimePointType m_Geometry3_5MinTP;
    mitk::TimePointType m_Geometry4MinTP;
    mitk::TimePointType m_Geometry5MinTP;

    mitk::TimePointType m_Geometry1MaxTP;
    mitk::TimePointType m_Geometry2MaxTP;
    mitk::TimePointType m_Geometry3MaxTP;
    mitk::TimePointType m_Geometry3_5MaxTP;
    mitk::TimePointType m_Geometry4MaxTP;
    mitk::TimePointType m_Geometry5MaxTP;

    std::vector<std::string> estimatedDefaultStatisticNames
    {
        "Mean",
        "Median",
        "StandardDeviation",
        "RMS",
        "Max",
        "MaxPosition",
        "Min",
        "MinPosition",
        "#Voxel",
        "Volume [mm^3]",
        "Skewness",
        "Kurtosis",
        "Uniformity",
        "Entropy",
        "MPP",
        "UPP"
    };

public:
    void setUp() override
    {
        m_StatisticsContainer = mitk::ImageStatisticsContainer::New();
        m_StatisticsContainer2 = mitk::ImageStatisticsContainer::New();
        m_StatisticsContainer3 = mitk::ImageStatisticsContainer::New();

        m_Geometry1 = mitk::Geometry3D::New();
        m_Geometry2 = mitk::Geometry3D::New();
        m_Geometry3 = mitk::Geometry3D::New();
        m_Geometry3_5 = mitk::Geometry3D::New();
        m_Geometry4 = mitk::Geometry3D::New();
        m_Geometry5 = mitk::Geometry3D::New();

        m_Geometry1MinTP = 1;
        m_Geometry2MinTP = 2;
        m_Geometry3MinTP = 3;
        m_Geometry3_5MinTP = 3.5;
        m_Geometry4MinTP = 4;
        m_Geometry5MinTP = 5;

        m_Geometry1MaxTP = 1.9;
        m_Geometry2MaxTP = 2.9;
        m_Geometry3MaxTP = 3.9;
        m_Geometry3_5MaxTP = 3.9;
        m_Geometry4MaxTP = 4.9;
        m_Geometry5MaxTP = 5.9;

        m_TimeGeometry = mitk::ArbitraryTimeGeometry::New();
        m_TimeGeometry->ClearAllGeometries();
        m_TimeGeometry->AppendNewTimeStep(m_Geometry1, m_Geometry1MinTP, m_Geometry1MaxTP);
        m_TimeGeometry->AppendNewTimeStep(m_Geometry2, m_Geometry2MinTP, m_Geometry2MaxTP);
        m_TimeGeometry->AppendNewTimeStep(m_Geometry3, m_Geometry3MinTP, m_Geometry3MaxTP);
        m_TimeGeometry->AppendNewTimeStep(m_Geometry4, m_Geometry4MinTP, m_Geometry4MaxTP);
        m_TimeGeometry->AppendNewTimeStep(m_Geometry5, m_Geometry5MinTP, m_Geometry5MaxTP);
    }

    void tearDown() override
    {
        m_StatisticsContainer->Reset();
        m_StatisticsContainer2->Reset();
        m_StatisticsContainer3->Reset();

        m_StatisticsObject.Reset();
        m_StatisticsObject2.Reset();
        m_StatisticsObject3.Reset();

        m_TimeGeometry->ClearAllGeometries();
    }

    void StatisticNamesIO()
    {
        auto defaultStatisticNames = mitk::ImageStatisticsContainer::ImageStatisticsObject::GetDefaultStatisticNames();
        for (size_t i = 0; i < estimatedDefaultStatisticNames.size(); i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated default statistics names are not correct.", defaultStatisticNames.at(i), estimatedDefaultStatisticNames.at(i));
        }

        std::string testName = "testName";
        double realTypeTest = 3.14;
        m_StatisticsObject.AddStatistic(testName, realTypeTest);
        auto customStatisticNames = m_StatisticsObject.GetCustomStatisticNames();
        size_t one = 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Amount of estimated custom statistics names is not correct.", customStatisticNames.size(), one);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated custom statistics names are not correct.", customStatisticNames.front(), testName);

        auto allStatisticNames = m_StatisticsObject.GetAllStatisticNames();
        estimatedDefaultStatisticNames.push_back(testName);
        for (size_t i = 0; i < estimatedDefaultStatisticNames.size(); i++)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated statistics names are not correct.", allStatisticNames.at(i), estimatedDefaultStatisticNames.at(i));
        }

        auto existingStatisticNames = m_StatisticsObject.GetExistingStatisticNames();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Amount of estimated existing statistics names is not correct.", existingStatisticNames.size(), one);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated existing statistics names are not correct.", existingStatisticNames.front(), testName);

        m_StatisticsObject.Reset();
        CPPUNIT_ASSERT_MESSAGE("Custom statistics names were not removed correctly.", m_StatisticsObject.GetCustomStatisticNames().empty());
    }

    void InvalidKey()
    {
        CPPUNIT_ASSERT_THROW_MESSAGE("Exception should have been thrown because key does not exists.", m_StatisticsObject.GetValueNonConverted("Test"), mitk::Exception);
    }

    void TimeSteps()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Previous added time step was not saved correctly.", m_StatisticsContainer->TimeStepExists(0), true);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Previous added time step was not saved correctly.", m_StatisticsContainer->TimeStepExists(1), true);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Previous added time step was not saved correctly.", m_StatisticsContainer->TimeStepExists(2), true);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Previous added time step was not saved correctly.", m_StatisticsContainer->TimeStepExists(3), true);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Previous added time step was not saved correctly.", m_StatisticsContainer->TimeStepExists(4), true);

        CPPUNIT_ASSERT_THROW_MESSAGE("Out of timeStep geometry bounds timeStep was added but no exception was thrown.", m_StatisticsContainer->SetStatisticsForTimeStep(42, m_StatisticsObject), mitk::Exception);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistics container does not contain the right amount of timeSteps.", static_cast<int> (m_StatisticsContainer->GetNumberOfTimeSteps()), 5);

        CPPUNIT_ASSERT_THROW_MESSAGE("A statistic for a non existing time steps was found.", m_StatisticsContainer->GetStatisticsForTimeStep(42), mitk::Exception);
    }

    void PrintSelf()
    {
        std::stringstream print;

        // Check print function for empty container
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("Print function throws an exception.", m_StatisticsContainer->Print(print));

        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);

        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        // Check print function for filled container
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("Print function throws an exception.", m_StatisticsContainer->Print(print));
    }

    void InternalClone()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);

        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        auto clone = m_StatisticsContainer->Clone();

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", m_StatisticsContainer->GetNumberOfTimeSteps(), clone->GetNumberOfTimeSteps());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->TimeStepExists(0), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->TimeStepExists(1), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->TimeStepExists(2), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->TimeStepExists(3), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->TimeStepExists(4), true);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->GetStatisticsForTimeStep(0).HasStatistic("Test"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->GetStatisticsForTimeStep(1).HasStatistic("Test"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->GetStatisticsForTimeStep(2).HasStatistic("Test"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->GetStatisticsForTimeStep(3).HasStatistic("Test"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Internal clone was not cloned correctly.", clone->GetStatisticsForTimeStep(4).HasStatistic("Test"), true);
    }

    void StatisticNames()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);
        m_StatisticsContainer2->SetTimeGeometry(m_TimeGeometry);
        m_StatisticsContainer3->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);
        m_StatisticsObject2.AddStatistic("Test2", 4.2);
        m_StatisticsObject3.AddStatistic("Test3", 4.2);

        // Setting the same statistics for different time steps will only add the statistics name once.
        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        // Setting the same statistics for different time steps will only add the statistics name once.
        m_StatisticsContainer2->SetStatisticsForTimeStep(0, m_StatisticsObject2);
        m_StatisticsContainer2->SetStatisticsForTimeStep(1, m_StatisticsObject2);
        m_StatisticsContainer2->SetStatisticsForTimeStep(2, m_StatisticsObject2);
        m_StatisticsContainer2->SetStatisticsForTimeStep(3, m_StatisticsObject2);
        m_StatisticsContainer2->SetStatisticsForTimeStep(4, m_StatisticsObject2);

        // Setting the same statistics for different time steps will only add the statistics name once.
        m_StatisticsContainer3->SetStatisticsForTimeStep(0, m_StatisticsObject3);
        m_StatisticsContainer3->SetStatisticsForTimeStep(1, m_StatisticsObject3);
        m_StatisticsContainer3->SetStatisticsForTimeStep(2, m_StatisticsObject3);
        m_StatisticsContainer3->SetStatisticsForTimeStep(3, m_StatisticsObject3);
        m_StatisticsContainer3->SetStatisticsForTimeStep(4, m_StatisticsObject3);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated statistics names are not correct.", mitk::GetAllStatisticNames(m_StatisticsContainer).size(), estimatedDefaultStatisticNames.size() + 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom statistic name was not saved correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(0).HasStatistic("Test"), true);

        std::vector<mitk::ImageStatisticsContainer::ConstPointer> containers;

        containers.push_back(m_StatisticsContainer2.GetPointer());
        containers.push_back(m_StatisticsContainer3.GetPointer());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated statistics names are not correct.", mitk::GetAllStatisticNames(containers).size(), estimatedDefaultStatisticNames.size() + 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom statistic name was not saved correctly.", m_StatisticsContainer2->GetStatisticsForTimeStep(0).HasStatistic("Test2"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom statistic name was not saved correctly.", m_StatisticsContainer3->GetStatisticsForTimeStep(0).HasStatistic("Test3"), true);
    }

    void OverwriteStatistic()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);

        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic was not correctly added to StatisticsObject.", boost::get<double> (m_StatisticsContainer->GetStatisticsForTimeStep(0).GetValueNonConverted("Test")), 4.2);

        // An existing statistic won't be updated by adding another statistic with same name to that object.
        m_StatisticsObject.AddStatistic("Test", 42.0);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic was overwritten.", boost::get<double>(m_StatisticsContainer->GetStatisticsForTimeStep(0).GetValueNonConverted("Test")), 4.2);
    }

    void AllStatisticNamesForContainer()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);
        m_StatisticsObject2.AddStatistic("Test2", 4.2);

        // Setting the same statistics for different time steps will only add the statistics name once.
        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject2);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject2);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        std::vector<mitk::ImageStatisticsContainer::ConstPointer> containers;

        // Check if empty vector triggers exception.
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("Empty vector triggered an exception.", mitk::GetAllStatisticNames(containers));

        containers.push_back(m_StatisticsContainer2.GetPointer());
        containers.push_back(m_StatisticsContainer3.GetPointer());

        // Check if vector with empty container triggers exception.
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("Empty container triggered an exception.", mitk::GetAllStatisticNames(containers));

        // Adding filled container to check if data was saved correctly.
        containers.push_back(m_StatisticsContainer.GetPointer());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Estimated statistics names are not correct.", mitk::GetAllStatisticNames(containers).size(), estimatedDefaultStatisticNames.size() + 2);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom statistic name was not saved correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(0).HasStatistic("Test"), true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Custom statistic name was not saved correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(1).HasStatistic("Test2"), true);
    }

    void Reset()
    {
        m_StatisticsContainer->SetTimeGeometry(m_TimeGeometry);

        m_StatisticsObject.AddStatistic("Test", 4.2);

        m_StatisticsContainer->SetStatisticsForTimeStep(0, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(1, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(2, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(3, m_StatisticsObject);
        m_StatisticsContainer->SetStatisticsForTimeStep(4, m_StatisticsObject);

        m_StatisticsContainer->Reset();

        m_StatisticsObject.Reset();

        m_TimeGeometry->ClearAllGeometries();
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(0).GetExistingStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(0).GetCustomStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(1).GetExistingStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(1).GetCustomStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(2).GetExistingStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(2).GetCustomStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(3).GetExistingStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(3).GetCustomStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(4).GetExistingStatisticNames().size(), size_t());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic object for time step was not deleted correctly.", m_StatisticsContainer->GetStatisticsForTimeStep(4).GetCustomStatisticNames().size(), size_t());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Statistic was not deleted correctly.", m_StatisticsObject.HasStatistic("Test"), false);
        
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Time geometry was not cleared correctly.", m_TimeGeometry->IsValid(), false);
    }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsContainer)
