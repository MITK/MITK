/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNullProgressMonitor.h"
#include "berryProgressProvider.h"

namespace berry
{

// returns the default NullProgressProvider
IProgressMonitor::Pointer ProgressProvider::CreateProgressGroup()
{
  IProgressMonitor::Pointer sptr_progressGroup(new NullProgressMonitor());
  return sptr_progressGroup;
}

// returns the default NullProgressProvider
IProgressMonitor::Pointer ProgressProvider::GetDefaultMonitor()
{
  IProgressMonitor::Pointer sptr_defaultMonitor(new NullProgressMonitor());
  return sptr_defaultMonitor;
}

IProgressMonitor::Pointer ProgressProvider::CreateMonitor(Job::Pointer /*job*/,
    IProgressMonitor::Pointer  /*group*/, int  /*ticks*/)
{
  //TODO SubProgressMonitor class
  // not implemented yet ( because the subProgressMonitor class is not implemented )returns a dummy null IProgressMonitor
  //Pointer
  //return IProgressMonitor::Pointer sptr_subProgressMonitor (new SubProgressMonitor()) ;
  IProgressMonitor::Pointer dummy(nullptr);
  return dummy;
}

}
