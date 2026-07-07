/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAboutDialog_h
#define QmitkAboutDialog_h

#include <MitkQtWidgetsExtExports.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkAboutDialog;
}

/**
 * \brief Dialog displaying information about the MITK application.
 *
 * Shows the application name, revision, and toolkit versions (ITK, VTK, Qt).
 * The about text, caption, and revision labels can be customized.
 */
class MITKQTWIDGETSEXT_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the about dialog.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags for the dialog.
   */
  QmitkAboutDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

  /** \brief Destructor. */
  ~QmitkAboutDialog() override;

  /**
   * \brief Get the current about text (HTML).
   * \return The about label text.
   */
  QString GetAboutText() const;

  /**
   * \brief Get the current caption text.
   * \return The caption label text.
   */
  QString GetCaptionText() const;

  /**
   * \brief Get the current revision text (HTML).
   * \return The revision label text including the Git commit link.
   */
  QString GetRevisionText() const;

  /**
   * \brief Set the about text displayed in the dialog.
   * \param[in] text The new about text (may contain HTML).
   */
  void SetAboutText(const QString &text);

  /**
   * \brief Set the caption text displayed in the dialog.
   * \param[in] text The new caption text.
   */
  void SetCaptionText(const QString &text);

  /**
   * \brief Set the revision text displayed in the dialog.
   * \param[in] text The new revision text (may contain HTML).
   */
  void SetRevisionText(const QString &text);

private:
  std::unique_ptr<Ui::QmitkAboutDialog> m_GUI;
};

#endif
