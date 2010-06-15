/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-16 19:57:43 +0100 (Sa, 16 Jan 2010) $
Version:   $Revision: 21070 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkCommonWorkbenchWindowAdvisor_h
#define QmitkCommonWorkbenchWindowAdvisor_h

#include "mitkQtCommonExtDll.h"

#include <berryWorkbenchWindowAdvisor.h>
#include <QmitkFunctionalityCoordinator.h>

///
/// \brief a WorkbenchWindowAdvisor that does common stuff when opening a MITK Workbench (e.g adding a
/// QmitkFunctionalityCoordinator instance)
/// \see QmitkFunctionalityCoordinator
///
class MITK_QT_COMMON_EXT_EXPORT QmitkCommonWorkbenchWindowAdvisor : public berry::WorkbenchWindowAdvisor
{
public:
  QmitkCommonWorkbenchWindowAdvisor( berry::IWorkbenchWindowConfigurer::Pointer configurer );
  void PostWindowCreate();

private:
  QmitkFunctionalityCoordinator::Pointer m_FunctionalityCoordinator;

};

#endif /*QMITKEXTWORKBENCHWINDOWADVISOR_H_*/
