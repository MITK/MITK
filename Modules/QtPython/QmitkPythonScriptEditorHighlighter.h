/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonScriptEditorHighlighter_h
#define QmitkPythonScriptEditorHighlighter_h

#include <QSyntaxHighlighter>
#include <MitkQtPythonExports.h>

///
/// A script highlighter for Python Scripts
class MITKQTPYTHON_EXPORT QmitkPythonScriptEditorHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  QmitkPythonScriptEditorHighlighter(QTextDocument *parent);
  ~QmitkPythonScriptEditorHighlighter() override;

protected:
  void highlightBlock(const QString &text) override;
  void highlightComments(const QString &text);

private:
};

#endif
