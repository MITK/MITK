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
#ifndef QmitkC3jsWidget_h
#define QmitkC3jsWidget_h

#include <QWidget>

#include <MitkC3jsExports.h>


/*!
\brief QmitkC3jsWidget is a widget to display various charts. Currently, bar charts, line charts and pie charts are supported (\sa DiagramType).
* Data is added via AddData1D() or AddData2D().
* Multiple adding of data leads to multiple charts (of the same type)
\ingroup Modules/C3js
*/
class MITKC3JS_EXPORT QmitkC3jsWidget : public QWidget
{
  Q_OBJECT

public:
  /*!
  * \brief enum of diagram types. Supported are bar, line, spline (a smoothed line) and pie.
  * Pie is significantly different than the other types. Here, the data given by AddData1D is summed. Each entry represents a different category.
  */
	enum class DiagramType { bar, line, spline, pie};

  explicit QmitkC3jsWidget(QWidget* parent = nullptr);
  QmitkC3jsWidget(const QString& id, QObject* object, QWidget* parent = nullptr);
  virtual ~QmitkC3jsWidget();

  /*!
  * \brief Adds 2D data to the widget. Call repeatedly for displaying multiple charts.
  * \details each entry represents a data point: key: value --> x-value: y-value.

  * \sa ClearDiagram() for Clearing the data
  */
  void AddData2D(const QMap<QVariant, QVariant>& data2D);

  /*!
  * \brief Adds 1D data to the widget
  * \details internally, the list is converted to a map with increasing integers keys starting at 0.
  * \sa ClearDiagram() for Clearing the data
  */
  void AddData1D(const QList<QVariant>& data1D);

  void SetDataLabels(const QList<QString>& labels);
  QList<QString> GetDataLabels() const;

  void SetXAxisLabel(const QString& label);
  QString GetXAxisLabel() const;

  void SetYAxisLabel(const QString& label);
  QString GetYAxisLabel() const;

  /*!
  * \sa DiagramType for available types
  */
  void SetDiagramType(DiagramType type);
  DiagramType GetDiagramType() const;

  /*!
  * \brief Shows the diagram
  * \param showSubChart if a subchart is displayed inside the widget or not.
  */
  void ShowDiagram(bool showSubChart=false);

  /*!
  * \brief Clears all data inside and resets the widget.
  */
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
