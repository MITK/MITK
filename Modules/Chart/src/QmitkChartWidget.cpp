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

#include <QmitkChartWidget.h>
#include <QGridLayout>
#include <QWebChannel>
#include <QWebEngineView>

#include <QmitkChartData.h>
#include <QmitkChartxyData.h>
#include "mitkExceptionMacro.h"

class QmitkChartWidget::Impl final
{
public:
  explicit Impl(QWidget* parent);
  ~Impl();

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  void AddData1D(const std::vector<double>& data1D, const std::string& label, QmitkChartWidget::ChartType chartType);
  void AddData2D(const std::map<double, double>& data2D, const std::string& label, QmitkChartWidget::ChartType chartType);

  void RemoveData(const std::string& label);

  void ClearData();

  void SetColor(const std::string& label, const std::string& colorName);
  void SetLineStyle(const std::string& label, LineStyle style);

  void SetYAxisScale(AxisScale scale);

  void SetXAxisLabel(const std::string& label);
  std::string GetXAxisLabel() const;

  void SetYAxisLabel(const std::string& label);
  std::string GetYAxisLabel() const;

  void SetTitle(const std::string &title);
  std::string GetTitle() const;

  void SetLegendPosition(LegendPosition position);
  LegendPosition GetLegendPosition() const;
  std::string GetLegendPositionAsString() const;

  void SetShowDataPoints(bool showDataPoints = false);
  bool GetShowDataPoints() const;

  void SetChartType(const std::string& label, QmitkChartWidget::ChartType chartType);
  std::string ConvertChartTypeToString(QmitkChartWidget::ChartType chartType) const;

  void ClearJavaScriptChart();
  void InitializeJavaScriptChart();
  void CallJavaScriptFuntion(const QString& command);

  QmitkChartData* GetC3Data() const;
  std::vector<QmitkChartxyData*>* GetC3xyData() const;

private:
  std::string GetUniqueLabelName(const QList<QVariant>& labelList, const std::string& label) const;
  QmitkChartxyData* GetDataElementByLabel(const std::vector<QmitkChartxyData*>* c3xyData, const std::string& label) const;
  QList<QVariant> GetDataLabels(const std::vector<QmitkChartxyData*>* c3xyData) const;

  QWebChannel* m_WebChannel;
  QWebEngineView* m_WebEngineView;

  QmitkChartData * m_C3Data;
  std::vector<QmitkChartxyData*> * m_C3xyData;
  std::map<QmitkChartWidget::ChartType, std::string> m_ChartTypeToName;
  std::map<QmitkChartWidget::LegendPosition, std::string> m_LegendPositionToName;
  std::map<QmitkChartWidget::LineStyle, std::string> m_LineStyleToName;
  std::map<QmitkChartWidget::AxisScale, std::string> m_AxisScaleToName;
};

QmitkChartWidget::Impl::Impl(QWidget* parent)
  : m_WebChannel(new QWebChannel(parent)),
  m_WebEngineView(new QWebEngineView(parent))
{
  //disable context menu for QWebEngineView
  m_WebEngineView->setContextMenuPolicy(Qt::NoContextMenu);

  //Set the webengineview to an initial empty page. The actual chart will be loaded once the data is calculated.
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
  m_WebEngineView->page()->setWebChannel(m_WebChannel);

  connect(m_WebEngineView, SIGNAL(loadFinished(bool)), parent, SLOT(OnLoadFinished(bool)));

  auto layout = new QGridLayout(parent);
  layout->setMargin(0);
  layout->addWidget(m_WebEngineView);

  parent->setLayout(layout);

  m_ChartTypeToName.emplace(ChartType::bar, "bar");
  m_ChartTypeToName.emplace(ChartType::line, "line");
  m_ChartTypeToName.emplace(ChartType::spline, "spline");
  m_ChartTypeToName.emplace(ChartType::pie, "pie");
  m_ChartTypeToName.emplace(ChartType::area, "area");
  m_ChartTypeToName.emplace(ChartType::area_spline  , "area-spline");

  m_LegendPositionToName.emplace(LegendPosition::bottom, "bottom");
  m_LegendPositionToName.emplace(LegendPosition::right, "right");
  m_LegendPositionToName.emplace(LegendPosition::inset, "inset");

  m_LineStyleToName.emplace(LineStyle::solid, "solid");
  m_LineStyleToName.emplace(LineStyle::dashed, "dashed");

  m_AxisScaleToName.emplace(AxisScale::linear, "");
  m_AxisScaleToName.emplace(AxisScale::log, "log");

  m_C3Data = new QmitkChartData();
  m_C3xyData = new std::vector<QmitkChartxyData*>();
}

QmitkChartWidget::Impl::~Impl()
{
  delete m_C3Data;
  delete m_C3xyData;
}

void QmitkChartWidget::Impl::AddData1D(const std::vector<double>& data1D, const std::string& label, QmitkChartWidget::ChartType type) {
  std::map<double, double> transformedData2D;
  unsigned int count = 0;
  //transform the 1D data to 2D data
  for (const auto& ele : data1D) {
    transformedData2D[count] = ele;
    count++;
  }

  this->AddData2D(transformedData2D, label, type);
}

void QmitkChartWidget::Impl::AddData2D(const std::map<double, double>& data2D, const std::string& label, QmitkChartWidget::ChartType type) {
  QMap<QVariant, QVariant> data2DConverted;
  for (const auto& aValue : data2D) {
    data2DConverted.insert(aValue.first, aValue.second);
  }
  const std::string chartTypeName(m_ChartTypeToName.at(type));

  auto definedLabels = GetDataLabels(GetC3xyData());
  auto uniqueLabel = GetUniqueLabelName(definedLabels, label);

  GetC3xyData()->push_back(new QmitkChartxyData(data2DConverted, QVariant(QString::fromStdString(uniqueLabel)), QVariant(QString::fromStdString(chartTypeName))));
}

void QmitkChartWidget::Impl::RemoveData(const std::string& label) {
  std::vector<QmitkChartxyData*>::iterator iter_temp = GetC3xyData()->end();
  for (std::vector<QmitkChartxyData*>::iterator iter = GetC3xyData()->begin(); iter != GetC3xyData()->end(); ++iter)
  {
    const auto &temp = *iter;
    if (temp->GetLabel().toString().toStdString() == label)
    {
      iter_temp = iter;
      break;
    }
  }
  if (iter_temp == GetC3xyData()->end()) {
    throw std::invalid_argument("Cannot Remove Data because the label does not exist.");
  }
  else
  {
    GetC3xyData()->erase(iter_temp);
  }
}

void QmitkChartWidget::Impl::ClearData() {
  for (auto& xyData : *m_C3xyData) {
    m_WebChannel->deregisterObject(xyData);
  }
  GetC3xyData()->clear();
}

void QmitkChartWidget::Impl::SetColor(const std::string& label, const std::string& colorName)
{
  auto element = GetDataElementByLabel(GetC3xyData(), label);
  if (element) {
    element->SetColor(QVariant(QString::fromStdString(colorName)));
  }
}

void QmitkChartWidget::Impl::SetLineStyle(const std::string& label, LineStyle style) {
  auto element = GetDataElementByLabel(GetC3xyData(), label);
  //only has effect with chart type line
  if (element && element->GetChartType()==QVariant(QString::fromStdString(ConvertChartTypeToString(ChartType::line)))) {
    const std::string lineStyleName(m_LineStyleToName.at(style));
    element->SetLineStyle(QVariant(QString::fromStdString(lineStyleName)));
  }
}

void QmitkChartWidget::Impl::SetYAxisScale(AxisScale scale)
{
  const std::string axisScaleName(m_AxisScaleToName.at(scale));
  GetC3Data()->SetYAxisScale(QString::fromStdString(axisScaleName));
}

QmitkChartxyData* QmitkChartWidget::Impl::GetDataElementByLabel(const std::vector<QmitkChartxyData*>* c3xyData, const std::string& label) const
{
  for (auto element = c3xyData->begin(); element != c3xyData->end(); ++element) {
    if ((*element)->GetLabel().toString().toStdString() == label) {
      return *element;
    }
  }
  MITK_WARN << "label " << label << " not found in QmitkChartWidget";
  return nullptr;
}

QList<QVariant> QmitkChartWidget::Impl::GetDataLabels(const std::vector<QmitkChartxyData*>* c3xyData) const {
  QList<QVariant> dataLabels;
  for (auto element = c3xyData->begin(); element != c3xyData->end(); ++element) {
    dataLabels.push_back((*element)->GetLabel());
  }
  return dataLabels;
}

void QmitkChartWidget::Impl::SetXAxisLabel(const std::string& label) { 
  GetC3Data()->SetXAxisLabel(QString::fromStdString(label)); 
}

std::string QmitkChartWidget::Impl::GetXAxisLabel() const { 
  return GetC3Data()->GetXAxisLabel().toString().toStdString(); 
}

void QmitkChartWidget::Impl::SetYAxisLabel(const std::string& label) {
  GetC3Data()->SetYAxisLabel(QString::fromStdString(label)); 
}

std::string QmitkChartWidget::Impl::GetYAxisLabel() const { 
  return GetC3Data()->GetYAxisLabel().toString().toStdString(); 
}

void QmitkChartWidget::Impl::SetTitle(const std::string& title) {
  GetC3Data()->SetTitle(QString::fromStdString(title));
}

std::string QmitkChartWidget::Impl::GetTitle() const {
  return GetC3Data()->GetTitle().toString().toStdString();
}

void QmitkChartWidget::Impl::SetLegendPosition(QmitkChartWidget::LegendPosition legendPosition) {
  const std::string legendPositionName(m_LegendPositionToName.at(legendPosition));
  GetC3Data()->SetLegendPosition(QString::fromStdString(legendPositionName));
}

QmitkChartWidget::LegendPosition QmitkChartWidget::Impl::GetLegendPosition() const {
  for (const auto& aLegendPosition : m_LegendPositionToName) {
    if (aLegendPosition.second == GetLegendPositionAsString()) {
      return aLegendPosition.first;
    }
  }
  mitkThrow() << "can't find legend position!";
}

std::string QmitkChartWidget::Impl::GetLegendPositionAsString() const {
  return GetC3Data()->GetLegendPosition().toString().toStdString();
}

void QmitkChartWidget::Impl::SetShowDataPoints(bool showDataPoints) {
  if (showDataPoints == true) {
    GetC3Data()->SetDataPointSize(3);
  }
  else {
    GetC3Data()->SetDataPointSize(0);
  }
}

bool QmitkChartWidget::Impl::GetShowDataPoints() const{
  int value = GetC3Data()->GetDataPointSize().toInt();
  if (value > 0) {
    return true;
  }
  else {
    return false;
  }
}

void QmitkChartWidget::Impl::SetChartType(const std::string& label, QmitkChartWidget::ChartType chartType) {
  auto element = GetDataElementByLabel(GetC3xyData(), label);
  if (element) {
    const std::string chartTypeName(m_ChartTypeToName.at(chartType));
    element->SetChartType(QVariant(QString::fromStdString(chartTypeName)));
  }
}

std::string QmitkChartWidget::Impl::ConvertChartTypeToString(QmitkChartWidget::ChartType chartType) const {
  return m_ChartTypeToName.at(chartType);
}

QmitkChartData* QmitkChartWidget::Impl::GetC3Data() const {
  return m_C3Data; 
}

std::vector<QmitkChartxyData*>* QmitkChartWidget::Impl::GetC3xyData() const {
  return m_C3xyData; 
}

void QmitkChartWidget::Impl::CallJavaScriptFuntion(const QString& command)
{
  m_WebEngineView->page()->runJavaScript(command);
}

void QmitkChartWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
}

void QmitkChartWidget::Impl::InitializeJavaScriptChart()
{
  m_WebChannel->registerObject(QStringLiteral("chartData"), m_C3Data);
  unsigned count = 0;
  for (auto& xyData : *m_C3xyData) {
	  QString variableName = "xyData" + QString::number(count);
	  m_WebChannel->registerObject(variableName, xyData);
	  count++;
  }
  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///C3js/QmitkChartWidget.html")));
}

std::string QmitkChartWidget::Impl::GetUniqueLabelName(const QList<QVariant>& labelList, const std::string& label) const
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

QmitkChartWidget::QmitkChartWidget(QWidget* parent)
  : QWidget(parent),
  m_Impl(new Impl(this))
{
}

QmitkChartWidget::~QmitkChartWidget()
{
  delete m_Impl;
}

void QmitkChartWidget::AddData2D(const std::map<double, double>& data2D, const std::string& label, ChartType type)
{
  m_Impl->AddData2D(data2D, label, type);
}

void QmitkChartWidget::SetColor(const std::string& label, const std::string& colorName)
{
  m_Impl->SetColor(label, colorName);
}

void QmitkChartWidget::SetLineStyle(const std::string& label, LineStyle style)
{
  m_Impl->SetLineStyle(label, style);
}

void QmitkChartWidget::SetYAxisScale(AxisScale scale)
{
  m_Impl->SetYAxisScale(scale);
}

void QmitkChartWidget::AddData1D(const std::vector<double>& data1D, const std::string& label, ChartType type)
{
  m_Impl->AddData1D(data1D, label, type);
}

void QmitkChartWidget::RemoveData(const std::string& label)
{
  m_Impl->RemoveData(label);
}

void QmitkChartWidget::SetXAxisLabel(const std::string & label)
{
	m_Impl->SetXAxisLabel(label);
}

std::string QmitkChartWidget::GetXAxisLabel() const
{
	return m_Impl->GetXAxisLabel();
}

void QmitkChartWidget::SetYAxisLabel(const std::string & label)
{
	m_Impl->SetYAxisLabel(label);
}

std::string QmitkChartWidget::GetYAxisLabel() const
{
	return m_Impl->GetYAxisLabel();
}

void QmitkChartWidget::SetTitle(const std::string & title)
{
  m_Impl->SetTitle(title);
}

std::string QmitkChartWidget::GetTitle() const
{
  return m_Impl->GetTitle();
}

void QmitkChartWidget::SetShowDataPoints(bool showDataPoints)
{
    m_Impl->SetShowDataPoints(showDataPoints);
}
bool QmitkChartWidget::GetShowDataPoints() const
{
  return m_Impl->GetShowDataPoints();
}

void QmitkChartWidget::SetChartType(const std::string& label, ChartType type)
{
  m_Impl->SetChartType(label, type);
}

void QmitkChartWidget::SetLegendPosition(LegendPosition position)
{
  m_Impl->SetLegendPosition(position);
}

QmitkChartWidget::LegendPosition QmitkChartWidget::GetLegendPosition() const
{
  return m_Impl->GetLegendPosition();
}

void QmitkChartWidget::Show(bool showSubChart)
{
  if (m_Impl->GetC3xyData()->empty()) {
    mitkThrow() << "no data available for display in chart";
  }
  this->m_Impl->GetC3Data()->SetAppearance(showSubChart, m_Impl->GetC3xyData()->front()->GetChartType()== QVariant("pie"));
  m_Impl->InitializeJavaScriptChart();
}

void QmitkChartWidget::Clear()
{
	m_Impl->ClearData();
	m_Impl->ClearJavaScriptChart();
}

void QmitkChartWidget::OnLoadFinished(bool isLoadSuccessfull)
{
  if(isLoadSuccessfull)
  {
    emit PageSuccessfullyLoaded();
  }
}

void QmitkChartWidget::SetChartTypeForAllDataAndReload(ChartType type)
{
  auto allData = this->m_Impl->GetC3xyData();
  for (auto iterator = allData->begin(); iterator != allData->end(); ++iterator) {
    SetChartType((*iterator)->GetLabel().toString().toStdString(), type);
  }
  auto chartTypeName = m_Impl->ConvertChartTypeToString(type);
  const QString command = QString::fromStdString("transformView('" + chartTypeName + "')");
  m_Impl->CallJavaScriptFuntion(command);
}

void QmitkChartWidget::SetTheme(ChartStyle themeEnabled)
{
  QString command;
  if (themeEnabled == ChartStyle::darkstyle)
  {
    command = QString("changeTheme('dark')");
  }
  else {
    command = QString("changeTheme('default')");
  }
  m_Impl->CallJavaScriptFuntion(command);
}

void QmitkChartWidget::Reload(bool showSubChart)
{
  QString subChartString;
  if (showSubChart) {
    subChartString = "true";
  }
  else {
    subChartString = "false";
  }
  const QString command = QString("ReloadChart(" + subChartString + ")");
  m_Impl->CallJavaScriptFuntion(command);
}
