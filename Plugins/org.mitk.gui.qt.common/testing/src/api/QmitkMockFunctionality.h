/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKMOCKFUNCTIONALITY_H_
#define QMITKMOCKFUNCTIONALITY_H_

#include <QmitkFunctionality.h>

#include <util/berryCallHistory.h>

class QmitkMockFunctionality : public QmitkFunctionality
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkMockFunctionality);

  static const std::string ID; // = "org.mitk.gui.qt.common.tests.api.MockFunctionality";

  QmitkMockFunctionality();
  QmitkMockFunctionality(const QmitkMockFunctionality& other);

  ~QmitkMockFunctionality();

  berry::CallHistory::Pointer GetCallHistory() const;

  void SetFocus();

protected:

  void CreateQtPartControl(QWidget* parent);

private:

  berry::CallHistory::Pointer callTrace;
};

#endif /* QMITKMOCKFUNCTIONALITY_H_ */
