/*=========================================================================

Program:   openCherry Platform
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

#ifndef __CHERRY_LOG_H__
#define __CHERRY_LOG_H__

#include <mbilog.h>

#define CHERRY_INFO LOG_INFO("BlueBerry")
#define CHERRY_WARN LOG_WARN("BlueBerry")
#define CHERRY_ERROR LOG_ERROR("BlueBerry")
#define CHERRY_FATAL LOG_FATAL("BlueBerry")

#define CHERRY_DEBUG LOG_DEBUG("BlueBerry")

#endif /*__CHERRY_LOG_H__*/
