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
#ifndef QmitkD3jsWidget_h
#define QmitkD3jsWidget_h

#include <QWidget>

#include <MitkC3jsExports.h>

class MITKC3JS_EXPORT QmitkC3jsWidget : public QWidget
{
  Q_OBJECT

public:
	enum class DiagramType { bar, line, spline, pie};

  explicit QmitkC3jsWidget(QWidget* parent = nullptr);
  QmitkC3jsWidget(const QString& id, QObject* object, QWidget* parent = nullptr);
  virtual ~QmitkC3jsWidget();

  void AddData2D(const QMap<QVariant, QVariant>& data2D);
  QList<QMap<QVariant, QVariant> > GetData2D() const;

  void AddData1D(const QList<QVariant>& data1D);
  QList<QList<QVariant> > GetData1D() const;

  void SetDataLabels(const QList<QString>& labels);
  QList<QString> GetDataLabels() const;

  void SetXAxisLabel(const QString& label);
  QString GetXAxisLabel() const;

  void SetYAxisLabel(const QString& label);
  QString GetYAxisLabel() const;

  void SetDiagramType(DiagramType type);
  DiagramType GetDiagramType() const;

  void ShowDiagram(bool showSubChart=false);

  void ClearDiagram();

  void SendCommand(QString command);
  
  void TransformView(QString transformTo);


private:
  class Impl;
  Impl* m_Impl;

public slots:
  void OnLoadFinished(bool isLoadSuccessfull);

signals:
  void PageSuccessfullyLoaded();
};

#endif
