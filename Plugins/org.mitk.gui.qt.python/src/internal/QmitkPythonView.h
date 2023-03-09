/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonView_h
#define QmitkPythonView_h

/// Qmitk
#include <QmitkAbstractView.h>

///
/// d pointer forward declaration
///
struct QmitkPythonViewData;

///
/// \brief New python view (CONSOLE)
///
class QmitkPythonView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID; // = "org.mitk.extapp.defaultperspective"
  ///
  /// \brief Standard ctor.
  ///
  QmitkPythonView();

  ///
  /// \brief Standard dtor.
  ///
  ~QmitkPythonView() override;

protected:

  ///
  /// \brief Create the view here.
  ///
  void CreateQtPartControl(QWidget* parent) override;

  ///
  /// focus on load image
  ///
  void SetFocus() override;

private:
  QmitkPythonViewData* d;
};

#endif
