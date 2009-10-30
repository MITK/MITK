/*=========================================================================
 
 Program:   openCherry Platform
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

#include "cherrySaveableMockViewPart.h"

namespace cherry
{

const std::string SaveableMockViewPart::ID =
    "org.opencherry.ui.tests.api.SaveableMockViewPart";

SaveableMockViewPart::SaveableMockViewPart() :
  isDirty(false), saveAsAllowed(false), saveNeeded(true)
{

}

void SaveableMockViewPart::CreatePartControl(void* parent)
{
  MockViewPart::CreatePartControl(parent);

  //        final Button dirtyToggle = new Button(parent, SWT.CHECK);
  //        dirtyToggle.setText("Dirty");
  //        dirtyToggle.addSelectionListener(new SelectionAdapter() {
  //            public void widgetSelected(SelectionEvent e) {
  //                setDirty(dirtyToggle.getSelection());
  //            }
  //        });
  //        dirtyToggle.setSelection(isDirty());
  //
  //        final Button saveNeededToggle = new Button(parent, SWT.CHECK);
  //        saveNeededToggle.setText("Save on close");
  //        saveNeededToggle.addSelectionListener(new SelectionAdapter() {
  //            public void widgetSelected(SelectionEvent e) {
  //                setSaveNeeded(saveNeededToggle.getSelection());
  //            }
  //        });
  //        saveNeededToggle.setSelection(saveNeeded);
  //
  //        final Button saveAsToggle = new Button(parent, SWT.CHECK);
  //        saveAsToggle.setText("Save as allowed");
  //        saveAsToggle.addSelectionListener(new SelectionAdapter() {
  //            public void widgetSelected(SelectionEvent e) {
  //                setSaveAsAllowed(saveAsToggle.getSelection());
  //            }
  //        });
  //        saveAsToggle.setSelection(saveAsAllowed);
}

void SaveableMockViewPart::DoSave(/*IProgressMonitor monitor*/)
{
  callTrace->Add("DoSave");
}

void SaveableMockViewPart::DoSaveAs()
{
  callTrace->Add("DoSaveAs");
}

bool SaveableMockViewPart::IsDirty() const
{
  callTrace->Add("IsDirty");
  return isDirty;
}

bool SaveableMockViewPart::IsSaveAsAllowed() const
{
  callTrace->Add("IsSaveAsAllowed");
  return saveAsAllowed;
}

bool SaveableMockViewPart::IsSaveOnCloseNeeded() const
{
  callTrace->Add("IsSaveOnCloseNeeded");
  return saveNeeded;
}

void SaveableMockViewPart::SetDirty(bool isDirty)
{
  this->isDirty = isDirty;
  FirePropertyChange(ISaveablePart::PROP_DIRTY);
}

void SaveableMockViewPart::SetSaveAsAllowed(bool isSaveAsAllowed)
{
  this->saveAsAllowed = isSaveAsAllowed;
}

void SaveableMockViewPart::SetSaveNeeded(bool isSaveOnCloseNeeded)
{
  this->saveNeeded = isSaveOnCloseNeeded;
}

}
