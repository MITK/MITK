/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-02-25 08:28:53 +0100 (Do, 25 Feb 2010) $
 Version:   $Revision: 21469 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkCommonWorkbenchWindowAdvisor.h"

QmitkCommonWorkbenchWindowAdvisor::QmitkCommonWorkbenchWindowAdvisor( berry::IWorkbenchWindowConfigurer::Pointer configurer )
: berry::WorkbenchWindowAdvisor(configurer)
{
  m_FunctionalityCoordinator = QmitkFunctionalityCoordinator::New();
  m_FunctionalityCoordinator->SetWindow(configurer->GetWindow());
}

void QmitkCommonWorkbenchWindowAdvisor::PostWindowCreate()
{

}