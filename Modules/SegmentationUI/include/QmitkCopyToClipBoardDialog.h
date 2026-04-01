/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCopyToClipBoardDialog_h
#define QmitkCopyToClipBoardDialog_h

#include <MitkSegmentationUIExports.h>
#include <mitkCommon.h>
#include <QDialog>
/**
 * \brief Dialog that displays a read-only text area for copying content to the clipboard.
 *
 * Shows a QTextEdit in read-only mode, suitable for displaying any kind of information
 * (e.g., statistics, measurements) that the user may want to copy into other applications.
 *
 * \sa QmitkImageStatisticsWidget
 */
class MITKSEGMENTATIONUI_EXPORT QmitkCopyToClipBoardDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the dialog with the given text content.
   * \param[in] text The text to display in the read-only text area.
   * \param[in] parent Optional parent widget.
   * \param[in] name Optional object name for the dialog.
   */
  QmitkCopyToClipBoardDialog(const QString &text, QWidget *parent = nullptr, const char *name = nullptr);

  /** \brief Destructor. */
  ~QmitkCopyToClipBoardDialog() override;

signals:

public slots:

protected slots:

protected:
};

#endif
