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

#include "QmitkPythonScriptEditorHighlighter.h"

QmitkPythonScriptEditorHighlighter::QmitkPythonScriptEditorHighlighter(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
}

QmitkPythonScriptEditorHighlighter::~QmitkPythonScriptEditorHighlighter()
{
}

void QmitkPythonScriptEditorHighlighter::highlightBlock(const QString &text)
{
  QTextCharFormat commentFormat;
  commentFormat.setFontWeight(QFont::Courier);
  commentFormat.setForeground(Qt::darkGreen);

  QTextCharFormat keywordFormat;
  keywordFormat.setFontWeight(QFont::Bold);
  keywordFormat.setForeground(Qt::blue);
  QString pattern = "\\b(and|assert|break|class|continue|def|del|elif|else|except|exec|finally|for|from|global|if|import|in|is|lambda|not|or|pass|print|raise|return|try|while|yield|None|True|False)\\b";

  QTextCharFormat qouteFormat;
  qouteFormat.setForeground(Qt::gray);

  QTextCharFormat numberFormat;
  numberFormat.setForeground(Qt::red);

  QRegExp numberExpression("\\d+");
  int indexNumbers = text.indexOf(numberExpression);

  while (indexNumbers >= 0) {
    int length = numberExpression.matchedLength();
    setFormat(indexNumbers, length, numberFormat);
    indexNumbers = text.indexOf(numberExpression, indexNumbers + length);
  }

  QRegExp qouteExpression("\"");
  int startQouteIndex = text.indexOf(qouteExpression);
  int endQouteIndex = text.indexOf(qouteExpression, startQouteIndex);

  QRegExp keyword(pattern);
  int indexKeyword = text.indexOf(keyword);

  QRegExp startCommentExpression("^#");
  QRegExp endCommentExpression("\\n");
  int startCommentIndex = text.indexOf(startCommentExpression);
  int endCommentIndex = text.indexOf(endCommentExpression, startCommentIndex);


  while (startQouteIndex >= 0) {
      endQouteIndex = text.indexOf(qouteExpression, startQouteIndex+1);
      int commentLength;
      if (endQouteIndex == -1) {
          setCurrentBlockState(1);
          commentLength = text.length() - startQouteIndex;
      } else {
          commentLength = endQouteIndex - startQouteIndex
                          + qouteExpression.matchedLength();
      }
      setFormat(startQouteIndex, commentLength, qouteFormat);
      startQouteIndex = text.indexOf(qouteExpression,
                                startQouteIndex + commentLength);
    }

  if(startCommentIndex < 0)
  {
    while (indexKeyword >= 0) {
      int length = keyword.matchedLength();
      setFormat(indexKeyword, length, keywordFormat);
      indexKeyword = text.indexOf(keyword, indexKeyword + length);
    }
  }

  while (startCommentIndex >= 0) {
    endCommentIndex = text.indexOf(endCommentExpression, startCommentIndex);
    int commentLength;
    if (endCommentIndex == -1) {
        setCurrentBlockState(1);
        commentLength = text.length() - startCommentIndex;
    } else {
        commentLength = endCommentIndex - startCommentIndex
                        + endCommentExpression.matchedLength();
    }
    setFormat(startCommentIndex, commentLength, commentFormat);
    startCommentIndex = text.indexOf(startCommentExpression,
                              startCommentIndex + commentLength);
  }
}

void QmitkPythonScriptEditorHighlighter::highlightComments(const QString &text)
{
}

