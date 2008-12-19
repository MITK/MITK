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

#ifndef QMITKVIEWPART_H_
#define QMITKVIEWPART_H_

#include <cherryQtViewPart.h>

#include <mitkDataStorage.h>

#include "mitkQtCommonDll.h"

//!mm-added
class QmitkStdMultiWidget;

class MITK_QT_COMMON QmitkViewPart : public cherry::QtViewPart
{

public:
  cherryClassMacro(QmitkViewPart)

  QmitkViewPart();

protected:

  void SetHandleMultipleDataStorages(bool multiple);
  bool HandlesMultipleDataStorages() const;

  mitk::DataStorage::Pointer GetDataStorage() const;
  //!mm-added
  QmitkStdMultiWidget * m_MultiWidget;
  unsigned long m_ObserverTag;
  //!
private:

  bool m_HandlesMultipleDataStorages;
};

#endif /*QMITKVIEWPART_H_*/
