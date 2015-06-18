/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryViewIntroAdapterPart.h"

#include "berryIntroPartAdapterSite.h"

#include "internal/berryWorkbench.h"
#include "internal/berryWorkbenchPlugin.h"

namespace berry
{

ViewIntroAdapterPart::ViewIntroAdapterPart() :
  propChangeListener(new PropertyChangeIntAdapter<ViewIntroAdapterPart>(this, &ViewIntroAdapterPart::PropertyChange))
{

}

void ViewIntroAdapterPart::SetStandby(bool standby)
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

void ViewIntroAdapterPart::CreatePartControl(QWidget* parent)
{
  //addPaneListener();
  introPart->CreatePartControl(parent);
}

ViewIntroAdapterPart::~ViewIntroAdapterPart()
{
  //setBarVisibility(true);
  introPart->RemovePropertyListener(propChangeListener.data());
  GetSite()->GetWorkbenchWindow()->GetWorkbench()->GetIntroManager()->CloseIntro(
      introPart);
}

QIcon ViewIntroAdapterPart::GetTitleImage() const
{
  return introPart->GetTitleImage();
}

QString ViewIntroAdapterPart::GetPartName() const
{
  // this method is called eagerly before our init method is called (and
  // therefore before our intropart is created).  By default return
  // the view title from the view declaration.  We will fire a property
  // change to set the title to the proper value in the init method.
  return introPart.IsNull() ? ViewPart::GetPartName() : introPart->GetPartName();
}

void ViewIntroAdapterPart::Init(IViewSite::Pointer site,
    IMemento::Pointer memento) throw (PartInitException)
{
  ViewPart::Init(site);
  Workbench* workbench =
      dynamic_cast<Workbench*>(site->GetWorkbenchWindow()->GetWorkbench());
  try
  {
    introPart = workbench->GetWorkbenchIntroManager() ->CreateNewIntroPart();
    // reset the part name of this view to be that of the intro title
    SetPartName(introPart->GetPartName());
    introPart->AddPropertyListener(propChangeListener.data());
    introSite
        = IIntroSite::Pointer(new IntroPartAdapterSite(site, workbench->GetIntroDescriptor()));
    introPart->Init(introSite, memento);

  } catch (CoreException& e)
  {
    //TODO IStatus
    //            WorkbenchPlugin.log(
    //                            IntroMessages.Intro_could_not_create_proxy,
    //                            new Status(IStatus.ERROR, WorkbenchPlugin.PI_WORKBENCH,
    //                                IStatus.ERROR, IntroMessages.Intro_could_not_create_proxy, e));
    WorkbenchPlugin::Log("Could not create intro view proxy.", e);
  }
}

void ViewIntroAdapterPart::PropertyChange(const Object::Pointer& /*source*/,
    int propId)
{
  FirePropertyChange(propId);
}

void ViewIntroAdapterPart::SetFocus()
{
  introPart->SetFocus();
}

void ViewIntroAdapterPart::SaveState(IMemento::Pointer memento)
{
  introPart->SaveState(memento);
}

}
