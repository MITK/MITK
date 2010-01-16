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

#include <berryQtEditorPart.h>
#include <berryIPartListener.h>

#include <QmitkStdMultiWidget.h>
#include <QmitkDnDFrameWidget.h>

#include "mitkQtCommonDll.h"

class MITK_QT_COMMON QmitkStdMultiWidgetEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{
public:
  berryObjectMacro(QmitkStdMultiWidgetEditor);

  static const std::string EDITOR_ID;

  QmitkStdMultiWidgetEditor();
  ~QmitkStdMultiWidgetEditor();

  QmitkStdMultiWidget* GetStdMultiWidget();

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

  void SetFocus();

  void DoSave() {}
  void DoSaveAs() {}
  bool IsDirty() const { return false; }
  bool IsSaveAsAllowed() const { return false; }

protected:

  void CreateQtPartControl(QWidget* parent);
  // IPartListener
  Events::Types GetPartEventTypes() const;
  virtual void PartClosed (berry::IWorkbenchPartReference::Pointer partRef);
  virtual void PartHidden (berry::IWorkbenchPartReference::Pointer partRef);
  virtual void PartVisible (berry::IWorkbenchPartReference::Pointer partRef);

private:

  QmitkStdMultiWidget* m_StdMultiWidget;
  QmitkDnDFrameWidget* m_DndFrameWidget;
};

#endif /*QMITKSTDMULTIWIDGETEDITOR_H_*/
