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

#ifndef _QmitkPythonSnippets_H
#define _QmitkPythonSnippets_H

#include <QWidget>
#include <QMap>
#include "mitkPythonExports.h"
struct QmitkPythonSnippetsData;

///
/// a widget that holds snippets and serializes the snippets to a certain places
class MITK_PYTHON_EXPORT QmitkPythonSnippets: public QWidget
{
  Q_OBJECT

  public:
    typedef QMap<QString, QString> QStringMap;
    ///
    /// creates default snippets
    static QStringMap CreateDefaultSnippets();
    static const QStringMap DEFAULT_SNIPPETS;

    QmitkPythonSnippets(QWidget* parent=0);
    virtual ~QmitkPythonSnippets();

    void SetFilePath( const QString& filePath );

    /*
  protected slots:
    void on_Name_currentIndexChanged(int i);
    void on_RenameSnippet_clicked();
    void on_AddNewSnippet_clicked();
    void on_RemoveSnippet_clicked();
    void on_PasteNow_clicked();
    void on_RestoreDefaultSnippets_clicked();
    void on_Content_textChanged();

  protected:
    void Update();
    void CreateUniqueName( QString& name );
    */
  private:
    QmitkPythonSnippetsData* d;
};




#endif // _QmitkPythonSnippets_H_INCLUDED

