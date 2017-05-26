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

#include <iostream>

class QmitkC3jsWidget::Impl final
{
public:
  explicit Impl(QWidget* parent);
  ~Impl();

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  QWebChannel* GetWebChannel();

  void AddData1D(const QList<QVariant>& data1D) { GetC3xyData()->push_back(new QmitkC3xyData(data1D)); }
  void AddData2D(const QMap<QVariant, QVariant>& data2D) { GetC3xyData()->push_back(new QmitkC3xyData(data2D)); }

  void ClearData2D() { GetC3xyData()->clear(); }

  void SetDataLabels(const QList<QString>& labels);

  QList<QString> GetDataLabels() const {
    auto dataLabels = GetC3Data()->GetDataLabels();
    QList<QString> dataLabelsAsString;
    for (const auto& label : dataLabels) {
      dataLabelsAsString.push_back(label.toString());
    }
    return dataLabelsAsString;
  }

  void SetXAxisLabel(const QString& label) { GetC3Data()->SetXAxisLabel(label); }
  QString GetXAxisLabel() const { return GetC3Data()->GetXAxisLabel().toString(); };

  void SetYAxisLabel(const QString& label) { GetC3Data()->SetYAxisLabel(label); };
  QString GetYAxisLabel() const { return GetC3Data()->GetYAxisLabel().toString(); };

  void SetDiagramType(QmitkC3jsWidget::DiagramType diagramType) { GetC3Data()->SetDiagramType(m_diagramTypeToName.value(diagramType)); }
  DiagramType GetDiagramType() const { return m_diagramTypeToName.key(GetC3Data()->GetDiagramType()); }

  void ClearJavaScriptChart();
  void initializeJavaScriptChart();
  void callJavaScriptFuntion(QString command);

  QmitkC3Data* GetC3Data() const { return m_c3Data; };
  QVector<QmitkC3xyData*>* GetC3xyData() const { return m_c3xyData; };

private:
  QWidget* m_Parent;
  QWebChannel* m_WebChannel;
  QWebEngineView* m_WebEngineView;

  QmitkC3Data * m_c3Data;
  QVector<QmitkC3xyData*> * m_c3xyData;
  QMap<QmitkC3jsWidget::DiagramType, QVariant> m_diagramTypeToName;
};

void QmitkC3jsWidget::Impl::SetDataLabels(const QList<QString>& labels)
{
  QVector<QVariant> variantList(labels.size());
  qCopy(labels.begin(), labels.end(), variantList.begin());
  GetC3Data()->SetDataLabels(variantList.toList());
}

QmitkC3jsWidget::Impl::Impl(QWidget* parent)
  : m_WebChannel(new QWebChannel(parent)),
  m_WebEngineView(new QWebEngineView(parent)),
  m_Parent(parent)
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

  m_c3Data = new QmitkC3Data();
  m_c3xyData = new QVector<QmitkC3xyData*>();
}

QmitkC3jsWidget::Impl::~Impl()
{
  delete m_c3Data;
  delete m_c3xyData;
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

void QmitkC3jsWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
}

void QmitkC3jsWidget::Impl::initializeJavaScriptChart()
{
  m_WebChannel->registerObject(QStringLiteral("chartData"), m_c3Data);
  unsigned count = 0;
  for (auto& xyData : *m_c3xyData) {
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


void QmitkC3jsWidget::AddData1D(const QList<QVariant>& data1D)
{
  m_Impl->AddData1D(data1D);
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
	this->m_Impl->GetC3Data()->SetAppearance(m_Impl->GetC3Data()->GetDiagramType(), showSubChart, m_Impl->GetC3Data()->GetDiagramType()== QVariant("pie"));

	m_Impl->initializeJavaScriptChart();
}

void QmitkC3jsWidget::ClearDiagram()
{
	m_Impl->ClearData2D();
	m_Impl->GetC3xyData()->clear();
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
