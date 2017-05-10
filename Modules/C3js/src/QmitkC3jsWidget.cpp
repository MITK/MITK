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

#include <QmitkC3jsWidget.h>
#include <QGridLayout>
#include <QWebChannel>
#include <QWebEngineView>

#include <QmitkC3Data.h>
#include <QmitkC3xyData.h>

class QmitkC3jsWidget::Impl final
{
public:
  explicit Impl(QWidget* parent);
  ~Impl();

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  QWebChannel* GetWebChannel();

  void AddData2D(const QMap<QVariant, QVariant>& data2D) { m_data2D.push_back(data2D); }
  QList<QMap<QVariant, QVariant> > GetData2D() const { return m_data2D; }

  void AddData1D(const QList<QVariant>& data1D) { m_data1D.push_back(data1D); }
  QList<QList<QVariant> > GetData1D() const { return m_data1D; }

  void SetDataLabels(const QList<QString>& labels) { m_dataLabels = labels; }
  QList<QString> GetDataLabels() const { return m_dataLabels; }
  QList<QVariant> GetDataLabelsAsQVariant() const;

  void SetXAxisLabel(const QString& label) { m_xAxisLabel = label; };
  QString GetXAxisLabel() const { return m_xAxisLabel; };
  QVariant GetXAxisLabelAsQVariant() const;

  void SetYAxisLabel(const QString& label) { m_yAxisLabel = label; };
  QString GetYAxisLabel() const { return m_yAxisLabel; };
  QVariant GetYAxisLabelAsQVariant() const;

  void SetDiagramType(QmitkC3jsWidget::DiagramType diagramType) { m_diagramType = diagramType; }
  QmitkC3jsWidget::DiagramType GetDiagramType() const { return m_diagramType; }
  QVariant GetDiagramTypeAsQVariant() const;

  void ClearJavaScriptChart();
  void initializeJavaScriptChart();
  void callJavaScriptFuntion(QString command);

  QmitkC3Data* GetC3Data() { return &m_c3Data; };
  QVector<QmitkC3xyData*>* GetC3xyData() { return &m_c3xyData; };

private:
  QWidget* m_Parent;
  QWebChannel* m_WebChannel;
  QWebEngineView* m_WebEngineView;

  QList<QMap<QVariant, QVariant> > m_data2D;
  QList<QList<QVariant> > m_data1D;
  QList<QString> m_dataLabels;
  QString m_xAxisLabel;
  QString m_yAxisLabel;

  QmitkC3jsWidget::DiagramType m_diagramType;

  QmitkC3Data m_c3Data;
  QVector<QmitkC3xyData*> m_c3xyData;
  QMap<QmitkC3jsWidget::DiagramType, QVariant> m_diagramTypeToName;
};

QmitkC3jsWidget::Impl::Impl(QWidget* parent)
  : m_WebChannel(new QWebChannel(parent)),
  m_WebEngineView(new QWebEngineView(parent)),
  m_Parent(parent), m_diagramType(DiagramType::line)
{
  //disable context menu for QWebEngineView
  m_WebEngineView->setContextMenuPolicy(Qt::NoContextMenu);

  //Set the webengineview to an initial empty page. The actual chart will be loaded once the data is calculated.
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
  m_WebEngineView->page()->setWebChannel(m_WebChannel);

  connect( m_WebEngineView, SIGNAL( loadFinished(bool) ), parent, SLOT( OnLoadFinished(bool) ) );

  auto layout = new QGridLayout(parent);
  layout->setMargin(0);
  layout->addWidget(m_WebEngineView);

  parent->setLayout(layout);

  m_diagramTypeToName.insert(DiagramType::bar, QVariant("bar"));
  m_diagramTypeToName.insert(DiagramType::line, QVariant("line"));
  m_diagramTypeToName.insert(DiagramType::spline, QVariant("spline"));
  m_diagramTypeToName.insert(DiagramType::pie, QVariant("pie"));
}

QmitkC3jsWidget::Impl::~Impl()
{
}

QWebChannel* QmitkC3jsWidget::Impl::GetWebChannel()
{
  return m_WebChannel;
}

QmitkC3jsWidget::QmitkC3jsWidget(QWidget* parent)
  : QWidget(parent),
  m_Impl(new Impl(this))
{
}

QmitkC3jsWidget::QmitkC3jsWidget(const QString& id, QObject* object, QWidget* parent)
	: QWidget(parent),
	m_Impl(new Impl(this))
{
	if (!id.isEmpty() && object != nullptr)
		m_Impl->GetWebChannel()->registerObject(id, object);
}

void QmitkC3jsWidget::Impl::callJavaScriptFuntion(QString command)
{
  m_WebEngineView->page()->runJavaScript(command);
}

QList<QVariant> QmitkC3jsWidget::Impl::GetDataLabelsAsQVariant() const
{
	QList<QVariant> result;
	std::copy(m_dataLabels.begin(), m_dataLabels.end(), std::back_inserter(result));
	return result;
}

QVariant QmitkC3jsWidget::Impl::GetXAxisLabelAsQVariant() const
{
	return m_xAxisLabel;
}

QVariant QmitkC3jsWidget::Impl::GetYAxisLabelAsQVariant() const
{
	return m_yAxisLabel;
}

QVariant QmitkC3jsWidget::Impl::GetDiagramTypeAsQVariant() const
{
	return m_diagramTypeToName.value(m_diagramType);
}

void QmitkC3jsWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
}

void QmitkC3jsWidget::Impl::initializeJavaScriptChart()
{
  m_WebChannel->registerObject(QStringLiteral("chartData"), &m_c3Data);
  unsigned count = 0;
  for (auto& xyData : m_c3xyData) {
	  QString variableName = "xyData" + QString::number(count);
	  m_WebChannel->registerObject(variableName, xyData);
	  count++;
  }
  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///C3js/QmitkC3jsWidget.html")));
}


QmitkC3jsWidget::~QmitkC3jsWidget()
{
  delete m_Impl;
}

void QmitkC3jsWidget::AddData2D(const QMap<QVariant, QVariant>& data2D)
{
	m_Impl->AddData2D(data2D);
}

QList<QMap<QVariant, QVariant> > QmitkC3jsWidget::GetData2D() const
{
	return m_Impl->GetData2D();
}

void QmitkC3jsWidget::AddData1D(const QList<QVariant>& data1D)
{
	QMap<QVariant, QVariant> augmentedData;
	unsigned int count = 0;
	//just augment the 1D data
	for (const auto& ele : data1D) {
		augmentedData[count] = ele;
		count++ ;
	}
	m_Impl->AddData2D(augmentedData);
}

QList<QList<QVariant> > QmitkC3jsWidget::GetData1D() const
{
	QList<QList<QVariant> > emulatedData1D;
	auto data2D = m_Impl->GetData2D();
	for (const auto& listElement : data2D) {
		QList<QVariant> innerList;
		for (const auto& innerListElement : listElement.toStdMap()) {
			innerList.push_back(innerListElement.second);
		}
		emulatedData1D.push_back(innerList);
	}
	return emulatedData1D;
}

void QmitkC3jsWidget::SetDataLabels(const QList<QString>& labels)
{
	m_Impl->SetDataLabels(labels);
}

QList<QString> QmitkC3jsWidget::GetDataLabels() const
{
	return m_Impl->GetDataLabels();
}

void QmitkC3jsWidget::SetXAxisLabel(const QString & label)
{
	m_Impl->SetXAxisLabel(label);
}

QString QmitkC3jsWidget::GetXAxisLabel() const
{
	return m_Impl->GetXAxisLabel();
}

void QmitkC3jsWidget::SetYAxisLabel(const QString & label)
{
	m_Impl->SetYAxisLabel(label);
}

QString QmitkC3jsWidget::GetYAxisLabel() const
{
	return m_Impl->GetYAxisLabel();
}

void QmitkC3jsWidget::SetDiagramType(DiagramType type)
{
	m_Impl->SetDiagramType(type);
}

QmitkC3jsWidget::DiagramType QmitkC3jsWidget::GetDiagramType() const
{
	return m_Impl->GetDiagramType();
}

void QmitkC3jsWidget::ShowDiagram(bool showSubChart)
{
	//Clear old data before loading new data. (Check if Statistics Plugin)
	//this->m_Impl->GetC3Data()->ClearData();
	this->m_Impl->GetC3Data()->SetXAxisLabel(m_Impl->GetXAxisLabelAsQVariant());
	this->m_Impl->GetC3Data()->SetYAxisLabel(m_Impl->GetYAxisLabelAsQVariant());
	this->m_Impl->GetC3Data()->SetDataLabels(m_Impl->GetDataLabelsAsQVariant());

	this->m_Impl->GetC3Data()->SetAppearance(m_Impl->GetDiagramTypeAsQVariant(), showSubChart, m_Impl->GetDiagramType()== DiagramType::pie);

	auto data2D = m_Impl->GetData2D();

	for (const auto& listEntry : data2D) {
		QmitkC3xyData * xyData = new QmitkC3xyData(listEntry);
		this->m_Impl->GetC3xyData()->push_back(xyData);
	}
	m_Impl->initializeJavaScriptChart();
}

void QmitkC3jsWidget::ClearDiagram()
{
	m_Impl->ClearJavaScriptChart();
}

void QmitkC3jsWidget::OnLoadFinished(bool isLoadSuccessfull)
{
  emit PageSuccessfullyLoaded();
}

void QmitkC3jsWidget::TransformView(QString transformTo)
{
  QString command = QString("transformView('" + transformTo + "')");
  m_Impl->callJavaScriptFuntion(command);
}

void QmitkC3jsWidget::SendCommand(QString command)
{
  m_Impl->callJavaScriptFuntion(command);
}
