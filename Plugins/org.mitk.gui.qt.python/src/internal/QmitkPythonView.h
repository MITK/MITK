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

#ifndef QmitkPythonView_H_
#define QmitkPythonView_H_

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
  virtual ~QmitkPythonView();

  ///
  /// \brief Standard dtor.
  ///
public slots:
  void OnVariableStackDoubleClicked( const QModelIndex & index );

protected:

  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent);

  ///
  /// focus on load image
  ///
  void SetFocus();

private:
  QmitkPythonViewData* d;
};

#endif /*QmitkPythonView_H_*/
