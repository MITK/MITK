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

#ifndef QmitkDataManagerLightView_H_
#define QmitkDataManagerLightView_H_

/// Qmitk
#include <QmitkAbstractView.h>

struct QmitkDataManagerLightViewData;

///
/// \brief Data management view with reduced functions ("light")
///
class QmitkDataManagerLightView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID; // = "org.mitk.extapp.defaultperspective"
  ///
  /// \brief Standard ctor.
  ///
  QmitkDataManagerLightView();

  ///
  /// \brief Standard dtor.
  ///
  virtual ~QmitkDataManagerLightView();

protected:

  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent);

  ///
  ///
  ///
  void SetFocus();

private:
  QmitkDataManagerLightViewData* d;
};

#endif /*QmitkDataManagerLightView_H_*/
