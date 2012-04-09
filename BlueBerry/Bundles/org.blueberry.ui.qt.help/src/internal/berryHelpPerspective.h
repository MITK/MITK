/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYHELPPERSPECTIVE_H_
#define BERRYHELPPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>

namespace berry {

class HelpPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  static const std::string ID;

  HelpPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

}

#endif /* BERRYHELPPERSPECTIVE_H_ */
