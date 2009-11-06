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

#ifndef QMITKSTDMULTIWIDGETEDITOR_H_
#define QMITKSTDMULTIWIDGETEDITOR_H_

#include <cherryQtEditorPart.h>
#include <cherryIPartListener.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDnDFrameWidget.h>

#include "mitkQtCommonDll.h"

class MITK_QT_COMMON QmitkStdMultiWidgetEditor : public cherry::QtEditorPart, virtual public cherry::IPartListener
{
public:
  cherryObjectMacro(QmitkStdMultiWidgetEditor);

  static const std::string EDITOR_ID;

  QmitkStdMultiWidgetEditor();
  ~QmitkStdMultiWidgetEditor();

  QmitkStdMultiWidget* GetStdMultiWidget();

  void Init(cherry::IEditorSite::Pointer site, cherry::IEditorInput::Pointer input);

  void SetFocus();

  void DoSave() {}
  void DoSaveAs() {}
  bool IsDirty() const { return false; }
  bool IsSaveAsAllowed() const { return false; }

protected:

  void CreateQtPartControl(QWidget* parent);
  // IPartListener
  Events::Types GetPartEventTypes() const;
  virtual void PartClosed (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void PartHidden (cherry::IWorkbenchPartReference::Pointer partRef);
  virtual void PartVisible (cherry::IWorkbenchPartReference::Pointer partRef);

private:

  QmitkStdMultiWidget* m_StdMultiWidget;
  QmitkDnDFrameWidget* m_DndFrameWidget;
};

#endif /*QMITKSTDMULTIWIDGETEDITOR_H_*/
