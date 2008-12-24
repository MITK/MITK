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


#ifndef CHERRYPARTINFO_H_
#define CHERRYPARTINFO_H_

#include <QPixmap>
#include <presentations/cherryIPresentablePart.h>

namespace cherry {

struct PartInfo {
     QString name;
     QString title;
     QString contentDescription;
     QString toolTip;
     QPixmap* image;
     bool dirty;

     PartInfo();

     PartInfo(IPresentablePart::Pointer part);

     void Set(IPresentablePart::Pointer part);
};

}


#endif /* CHERRYPARTINFO_H_ */
