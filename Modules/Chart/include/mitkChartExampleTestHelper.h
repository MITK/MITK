/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkChartExampleTestHelper_h
#define mitkChartExampleTestHelper_h

#include <MitkChartExports.h>

// mitk core
#include <QmitkChartxyData.h>
#include <QmitkChartWidget.h>

// std includes
#include <map>
#include <memory>
#include <vector>
#include <string>

namespace mitk
{
    class MITKCHART_EXPORT ChartExampleTestHelper
    {

    public:
        //////////////////////////////////////////////////////////////////////////
        // ACCESS TO DATA SETS
        //////////////////////////////////////////////////////////////////////////

        std::unique_ptr<QmitkChartxyData> GetDataOne();
        std::unique_ptr<QmitkChartxyData> GetDataTwo();
        std::unique_ptr<QmitkChartxyData> GetDataThree();
        std::unique_ptr<QmitkChartxyData> GetDataFour();
        std::unique_ptr<QmitkChartxyData> GetDataFive();

        //////////////////////////////////////////////////////////////////////////
        // INSTANCE OF CLASS TO BE TESTED
        //////////////////////////////////////////////////////////////////////////

        QmitkChartWidget qmitkChartWidget;

        //////////////////////////////////////////////////////////////////////////
        // AUXILIARY FUNCTIONS
        //////////////////////////////////////////////////////////////////////////

        QmitkChartWidget::ChartType ReturnChartTypeByString(std::string chartTypeString);
        QmitkChartWidget::ChartColor ReturnChartColorByString(std::string chartColorString);
        QmitkChartWidget::LineStyle ReturnChartStyleByString(std::string chartStyleString);

        void Add(int dataSet);
        std::vector<std::pair<double, double> > ToStdPairList(QVariantList xData, QVariantList yData);
        void ClearMemory();

    }; // end ChartExampleTestHelper
} // end mitk

#endif
