/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEditPointDialog_h
#define QmitkEditPointDialog_h

#include "MitkQtWidgetsExtExports.h"

#include <QDialog>
#include <mitkPointSet.h>

struct QmitkEditPointDialogData;

/*!
 * \brief A dialog for editing points directly (coordinates) via TextEdits
 *
 */
class MITKQTWIDGETSEXT_EXPORT QmitkEditPointDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkEditPointDialog(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkEditPointDialog() override;

  void SetPoint(mitk::PointSet *_PointSet, mitk::PointSet::PointIdentifier _PointId, int timestep = 0);

protected slots:
  void OnOkButtonClicked(bool);

protected:
  QmitkEditPointDialogData *d;
};

#endif
