/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QMITKEVENTADAPTER_H_
#define QMITKEVENTADAPTER_H_

#include <QMouseEvent>
#include <mitkEventTypedefs.h>

class QMITK_EXPORT QmitkEventAdapter
{
public:
  
  static mitk::MouseEvent AdaptMouseEvent(mitk::BaseRenderer* sender, QMouseEvent* mouseEvent);
};

#endif /*QMITKEVENTADAPTER_H_*/
