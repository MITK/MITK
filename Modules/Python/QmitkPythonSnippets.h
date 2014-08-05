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
#include <MitkPythonExports.h>
struct QmitkPythonSnippetsData;

///
/// a widget that holds snippets and serializes the snippets to a certain places
class MITK_PYTHON_EXPORT QmitkPythonSnippets: public QWidget
{
  Q_OBJECT

  public:
    static const QString DEFAULT_SNIPPET_FILE;
    static const QString SNIPPETS_ROOT_XML_ELEMENT_NAME;
    static const QString SNIPPETS_XML_ELEMENT_NAME;
    ///
    /// typedef for string map
    typedef QMap<QString, QString> QStringMap;
    ///
    /// build ui here
    /// the snippets will be loaded from _AutoSaveFileName if not empty and readable
    /// otherwise the default snippets will be loaded
    QmitkPythonSnippets( const QString& _AutoSaveFileName="", QWidget* parent=0 );
    ///
    /// delete d pointer
    virtual ~QmitkPythonSnippets();
    ///
    /// read string map from xml file
    static bool LoadStringMap( const QString& filename, QStringMap& oldMap );

signals:
    ///
    /// this class whishes to paste sth command
    void PasteCommandRequested(const QString& command);

  protected slots:
    ///
    /// emits PasteCommandRequested signal
    void on_PasteSnippet_triggered( bool checked = false );
    ///
    /// ask for name as long as it exists, call update()
    void on_RenameSnippet_triggered( bool checked = false );
    ///
    /// ask for name, create snippet, call update()
    void on_AddSnippet_triggered( bool checked = false );
    ///
    /// remove the current snippet, call update()
    void on_RemoveSnippet_triggered( bool checked = false );
    ///
    /// call LoadStringMap with d->m_DefaultSnippetsAutoSaveFileName
    void on_RestoreDefaultSnippets_triggered( bool checked = false );
    ///
    /// update action state (enable/disable), update text box
    void on_Name_currentIndexChanged( int i );
    ///
    /// save changed snippet
    void on_Content_textChanged();
    ///
    /// ask for file, save snippets
    void on_SaveSnippets_triggered( bool checked = false );
    ///
    /// ask for file, load snippets (do not replace)
    void on_LoadSnippets_triggered( bool checked = false );

  protected:
    ///
    /// write string map to xml file
    void SaveStringMap( const QString& filename, const QStringMap& map ) const;
    ///
    /// creates a name which does not exist in the list
    QString CreateUniqueName(const QString &name) const;
    ///
    /// update combo box
    /// if name is passed, the according element will be selected
    void Update(const QString &name = "");
  private:
    ///
    /// d pointer declaration (holds members)
    QmitkPythonSnippetsData* d;
};




#endif // _QmitkPythonSnippets_H_INCLUDED

