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

#ifndef QmitkComboBoxStepThrough_H
#define QmitkComboBoxStepThrough_H

#include <QComboBox>

class QmitkComboBoxStepThrough : public QComboBox
{
  Q_OBJECT

signals:
  void SignalReachedEnd(bool);
  void SignalReachedBegin(bool);

public slots:
  void OnSetPreviousIndex();
  void OnSetNextIndex();

private slots:
  void OnCurrentIndexChanged(int);

public:
  QmitkComboBoxStepThrough(QWidget* parent = 0);
  ~QmitkComboBoxStepThrough();

private:
  int m_LastIndex;
};

#endif // QmitkComboBoxStepThrough_H
