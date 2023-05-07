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
#include <qdialog.h>
/**
  \brief Displays read-only QTextEdit.

  For output of any kind of information that might be copied into other applications.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkCopyToClipBoardDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkCopyToClipBoardDialog(const QString &text, QWidget *parent = nullptr, const char *name = nullptr);
  ~QmitkCopyToClipBoardDialog() override;

signals:

public slots:

protected slots:

protected:
};

#endif
