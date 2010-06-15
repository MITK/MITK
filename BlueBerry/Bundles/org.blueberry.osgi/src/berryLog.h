/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date: 2009-03-06 01:07:35 +0100 (Fri, 06 Mar 2009) $
Version:   $Revision: 16492 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __BERRY_LOG_H__
#define __BERRY_LOG_H__

#include <mbilog.h>

#define BERRY_INFO MBI_INFO("BlueBerry")
#define BERRY_WARN MBI_WARN("BlueBerry")
#define BERRY_ERROR MBI_ERROR("BlueBerry")
#define BERRY_FATAL MBI_FATAL("BlueBerry")

#define BERRY_DEBUG MBI_DEBUG("BlueBerry")

#endif /*__BERRY_LOG_H__*/
