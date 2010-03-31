/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkEditPointDialog_h
#define QmitkEditPointDialog_h

#include "QmitkExtExports.h"

#include <QDialog>
#include <mitkPointSet.h>

struct QmitkEditPointDialogData;
/*!
 * \brief A dialog for editing points directly (coordinates) via TextEdits
 *
 */
class QmitkExt_EXPORT QmitkEditPointDialog : public QDialog
{
  Q_OBJECT

  public:
    QmitkEditPointDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    virtual ~QmitkEditPointDialog();

    void SetPoint( mitk::PointSet* _PointSet, mitk::PointSet::PointIdentifier _PointId, int timestep=0 );
  protected slots:
    void OnOkButtonClicked( bool triggered = false );
  protected:
    QmitkEditPointDialogData* d;
};

#endif // QmitkEditPointDialog_h