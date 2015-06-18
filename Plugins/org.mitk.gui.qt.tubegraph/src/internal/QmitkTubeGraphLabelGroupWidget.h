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

#ifndef _QMITKLABELGROUPWIDGET_H_INCLUDED
#define _QMITKLABELGROUPWIDGET_H_INCLUDED

#include "mitkColorProperty.h"

#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class QmitkTubeGraphLabelGroupWidget : public QWidget
{
    Q_OBJECT

public:
    QmitkTubeGraphLabelGroupWidget(QWidget *parent = nullptr, const char* name = nullptr);
    virtual ~QmitkTubeGraphLabelGroupWidget();

    void SetGroupName(QString name);
    QString GetGroupName();
    void AddLabel(QString name, mitk::Color color);

signals:
  void SignalLabelVisibilityInGroupToggled(bool, QString, QString);
  void SignalLabelButtonInGroupClicked(QString, QString);
  void SignalLabelColorInGroupChanged(mitk::Color, QString, QString);

  protected slots:

    void OnVisibilityToggled(bool isVisible, QString labelName);
    void OnLabelButtonClicked(QString labelName);
   void OnColoringButtonClicked(mitk::Color color, QString labelName);

private:

  QString                            m_GroupName;
  QVBoxLayout*                       m_GroupLayout;
  //QVector<QmitkTubeGraphLabelWidget*> m_Labels;

};

#endif
