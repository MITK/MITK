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


#ifndef QmitkDicomEditor_h
#define QmitkDicomEditor_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkDicomEditorControls.h"

/*!
  \brief QmitkDicomEditor

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class QmitkDicomEditor : public QmitkFunctionality, public berry::QtEditorPart, virtual public berry::IPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string EDITOR_ID;

    QmitkDicomEditor();
    virtual ~QmitkDicomEditor();

    virtual void CreateQtPartControl(QWidget *parent);

  public slots:
    void openImportDialog();
    void openQueryDialog();

  protected slots:

    /// \brief Called when series in TreeView is double clicked.
    void onSeriesModelSelected(QModelIndex index);

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::QmitkDicomEditorControls m_Controls;

};

#endif // QmitkDicomEditor_h

