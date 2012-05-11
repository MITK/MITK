/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-19 21:14:21 +0200 (Di, 19 Mai 2009) $
Version:   $Revision: 17326 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _MBILOG_LOGGINGTYPES_H_
#define _MBILOG_LOGGINGTYPES_H_

namespace mbilog {
/** \brief This enum defines the message/event levels of the mbi logging mechanism.
 *		   Info: TODO
 *         Warn: TODO
 *         Error: TODO
 *         Fatal: TODO
 *         Debug: TODO
 */
  enum {
    Info,
    Warn,
    Error,
    Fatal,
    Debug
  };
}

#endif 