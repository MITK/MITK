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

#ifndef _QMITKTUBEGRAPHLABELWIDGET_H_INCLUDED
#define _QMITKTUBEGRAPHLABELWIDGET_H_INCLUDED

#include "mitkColorProperty.h"

#include <QCheckBox>
#include <QPushButton>

class QmitkTubeGraphLabelWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkTubeGraphLabelWidget(QWidget* parent = nullptr);
  //QmitkTubeGraphLabelWidget(QWidget* parent = 0, const char* name = 0, mitk::Color* color = 0 /*TODO*/);
  virtual ~QmitkTubeGraphLabelWidget();

  void SetLabelName(QString name);
  QString GetLabelName();
  void SetLabelColor(mitk::Color* color);
  mitk::Color* GetLabelColor();

signals:

  void SignalLabelVisibilityToggled(bool, QString);
  void SignalLabelButtonClicked(QString);
  void SignalLabelColorChanged(mitk::Color, QString);

  protected slots:

    void OnVisibilityToggled(bool isVisible);
    void OnLabelButtonClicked();
    void OnColoringButtonClicked();


private:

  void InitWidget();

  QCheckBox* m_VisibilityCheckBox;
  QPushButton* m_LabelPushButton;
  QPushButton* m_ColoringPushButton;

};
#endif
