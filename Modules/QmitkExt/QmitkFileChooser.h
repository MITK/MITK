/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-28 18:32:03 +0100 (Do, 28 Jan 2010) $
Version:   $Revision: 1.0 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkFileChooser_h_Included
#define QmitkFileChooser_h_Included

#include <QWidget>
#include "QmitkExtExports.h"

class QPushButton;
class QLineEdit;

///
/// \brief Widget showing a line edit with the path of
/// a file or directory and a button which invokes a file choose dialog
///
class QmitkExt_EXPORT QmitkFileChooser : public QWidget
{
  Q_OBJECT

  public:
    explicit QmitkFileChooser( bool selectDir = false,
      QWidget* parent = 0, Qt::WindowFlags f = 0);
  
    void SetSelectDir( bool selectDir );
    void SetFile( const std::string& file );
    void SetFilePattern( const std::string& filepattern );

    bool IsValidFile() const;
    std::string GetFile() const;

  protected slots:
    void OnSelectFileClicked ( bool checked=false );

  protected:
    bool m_SelectDir;
    bool m_ValidFile;

    QString m_FilePattern;
    QPushButton* m_SelectFile;
    QLineEdit* m_File;
};

#endif

