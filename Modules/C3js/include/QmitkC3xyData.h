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

#ifndef QmitkC3xyData_h
#define QmitkC3xyData_h

#include <QVariant>

/** /brief This class holds the actual data for the chart generation with C3.
* data can be loaded in constructor directly or with SetData
* It is derived from QObject, because we need Q_PROPERTIES to send Data via QWebChannel to JavaScript.
*/
class QmitkC3xyData : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QList<QVariant> m_YData READ GetYData WRITE SetYData NOTIFY SignalYDataChanged);
  Q_PROPERTY(QList<QVariant> m_XData READ GetXData WRITE SetXData NOTIFY SignalXDataChanged);

public:
  QmitkC3xyData();
  QmitkC3xyData(QMap<QVariant, QVariant> data);

  void SetData(QMap<QVariant, QVariant> data);

  Q_INVOKABLE QList<QVariant> GetYData() { return m_YData; };
  Q_INVOKABLE void SetYData(QList<QVariant> yData) { m_YData =yData; };

  Q_INVOKABLE QList<QVariant> GetXData() { return m_XData; };
  Q_INVOKABLE void SetXData(QList<QVariant> xData) { m_XData =xData; };

  
  /**
  * \brief Clears the Data.
  *
  * This function clears the data.
  */
  void ClearData();

signals:
  void SignalYDataChanged(const QList<QVariant> yData);
  void SignalXDataChanged(const QList<QVariant> xData);

private:

  QList<QVariant>  m_YData;
  QList<QVariant>  m_XData;
};

#endif  //QmitkC3xyData_h