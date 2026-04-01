/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEditPointDialog_h
#define QmitkEditPointDialog_h

#include <MitkQtWidgetsExtExports.h>

#include <QDialog>
#include <mitkPointSet.h>

struct QmitkEditPointDialogData;

/**
 * \brief A dialog for editing 3D point coordinates via text input fields.
 *
 * Displays X, Y, and Z coordinate line edits for a specific point in a
 * mitk::PointSet. On confirmation, the new coordinates are validated for
 * double precision and written back to the point set.
 *
 * \sa QmitkPointListView, QmitkPointListWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkEditPointDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the edit point dialog.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkEditPointDialog(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkEditPointDialog() override;

  /**
   * \brief Set the point to edit.
   *
   * Populates the coordinate fields with the current values of the specified point.
   *
   * \param[in] _PointSet The point set containing the point.
   * \param[in] _PointId The identifier of the point within the set.
   * \param[in] timestep The time step at which to read/write the point (default 0).
   */
  void SetPoint(mitk::PointSet *_PointSet, mitk::PointSet::PointIdentifier _PointId, int timestep = 0);

protected slots:
  void OnOkButtonClicked(bool);

protected:
  QmitkEditPointDialogData *d;
};

#endif
