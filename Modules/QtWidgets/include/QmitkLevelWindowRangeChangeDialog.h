/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowRangeChangeDialog_h
#define QmitkLevelWindowRangeChangeDialog_h

#include <MitkQtWidgetsExports.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkLevelWindowRangeChange;
}

/** \brief Dialog for changing the level window range limits.
 *
 * Provides input fields for setting the lower and upper limits of the
 * level window range with input validation.
 *
 * \ingroup QmitkModule
 * \sa QmitkLevelWindowWidgetContextMenu, QmitkSliderLevelWindowWidget
 */
class MITKQTWIDGETS_EXPORT QmitkLevelWindowRangeChangeDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Construct the range change dialog.
   * \param[in] parent Parent widget.
   * \param[in] f Window flags.
   */
  QmitkLevelWindowRangeChangeDialog(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkLevelWindowRangeChangeDialog() override;

  /** \brief Get the lower range limit entered by the user.
   * \return The lower limit value.
   */
  double getLowerLimit();

  /** \brief Get the upper range limit entered by the user.
   * \return The upper limit value.
   */
  double getUpperLimit();

  /** \brief Set the initial lower range limit shown in the dialog.
   * \param[in] rangeMin The lower limit value.
   */
  void setLowerLimit(double rangeMin);

  /** \brief Set the initial upper range limit shown in the dialog.
   * \param[in] rangeMax The upper limit value.
   */
  void setUpperLimit(double rangeMax);

protected slots:

  void inputValidator();

private:
  std::unique_ptr<Ui::QmitkLevelWindowRangeChange> m_Controls;
};

#endif
