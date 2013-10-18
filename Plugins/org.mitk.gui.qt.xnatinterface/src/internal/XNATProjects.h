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


#ifndef XNATProjects_h
#define XNATProjects_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_XNATProjectsControls.h"

#include "Poco/DOM/Node.h"
#include "Poco/URI.h"

#include <QScopedPointer>
#include <QStandardItemModel>

/*!
\brief XNATProjects

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class XNATProjects : public QmitkAbstractView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  
  QScopedPointer<QStandardItemModel> projectModel;
  QScopedPointer<QStandardItemModel> subjectModel;
  QScopedPointer<QStandardItemModel> experimentModel;

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  void SearchWithXmlFilePath(QString filepath);

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void GetAllProjects();

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes );

  Ui::XNATProjectsControls m_Controls;


private:
  
  QString ConvertFromXMLString(const Poco::XML::XMLString);

};

#endif // XNATProjects_h

