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

  void AddData1D(const std::vector<double>& data1D);

  void AddData2D(const std::map<double, double>& data2D);

  void ClearData2D();

  void SetDataLabels(const std::vector<std::string>& labels);

  std::vector<std::string> GetDataLabels() const;

  void SetXAxisLabel(const std::string& label);
  std::string GetXAxisLabel() const;

  void SetYAxisLabel(const std::string& label);
  std::string GetYAxisLabel() const;

  void SetDiagramType(QmitkChartWidget::ChartType diagramType);
  ChartType GetDiagramType() const;
  std::string GetDiagramTypeAsString() const;

  void ClearJavaScriptChart();
  void initializeJavaScriptChart();
  void callJavaScriptFuntion(const QString& command);

  QmitkChartData* GetC3Data() const;
  std::vector<QmitkChartxyData*>* GetC3xyData() const;

private:
  QWebChannel* m_WebChannel;
  QWebEngineView* m_WebEngineView;
  QWidget* m_Parent;

  QmitkChartData * m_C3Data;
  std::vector<QmitkChartxyData*> * m_C3xyData;
  std::map<QmitkChartWidget::ChartType, std::string> m_DiagramTypeToName;
};

QmitkChartWidget::Impl::Impl(QWidget* parent)
  : m_WebChannel(new QWebChannel(parent)),
  m_WebEngineView(new QWebEngineView(parent)),
  m_Parent(parent)
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

  m_DiagramTypeToName.emplace(ChartType::bar, "bar");
  m_DiagramTypeToName.emplace(ChartType::line, "line");
  m_DiagramTypeToName.emplace(ChartType::spline, "spline");
  m_DiagramTypeToName.emplace(ChartType::pie, "pie");

  m_C3Data = new QmitkChartData();
  m_C3xyData = new std::vector<QmitkChartxyData*>();
}

QmitkChartWidget::Impl::~Impl()
{
  delete m_C3Data;
  delete m_C3xyData;
}

void QmitkChartWidget::Impl::SetDataLabels(const std::vector<std::string>& labels)
{
  QList<QVariant> variantList;
  for (const auto& label : labels) {
    variantList.append(QString::fromStdString(label));
  }
  GetC3Data()->SetDataLabels(variantList);
}

void QmitkChartWidget::Impl::AddData1D(const std::vector<double>& data1D) {
  QList<QVariant> data1DConverted;
  for (const auto& aValue : data1D) {
    data1DConverted.append(aValue);
  }
  GetC3xyData()->push_back(new QmitkChartxyData(data1DConverted));
}

void QmitkChartWidget::Impl::AddData2D(const std::map<double, double>& data2D) {
  QMap<QVariant, QVariant> data2DConverted;
  for (const auto& aValue : data2D) {
    data2DConverted.insert(aValue.first, aValue.second);
  }
  GetC3xyData()->push_back(new QmitkChartxyData(data2DConverted));
}

std::vector<std::string> QmitkChartWidget::Impl::GetDataLabels() const {
  auto dataLabels = GetC3Data()->GetDataLabels();
  std::vector<std::string> dataLabelsAsStringVector;
  for (const auto& label : dataLabels) {
    dataLabelsAsStringVector.push_back(label.toString().toStdString());
  }
  return dataLabelsAsStringVector;
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

void QmitkChartWidget::Impl::SetDiagramType(QmitkChartWidget::ChartType diagramType) {
  const std::string diagramTypeName(m_DiagramTypeToName.at(diagramType));
  GetC3Data()->SetDiagramType(QString::fromStdString(diagramTypeName)); 
}

QmitkChartWidget::ChartType QmitkChartWidget::Impl::GetDiagramType() const {
  for (const auto& aDiagramType : m_DiagramTypeToName) {
    if (aDiagramType.second == GetDiagramTypeAsString()){
      return aDiagramType.first;
    }
  }
  mitkThrow() << "can't find diagram type!";
}

std::string QmitkChartWidget::Impl::GetDiagramTypeAsString() const {
  return GetC3Data()->GetDiagramType().toString().toStdString();
}

QmitkChartData* QmitkChartWidget::Impl::GetC3Data() const {
  return m_C3Data; 
}

std::vector<QmitkChartxyData*>* QmitkChartWidget::Impl::GetC3xyData() const {
  return m_C3xyData; 
}

void QmitkChartWidget::Impl::ClearData2D() {
  GetC3xyData()->clear(); 
}

QmitkChartWidget::QmitkChartWidget(ChartType type, QWidget* parent)
  : QWidget(parent),
  m_Impl(new Impl(this))
{
  SetChartType(type);
}

void QmitkChartWidget::Impl::callJavaScriptFuntion(const QString& command)
{
  m_WebEngineView->page()->runJavaScript(command);
}

void QmitkChartWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
}

void QmitkChartWidget::Impl::initializeJavaScriptChart()
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

QmitkChartWidget::~QmitkChartWidget()
{
  delete m_Impl;
}

void QmitkChartWidget::AddData2D(const std::map<double, double>& data2D)
{
	m_Impl->AddData2D(data2D);
}

void QmitkChartWidget::AddData1D(const std::vector<double>& data1D)
{
  m_Impl->AddData1D(data1D);
}

void QmitkChartWidget::SetDataLabels(const std::vector<std::string>& labels)
{
	m_Impl->SetDataLabels(labels);
}

std::vector<std::string> QmitkChartWidget::GetDataLabels() const
{
	return m_Impl->GetDataLabels();
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

void QmitkChartWidget::SetChartType(ChartType type)
{
	m_Impl->SetDiagramType(type);
}

QmitkChartWidget::ChartType QmitkChartWidget::GetChartType() const
{
  return m_Impl->GetDiagramType();
}

void QmitkChartWidget::Show(bool showSubChart)
{
	this->m_Impl->GetC3Data()->SetAppearance(m_Impl->GetC3Data()->GetDiagramType(), showSubChart, m_Impl->GetC3Data()->GetDiagramType()== QVariant("pie"));

	m_Impl->initializeJavaScriptChart();
}

void QmitkChartWidget::Clear()
{
	m_Impl->ClearData2D();
	m_Impl->GetC3xyData()->clear();
	m_Impl->ClearJavaScriptChart();
}

void QmitkChartWidget::OnLoadFinished(bool isLoadSuccessfull)
{
  if(isLoadSuccessfull)
  {
    emit PageSuccessfullyLoaded();
  }
}

void QmitkChartWidget::SetChartTypeAndReload(ChartType type)
{
  SetChartType(type);
  auto diagramTypeName = m_Impl->GetDiagramTypeAsString();
  const QString command = QString::fromStdString("transformView('" + diagramTypeName + "')");
  m_Impl->callJavaScriptFuntion(command);
}

void QmitkChartWidget::ChangeTheme(bool darkThemeEnabled)
{
  QString command;
  if (darkThemeEnabled)
  {
    command = QString("changeTheme('dark')");
  }
  else {
    command = QString("changeTheme('default')");
  }
  m_Impl->callJavaScriptFuntion(command);
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
  m_Impl->callJavaScriptFuntion(command);
}
