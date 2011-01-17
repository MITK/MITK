/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKPYTHONSCRIPTEDITORHIGHLIGHTER_H_
#define QMITKPYTHONSCRIPTEDITORHIGHLIGHTER_H_

#include <QTextEdit>
#include <QSyntaxHighlighter>

class QmitkPythonScriptEditorHighlighter : public QSyntaxHighlighter
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkPythonScriptEditorHighlighter(QTextDocument *parent);
  virtual ~QmitkPythonScriptEditorHighlighter();

signals:

protected slots:  

protected:
  virtual void highlightBlock(const QString &text);
  void highlightComments(const QString &text);

private:
};

#endif
