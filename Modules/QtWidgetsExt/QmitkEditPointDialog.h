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
class MitkQtWidgetsExt_EXPORT QmitkEditPointDialog : public QDialog
{
  Q_OBJECT

  public:
    QmitkEditPointDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    virtual ~QmitkEditPointDialog();

    void SetPoint( mitk::PointSet* _PointSet, mitk::PointSet::PointIdentifier _PointId, int timestep=0 );

  protected slots:
    void OnOkButtonClicked(bool);

  protected:
    QmitkEditPointDialogData* d;
};

#endif

