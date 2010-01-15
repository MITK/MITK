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

#include "cherryNullProgressMonitor.h"
#include "cherryProgressProvider.h"

namespace cherry
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

IProgressMonitor::Pointer ProgressProvider::CreateMonitor(Job::Pointer job,
    IProgressMonitor::Pointer  /*group*/, int  /*ticks*/)
{
  //TODO SubProgressMonitor class
  // not implemented yet ( because the subProgressMonitor class is not implemented )returns a dummy null IProgressMonitor
  //Pointer
  //return IProgressMonitor::Pointer sptr_subProgressMonitor (new SubProgressMonitor()) ;
  IProgressMonitor::Pointer dummy(0);
  return dummy;
}

}
