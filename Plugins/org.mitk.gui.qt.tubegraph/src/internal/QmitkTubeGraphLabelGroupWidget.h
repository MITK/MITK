/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTubeGraphLabelGroupWidget_h
#define QmitkTubeGraphLabelGroupWidget_h

#include "mitkColorProperty.h"

#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class QmitkTubeGraphLabelGroupWidget : public QWidget
{
    Q_OBJECT

public:
    QmitkTubeGraphLabelGroupWidget(QWidget *parent = nullptr, const char* name = nullptr);
    ~QmitkTubeGraphLabelGroupWidget() override;

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
