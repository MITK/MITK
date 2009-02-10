/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKTIMESTAMP_H_HEADER_INCLUDED_
#define MITKTIMESTAMP_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>


namespace mitk {

  /**Documentation
  * @brief Time stamp in milliseconds
  *
  * This class provides a timestamp in milliseconds.
  *  WORK IN PROGRESS
  *
  *@ingroup Navigation
  */
  class TimeStamp : public itk::Object
  {
  public:
    mitkClassMacro(TimeStamp, itk::Object);
    itkNewMacro(Self);
    static const TimeStamp* GetTimeStamp();
 
  protected:
    TimeStamp();
    virtual ~TimeStamp();

    double m_Time;
  };
} // namespace mitk

#endif /* MITKTIMESTAMP_H_HEADER_INCLUDED_ */
