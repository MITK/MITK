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

#ifndef QmitkPointListViewWidget_h
#define QmitkPointListViewWidget_h

#include <QListWidget>
#include "MitkQtWidgetsExtExports.h"

#include <mitkWeakPointer.h>
#include <mitkPointSet.h>

class QmitkStdMultiWidget;

/*!
 * \brief GUI widget for handling mitk::PointSet
 *
 * Displays all the points in a mitk::PointSet graphically.
 * Reacts automatically to changes in the PointSet's selection status.
 * Updates PointSet's selection status when this list's selection changes.
 *
 * If a QmitkStdMultiWidget is assigned via SetMultiWidget(), the
 * crosshair of the QmitkStdMultiWidget is moved to the currently selected
 * point.
 *
 */
class MitkQtWidgetsExt_EXPORT QmitkPointListViewWidget : public QListWidget
{
  Q_OBJECT

  signals:
      void PointSelectionChanged();  ///< this signal is emmitted, if the selection of a point in the pointset is changed
  public:

    QmitkPointListViewWidget( QWidget* parent = 0 );
    ~QmitkPointListViewWidget();

    /// assign a point set for observation
    void SetPointSet( mitk::PointSet* pointSet );

    /// which point set to work on
    const mitk::PointSet* GetPointSet() const;

    void SetMultiWidget( QmitkStdMultiWidget* multiWidget ); ///< assign a QmitkStdMultiWidget for updating render window crosshair

    QmitkStdMultiWidget* GetMultiWidget() const;  ///< return the QmitkStdMultiWidget that is used for updating render window crosshair

    /// which time step to display/model
    void SetTimeStep(int t);

    /// which time step to display/model
    int GetTimeStep() const;

    /// observer for point set "modified" events
    void OnPointSetChanged( const itk::Object* /*obj*/ );

    /// observer for point set "delete" events
    void OnPointSetDeleted( const itk::Object* /*obj*/ );

  protected slots:
    ///
    /// Filtering double click event for editing point coordinates via a dialog
    ///
    void OnItemDoubleClicked( QListWidgetItem * item );

    /// called when the selection of the view widget changes
    void OnCurrentRowChanged( int /*currentRow*/ );

  protected:
    void keyPressEvent( QKeyEvent * e ); ///< react to F2, F3 and DEL keys
    void MoveSelectedPointUp();
    void MoveSelectedPointDown();
    void RemoveSelectedPoint();
    void Update(bool currentRowChanged=false);

  protected:
    mitk::WeakPointer<mitk::PointSet> m_PointSet;
    int m_TimeStep;

    bool m_SelfCall;

    /// used to position the planes on a selected point
    QmitkStdMultiWidget* m_MultiWidget;
};

#endif

