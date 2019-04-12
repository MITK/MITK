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

#include <regex>

#include <QGridLayout>
#include <QWebChannel>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include <QmitkChartWidget.h>

#include "mitkExceptionMacro.h"
#include <QmitkChartData.h>
#include <QmitkChartxyData.h>

class CustomPage : public QWebEnginePage
{
public:
  CustomPage(QObject *parent = 0) : QWebEnginePage(parent) {}
  virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel /*level*/,
                                        const QString &message,
                                        int lineNumber,
                                        const QString & /*sourceID*/)
  {
    MITK_INFO << "JS > " << lineNumber << ": " << message.toStdString();
  }
};

class QmitkChartWidget::Impl final
{
public:
  explicit Impl(QWidget *parent);
  ~Impl();

  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;

  void AddData1D(const std::vector<double> &data1D, const std::string &label, QmitkChartWidget::ChartType chartType);
  void AddData2D(const std::map<double, double> &data2D,
                 const std::string &label,
                 QmitkChartWidget::ChartType chartType);

  void RemoveData(const std::string &label);

  void ClearData();

  void SetColor(const std::string &label, const std::string &colorName);
  void SetLineStyle(const std::string &label, LineStyle style);

  void SetYAxisScale(AxisScale scale);

  void SetXAxisLabel(const std::string &label);

  void SetYAxisLabel(const std::string &label);

  void SetTitle(const std::string &title);

  void SetXErrorBars(const std::string &label,
                     const std::vector<double> &errorPlus,
                     const std::vector<double> &errorMinus = std::vector<double>());
  void SetYErrorBars(const std::string &label,
                     const std::vector<double> &errorPlus,
                     const std::vector<double> &errorMinus = std::vector<double>());
  std::string GetThemeName() const;
  void SetThemeName(ColorTheme style);

  void SetChartType(QmitkChartWidget::ChartType chartType);
  void SetLegendPosition(LegendPosition position);
  void SetChartTypeByLabel(const std::string &label, QmitkChartWidget::ChartType chartType);

  void Show(bool showSubChart);

  void SetShowLegend(bool show);
  void SetShowErrorBars(bool show);

  void SetStackedData(bool stacked);

  void SetShowDataPoints(bool showDataPoints = false);

  void SetChartType(const std::string &label, QmitkChartWidget::ChartType chartType);
  QList<QVariant> ConvertErrorVectorToQList(const std::vector<double> &error);

  std::string ConvertChartTypeToString(QmitkChartWidget::ChartType chartType) const;

  void ClearJavaScriptChart();
  void InitializeJavaScriptChart();
  void CallJavaScriptFuntion(const QString &command);

  QSize sizeHint() const;

private:
  using ChartxyDataVector = std::vector<std::unique_ptr<QmitkChartxyData>>;
  std::string GetUniqueLabelName(const QList<QVariant> &labelList, const std::string &label) const;
  QmitkChartxyData *GetDataElementByLabel(const std::string &label) const;
  QList<QVariant> GetDataLabels(const ChartxyDataVector &c3xyData) const;

  QWebChannel *m_WebChannel;
  QWebEngineView *m_WebEngineView;

  QmitkChartData m_C3Data;
  ChartxyDataVector m_C3xyData;
  std::map<QmitkChartWidget::ChartType, std::string> m_ChartTypeToName;
  std::map<QmitkChartWidget::ColorTheme, std::string> m_ColorThemeToName;
  std::map<QmitkChartWidget::LegendPosition, std::string> m_LegendPositionToName;
  std::map<QmitkChartWidget::LineStyle, std::string> m_LineStyleToName;
  std::map<QmitkChartWidget::AxisScale, std::string> m_AxisScaleToName;
};

std::string QmitkChartWidget::Impl::GetThemeName() const {
  return m_C3Data.GetThemeName().toString().toStdString();
}

QmitkChartWidget::Impl::Impl(QWidget *parent)
  : m_WebChannel(new QWebChannel(parent)), m_WebEngineView(new QWebEngineView(parent))
{
  // disable context menu for QWebEngineView
  m_WebEngineView->setContextMenuPolicy(Qt::NoContextMenu);

  m_WebEngineView->setPage(new CustomPage());

  // Set the webengineview to an initial empty page. The actual chart will be loaded once the data is calculated.

  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///Chart/empty.html")));
  m_WebEngineView->page()->setWebChannel(m_WebChannel);

  m_WebEngineView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

  connect(m_WebEngineView, SIGNAL(loadFinished(bool)), parent, SLOT(OnLoadFinished(bool)));
  auto layout = new QGridLayout(parent);
  layout->setMargin(0);
  layout->addWidget(m_WebEngineView);
  m_ChartTypeToName.emplace(ChartType::bar, "bar");
  m_ChartTypeToName.emplace(ChartType::line, "line");
  m_ChartTypeToName.emplace(ChartType::spline, "spline");
  m_ChartTypeToName.emplace(ChartType::pie, "pie");
  m_ChartTypeToName.emplace(ChartType::area, "area");
  m_ChartTypeToName.emplace(ChartType::area_spline, "area-spline");
  m_ChartTypeToName.emplace(ChartType::scatter, "scatter");

  m_LegendPositionToName.emplace(LegendPosition::bottomMiddle, "bottomMiddle");
  m_LegendPositionToName.emplace(LegendPosition::bottomRight, "bottomRight");
  m_LegendPositionToName.emplace(LegendPosition::topRight, "topRight");
  m_LegendPositionToName.emplace(LegendPosition::topLeft, "topLeft");
  m_LegendPositionToName.emplace(LegendPosition::middleRight, "middleRight");

  m_LineStyleToName.emplace(LineStyle::solid, "solid");
  m_LineStyleToName.emplace(LineStyle::dashed, "dashed");

  m_AxisScaleToName.emplace(AxisScale::linear, "");
  m_AxisScaleToName.emplace(AxisScale::log, "log");

  m_ColorThemeToName.emplace(ColorTheme::lightstyle, "light");
  m_ColorThemeToName.emplace(ColorTheme::darkstyle, "dark");
}

QmitkChartWidget::Impl::~Impl() {}

std::string CheckForCorrectHex(const std::string &colorName)
{
  std::regex rgx("([A-Fa-f0-9]{6}|[A-Fa-f0-9]{3})");
  std::smatch match;

  if (!colorName.empty() && colorName.at(0) != '#' && std::regex_search(colorName.begin(), colorName.end(), match, rgx))
  {
    return "#" + colorName;
  }
  else
  {
    return colorName;
  }
}

void QmitkChartWidget::Impl::AddData1D(const std::vector<double> &data1D,
                                       const std::string &label,
                                       QmitkChartWidget::ChartType type)
{
  std::map<double, double> transformedData2D;
  unsigned int count = 0;
  // transform the 1D data to 2D data
  for (const auto &ele : data1D)
  {
    transformedData2D[count] = ele;
    count++;
  }

  AddData2D(transformedData2D, label, type);
}

void QmitkChartWidget::Impl::AddData2D(const std::map<double, double> &data2D,
                                       const std::string &label,
                                       QmitkChartWidget::ChartType type)
{
  QMap<QVariant, QVariant> data2DConverted;
  for (const auto &aValue : data2D)
  {
    data2DConverted.insert(aValue.first, aValue.second);
  }
  const std::string chartTypeName(m_ChartTypeToName.at(type));

  auto definedLabels = GetDataLabels(m_C3xyData);
  auto uniqueLabel = GetUniqueLabelName(definedLabels, label);
  if (type == ChartType::scatter)
  {
    SetShowDataPoints(true);
    MITK_INFO << "Enabling data points for all because of scatter plot";
  }

  m_C3xyData.push_back(std::make_unique<QmitkChartxyData>(
    data2DConverted, QVariant(QString::fromStdString(uniqueLabel)), QVariant(QString::fromStdString(chartTypeName))));
}

void QmitkChartWidget::Impl::RemoveData(const std::string &label)
{
  for (ChartxyDataVector::iterator iter = m_C3xyData.begin(); iter != m_C3xyData.end(); ++iter)
  {
    if ((*iter)->GetLabel().toString().toStdString() == label)
    {
      m_C3xyData.erase(iter);
      return;
    }
  }

  throw std::invalid_argument("Cannot Remove Data because the label does not exist.");
}

void QmitkChartWidget::Impl::ClearData()
{
  for (auto &xyData : m_C3xyData)
  {
    m_WebChannel->deregisterObject(xyData.get());
  }
  m_C3xyData.clear();
}

void QmitkChartWidget::Impl::SetColor(const std::string &label, const std::string &colorName)
{
  auto element = GetDataElementByLabel(label);
  if (element)
  {
    auto colorChecked = CheckForCorrectHex(colorName);
    element->SetColor(QVariant(QString::fromStdString(colorChecked)));
  }
}

void QmitkChartWidget::Impl::SetLineStyle(const std::string &label, LineStyle style)
{
  auto element = GetDataElementByLabel(label);
  const std::string lineStyleName(m_LineStyleToName.at(style));
  element->SetLineStyle(QVariant(QString::fromStdString(lineStyleName)));
}

void QmitkChartWidget::Impl::SetYAxisScale(AxisScale scale)
{
  const std::string axisScaleName(m_AxisScaleToName.at(scale));
  m_C3Data.SetYAxisScale(QString::fromStdString(axisScaleName));
}

QmitkChartxyData *QmitkChartWidget::Impl::GetDataElementByLabel(const std::string &label) const
{
  for (const auto &qmitkChartxyData : m_C3xyData)
  {
    if (qmitkChartxyData->GetLabel().toString() == label.c_str())
    {
      return qmitkChartxyData.get();
    }
  }
  MITK_WARN << "label " << label << " not found in QmitkChartWidget";
  return nullptr;
}

QList<QVariant> QmitkChartWidget::Impl::GetDataLabels(const ChartxyDataVector &c3xyData) const
{
  QList<QVariant> dataLabels;
  for (auto element = c3xyData.begin(); element != c3xyData.end(); ++element)
  {
    dataLabels.push_back((*element)->GetLabel());
  }
  return dataLabels;
}

void QmitkChartWidget::Impl::SetXAxisLabel(const std::string &label)
{
  m_C3Data.SetXAxisLabel(QString::fromStdString(label));
}

void QmitkChartWidget::Impl::SetYAxisLabel(const std::string &label)
{
  m_C3Data.SetYAxisLabel(QString::fromStdString(label));
}

void QmitkChartWidget::Impl::SetTitle(const std::string &title)
{
  m_C3Data.SetTitle(QString::fromStdString(title));
}

void QmitkChartWidget::Impl::SetThemeName(QmitkChartWidget::ColorTheme style)
{
  const std::string themeName(m_ColorThemeToName.at(style));
  m_C3Data.SetThemeName(QString::fromStdString(themeName));
}

void QmitkChartWidget::Impl::SetChartType(QmitkChartWidget::ChartType chartType)
{
  for (auto iterator = m_C3xyData.begin(); iterator != m_C3xyData.end(); ++iterator)
  {
    SetChartTypeByLabel((*iterator)->GetLabel().toString().toStdString(), chartType);
  }

  auto chartTypeName = ConvertChartTypeToString(chartType);
  const QString command = QString::fromStdString("transformView('" + chartTypeName + "')");
  CallJavaScriptFuntion(command);
}

void QmitkChartWidget::Impl::SetChartTypeByLabel(const std::string &label, QmitkChartWidget::ChartType chartType)
{
  auto element = GetDataElementByLabel(label);
  if (element)
  {
    if (chartType == ChartType::scatter)
    {
      SetShowDataPoints(true);
      MITK_INFO << "Enabling data points for all because of scatter plot";
    }
    auto chartTypeName = ConvertChartTypeToString(chartType);
    element->SetChartType(QVariant(QString::fromStdString(chartTypeName)));
  }
}

void QmitkChartWidget::Impl::SetLegendPosition(QmitkChartWidget::LegendPosition legendPosition)
{
  const std::string legendPositionName(m_LegendPositionToName.at(legendPosition));
  m_C3Data.SetLegendPosition(QString::fromStdString(legendPositionName));
}

void QmitkChartWidget::Impl::Show(bool showSubChart)
{
  if (m_C3xyData.empty())
  {
    MITK_WARN << "no data available for display in chart";
  }
  else
  {
    m_C3Data.SetAppearance(showSubChart, m_C3xyData.front()->GetChartType() == QVariant("pie"));
  }

  InitializeJavaScriptChart();
}

void QmitkChartWidget::Impl::SetShowLegend(bool show)
{
  m_C3Data.SetShowLegend(show);
}

void QmitkChartWidget::Impl::SetStackedData(bool stacked)
{
  m_C3Data.SetStackedData(stacked);
}

void QmitkChartWidget::Impl::SetShowErrorBars(bool show)
{
  m_C3Data.SetShowErrorBars(show);
}

void QmitkChartWidget::Impl::SetShowDataPoints(bool showDataPoints)
{
  if (showDataPoints == true)
  {
    m_C3Data.SetDataPointSize(6.5);
  }
  else
  {
    m_C3Data.SetDataPointSize(0);
  }
}

void QmitkChartWidget::Impl::SetChartType(const std::string &label, QmitkChartWidget::ChartType chartType)
{
  auto element = GetDataElementByLabel(label);
  if (element)
  {
    if (chartType == ChartType::scatter)
    {
      SetShowDataPoints(true);
      MITK_INFO << "Enabling data points for all because of scatter plot";
    }
    const std::string chartTypeName(m_ChartTypeToName.at(chartType));
    element->SetChartType(QVariant(QString::fromStdString(chartTypeName)));
  }
}

QList<QVariant> QmitkChartWidget::Impl::ConvertErrorVectorToQList(const std::vector<double> &error)
{
  QList<QVariant> errorConverted;
  for (const auto &aValue : error)
  {
    errorConverted.append(aValue);
  }

  return errorConverted;
}

void QmitkChartWidget::Impl::SetXErrorBars(const std::string &label,
                                           const std::vector<double> &errorPlus,
                                           const std::vector<double> &errorMinus)
{
  auto element = GetDataElementByLabel(label);
  if (element)
  {
    auto errorConvertedPlus = ConvertErrorVectorToQList(errorPlus);
    auto errorConvertedMinus = ConvertErrorVectorToQList(errorMinus);

    element->SetXErrorDataPlus(errorConvertedPlus);
    element->SetXErrorDataMinus(errorConvertedMinus);
  }
}

void QmitkChartWidget::Impl::SetYErrorBars(const std::string &label,
                                           const std::vector<double> &errorPlus,
                                           const std::vector<double> &errorMinus)
{
  auto element = GetDataElementByLabel(label);
  if (element)
  {
    auto errorConvertedPlus = ConvertErrorVectorToQList(errorPlus);
    auto errorConvertedMinus = ConvertErrorVectorToQList(errorMinus);

    element->SetYErrorDataPlus(errorConvertedPlus);
    element->SetYErrorDataMinus(errorConvertedMinus);
  }
}

std::string QmitkChartWidget::Impl::ConvertChartTypeToString(QmitkChartWidget::ChartType chartType) const
{
  return m_ChartTypeToName.at(chartType);
}

QSize QmitkChartWidget::Impl::sizeHint() const
{
  return QSize(400, 300);
}

void QmitkChartWidget::Impl::CallJavaScriptFuntion(const QString &command)
{
  m_WebEngineView->page()->runJavaScript(command);
}

void QmitkChartWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///Chart/empty.html")));
}

void QmitkChartWidget::Impl::InitializeJavaScriptChart()
{
  auto alreadyRegisteredObjects = m_WebChannel->registeredObjects();
  auto alreadyRegisteredObjectsValues = alreadyRegisteredObjects.values();
  //only register objects that have not been registered yet
  if (alreadyRegisteredObjectsValues.indexOf(&m_C3Data) == -1)
  {
    m_WebChannel->registerObject(QStringLiteral("chartData"), &m_C3Data);
  }

  unsigned count = 0;
  for (auto &xyData : m_C3xyData)
  {
    // only register objects that have not been registered yet
    if (alreadyRegisteredObjectsValues.indexOf(xyData.get()) == -1)
    {
      QString variableName = "xyData" + QString::number(count);
      m_WebChannel->registerObject(variableName, xyData.get());
    }
    count++;
  }

  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///Chart/QmitkChartWidget.html")));
}

std::string QmitkChartWidget::Impl::GetUniqueLabelName(const QList<QVariant> &labelList, const std::string &label) const
{
  QString currentLabel = QString::fromStdString(label);
  int counter = 0;
  while (labelList.contains(currentLabel))
  {
    currentLabel = QString::fromStdString(label + std::to_string(counter));
    counter++;
  }
  return currentLabel.toStdString();
}

QmitkChartWidget::QmitkChartWidget(QWidget *parent) : QWidget(parent), m_Impl(new Impl(this)) {
  connect(this, &QmitkChartWidget::PageSuccessfullyLoaded, this, &QmitkChartWidget::OnPageSuccessfullyLoaded);
}

QmitkChartWidget::~QmitkChartWidget() {}

void QmitkChartWidget::AddData2D(const std::map<double, double> &data2D, const std::string &label, ChartType type)
{
  m_Impl->AddData2D(data2D, label, type);
}

void QmitkChartWidget::SetColor(const std::string &label, const std::string &colorName)
{
  m_Impl->SetColor(label, colorName);
}

void QmitkChartWidget::SetLineStyle(const std::string &label, LineStyle style)
{
  m_Impl->SetLineStyle(label, style);
}

void QmitkChartWidget::SetYAxisScale(AxisScale scale)
{
  m_Impl->SetYAxisScale(scale);
}

void QmitkChartWidget::AddData1D(const std::vector<double> &data1D, const std::string &label, ChartType type)
{
  m_Impl->AddData1D(data1D, label, type);
}

void QmitkChartWidget::RemoveData(const std::string &label)
{
  m_Impl->RemoveData(label);
}

void QmitkChartWidget::SetXAxisLabel(const std::string &label)
{
  m_Impl->SetXAxisLabel(label);
}

void QmitkChartWidget::SetYAxisLabel(const std::string &label)
{
  m_Impl->SetYAxisLabel(label);
}

void QmitkChartWidget::SetTitle(const std::string &title)
{
  m_Impl->SetTitle(title);
}

void QmitkChartWidget::SetShowDataPoints(bool showDataPoints)
{
  m_Impl->SetShowDataPoints(showDataPoints);
}

void QmitkChartWidget::SetChartType(const std::string &label, ChartType type)
{
  m_Impl->SetChartType(label, type);
}

void QmitkChartWidget::SetXErrorBars(const std::string &label,
                                     const std::vector<double> &errorPlus,
                                     const std::vector<double> &errorMinus)
{
  m_Impl->SetXErrorBars(label, errorPlus, errorMinus);
}

void QmitkChartWidget::SetYErrorBars(const std::string &label,
                                     const std::vector<double> &errorPlus,
                                     const std::vector<double> &errorMinus)
{
  m_Impl->SetYErrorBars(label, errorPlus, errorMinus);
}

void QmitkChartWidget::SetLegendPosition(LegendPosition position)
{
  m_Impl->SetLegendPosition(position);
}

void QmitkChartWidget::SetShowLegend(bool show)
{
  m_Impl->SetShowLegend(show);
}

void QmitkChartWidget::SetStackedData(bool stacked)
{
  m_Impl->SetStackedData(stacked);
}

void QmitkChartWidget::Show(bool showSubChart)
{
  m_Impl->Show(showSubChart);
}

void QmitkChartWidget::Clear()
{
  m_Impl->ClearData();
  m_Impl->ClearJavaScriptChart();
}

void QmitkChartWidget::OnLoadFinished(bool isLoadSuccessful)
{
  if (isLoadSuccessful)
  {
    emit PageSuccessfullyLoaded();
  }
}

void QmitkChartWidget::OnPageSuccessfullyLoaded() {
  auto themeName = m_Impl->GetThemeName();
  QString command;
  if (themeName == "dark")
  {
    command = QString("changeTheme('dark')");
  }
  else
  {
    command = QString("changeTheme('light')");
  }

  m_Impl->CallJavaScriptFuntion(command);
}

std::string QmitkChartWidget::convertBooleanValue(bool value) const {
  std::stringstream converter;
  converter << std::boolalpha << value;
  return converter.str();
}

void QmitkChartWidget::SetChartTypeForAllDataAndReload(ChartType type)
{
  m_Impl->SetChartType(type);
}

void QmitkChartWidget::SetTheme(ColorTheme themeEnabled)
{
  m_Impl->SetThemeName(themeEnabled);
}

void QmitkChartWidget::SetShowSubchart(bool showSubChart)
{
  QString subChartString = QString::fromStdString(convertBooleanValue(showSubChart));
  const QString command = QString("SetShowSubchart(" + subChartString + ")");
  m_Impl->CallJavaScriptFuntion(command);
}

void QmitkChartWidget::SetShowErrorBars(bool showErrorBars)
{
  m_Impl->SetShowErrorBars(showErrorBars);
}

void QmitkChartWidget::UpdateMinMaxValueXView(double minValueX, double maxValueX)
{
  QString minMaxValueXString = QString::fromStdString(std::to_string(minValueX))+QString(",");
  minMaxValueXString += QString::fromStdString(std::to_string(maxValueX));
  const QString command = QString("UpdateMinMaxValueXView(" + minMaxValueXString + ")");
  m_Impl->CallJavaScriptFuntion(command);
}

void QmitkChartWidget::UpdateMinMaxValueYView(double minValueY, double maxValueY) {
  QString minMaxValueYString = QString::fromStdString(std::to_string(minValueY)) + QString(",");
  minMaxValueYString += QString::fromStdString(std::to_string(maxValueY));
  const QString command = QString("UpdateMinMaxValueYView(" + minMaxValueYString + ")");
  m_Impl->CallJavaScriptFuntion(command);
}

void QmitkChartWidget::Reload()
{
  const QString command = QString("Reload()");
  m_Impl->CallJavaScriptFuntion(command);
}

QSize QmitkChartWidget::sizeHint() const
{
  return m_Impl->sizeHint();
}
