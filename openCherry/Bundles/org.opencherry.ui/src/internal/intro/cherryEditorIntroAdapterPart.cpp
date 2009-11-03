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

#include "cherryEditorIntroAdapterPart.h"

#include "cherryIntroPartAdapterSite.h"

#include "../cherryWorkbench.h"
#include "../cherryWorkbenchPlugin.h"

namespace cherry
{

EditorIntroAdapterPart::EditorIntroAdapterPart() :
  propChangeListener(new PropertyChangeIntAdapter<EditorIntroAdapterPart>(this, &EditorIntroAdapterPart::PropertyChange))
{

}

void EditorIntroAdapterPart::SetStandby(bool standby)
{
  //        final Control control = ((PartSite) getSite()).getPane().getControl();
  //        BusyIndicator.showWhile(control.getDisplay(), new Runnable() {
  //            public void run() {
  //                try {
  //                    control.setRedraw(false);
  introPart->StandbyStateChanged(standby);
  //                } finally {
  //                    control.setRedraw(true);
  //                }
  //
  //                setBarVisibility(standby);
  //            }
  //        });
}

void EditorIntroAdapterPart::CreatePartControl(void* parent)
{
  //addPaneListener();
  introPart->CreatePartControl(parent);
}

EditorIntroAdapterPart::~EditorIntroAdapterPart()
{
  //setBarVisibility(true);
  introPart->RemovePropertyListener(propChangeListener);
  GetSite()->GetWorkbenchWindow()->GetWorkbench()->GetIntroManager()->CloseIntro(
      introPart);
}

void* EditorIntroAdapterPart::GetTitleImage()
{
  return introPart->GetTitleImage();
}

std::string EditorIntroAdapterPart::GetPartName()
{
  // this method is called eagerly before our init method is called (and
  // therefore before our intropart is created).  By default return
  // the view title from the view declaration.  We will fire a property
  // change to set the title to the proper value in the init method.
  return introPart.IsNull() ? EditorPart::GetPartName() : introPart->GetPartName();
}

void EditorIntroAdapterPart::Init(IEditorSite::Pointer site,
    IEditorInput::Pointer input)
{
  Workbench* workbench =
      dynamic_cast<Workbench*>(site->GetWorkbenchWindow()->GetWorkbench());
  try
  {
    introPart = workbench->GetWorkbenchIntroManager()->CreateNewIntroPart();
    // reset the part name of this view to be that of the intro title
    SetPartName(introPart->GetPartName());
    introPart->AddPropertyListener(propChangeListener);
    introSite
        = IIntroSite::Pointer(new IntroPartAdapterSite(site, workbench->GetIntroDescriptor()));
    introPart->Init(introSite, IMemento::Pointer(0));
  }
  catch (CoreException& e)
  {
    //TODO IStatus
    //            WorkbenchPlugin.log(
    //                            IntroMessages.Intro_could_not_create_proxy,
    //                            new Status(IStatus.ERROR, WorkbenchPlugin.PI_WORKBENCH,
    //                                IStatus.ERROR, IntroMessages.Intro_could_not_create_proxy, e));
    WorkbenchPlugin::Log("Could not create intro editor proxy.", e);
  }

  this->SetSite(site);
  this->SetInput(input);
}

void EditorIntroAdapterPart::DoSave(/*IProgressMonitor monitor*/)
{

}

void EditorIntroAdapterPart::DoSaveAs()
{

}

bool EditorIntroAdapterPart::IsDirty() const
{
  return false;
}

bool EditorIntroAdapterPart::IsSaveAsAllowed() const
{
  return false;
}

void EditorIntroAdapterPart::PropertyChange(Object::Pointer /*source*/,
    int propId)
{
  FirePropertyChange(propId);
}

void EditorIntroAdapterPart::SetFocus()
{
  introPart->SetFocus();
}

}
