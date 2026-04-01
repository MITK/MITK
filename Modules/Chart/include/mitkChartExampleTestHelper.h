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
    /**
     * \brief Test helper class for QmitkChartWidget unit tests.
     *
     * Provides predefined data sets and utility functions to facilitate
     * unit testing of the QmitkChartWidget charting framework. Each
     * data set is a simple linear sequence of (x, y) pairs with
     * preconfigured chart type, color, and line style.
     *
     * \sa QmitkChartWidget
     * \sa QmitkChartxyData
     */
    class MITKCHART_EXPORT ChartExampleTestHelper
    {

    public:
        /// \name Predefined Test Data Sets
        /// @{

        /**
         * \brief Create test data set #1 (x=0..9, y=0..9, bar, red, solid).
         * \return A unique pointer to the populated QmitkChartxyData.
         */
        std::unique_ptr<QmitkChartxyData> GetDataOne();

        /**
         * \brief Create test data set #2 (x=10..19, y=10..19, bar, green, solid).
         * \return A unique pointer to the populated QmitkChartxyData.
         */
        std::unique_ptr<QmitkChartxyData> GetDataTwo();

        /**
         * \brief Create test data set #3 (x=20..29, y=20..29, bar, blue, solid).
         * \return A unique pointer to the populated QmitkChartxyData.
         */
        std::unique_ptr<QmitkChartxyData> GetDataThree();

        /**
         * \brief Create test data set #4 (x=30..39, y=30..39, bar, yellow, solid).
         * \return A unique pointer to the populated QmitkChartxyData.
         */
        std::unique_ptr<QmitkChartxyData> GetDataFour();

        /**
         * \brief Create test data set #5 (x=40..49, y=40..49, bar, black, solid).
         * \return A unique pointer to the populated QmitkChartxyData.
         */
        std::unique_ptr<QmitkChartxyData> GetDataFive();

        /// @}

        /** \brief The QmitkChartWidget instance used for testing. */
        QmitkChartWidget qmitkChartWidget;

        /// \name Utility Functions
        /// @{

        /**
         * \brief Convert a chart type string to the corresponding enum value.
         *
         * Supported strings: "bar", "line", "spline", "pie", "area",
         * "area_spline", "scatter". Unknown strings default to ChartType::bar.
         *
         * \param[in] chartTypeString The chart type as a string.
         * \return The corresponding QmitkChartWidget::ChartType enum value.
         */
        QmitkChartWidget::ChartType ReturnChartTypeByString(std::string chartTypeString);

        /**
         * \brief Convert a color name string to the corresponding enum value.
         *
         * Supported strings include "red", "orange", "yellow", "green", "blue",
         * "purple", "brown", "magenta", etc. Unknown strings default to ChartColor::red.
         *
         * \param[in] chartColorString The color name as a string.
         * \return The corresponding QmitkChartWidget::ChartColor enum value.
         */
        QmitkChartWidget::ChartColor ReturnChartColorByString(std::string chartColorString);

        /**
         * \brief Convert a line style string to the corresponding enum value.
         *
         * Supported strings: "solid", "dashed". Unknown strings default to LineStyle::solid.
         *
         * \param[in] chartStyleString The line style as a string.
         * \return The corresponding QmitkChartWidget::LineStyle enum value.
         */
        QmitkChartWidget::LineStyle ReturnChartStyleByString(std::string chartStyleString);

        /**
         * \brief Add a predefined data set to the chart widget.
         *
         * Retrieves the data set corresponding to the given number (1-5),
         * extracts its properties, and adds it to qmitkChartWidget.
         *
         * \param[in] dataSet The data set number (1 through 5).
         */
        void Add(int dataSet);

        /**
         * \brief Convert QVariantLists of x and y data to a vector of std::pair.
         *
         * \param[in] xData The x values as a QVariantList.
         * \param[in] yData The y values as a QVariantList.
         * \return A vector of (x, y) pairs with double values.
         * \pre xData and yData must have the same length.
         */
        std::vector<std::pair<double, double> > ToStdPairList(QVariantList xData, QVariantList yData);

        /**
         * \brief Clear all chart data and reset the widget.
         */
        void ClearMemory();

        /// @}

    }; // end ChartExampleTestHelper
} // end mitk

#endif
