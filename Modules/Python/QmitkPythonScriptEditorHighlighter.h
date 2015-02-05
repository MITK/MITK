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

#ifndef QmitkPythonScriptEditorHighlighter_h
#define QmitkPythonScriptEditorHighlighter_h

#include <QSyntaxHighlighter>
#include <MitkPythonExports.h>

///
/// A script highlighter for Python Scripts
class MITKPYTHON_EXPORT QmitkPythonScriptEditorHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  QmitkPythonScriptEditorHighlighter(QTextDocument *parent);
  virtual ~QmitkPythonScriptEditorHighlighter();

protected:
  void highlightBlock(const QString &text);
  void highlightComments(const QString &text);

private:
};

#endif // QmitkPythonScriptEditorHighlighter_h
