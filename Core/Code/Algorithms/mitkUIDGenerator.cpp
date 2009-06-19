/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Last checked in by: $Author$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <mitkUIDGenerator.h>
#include <ctime>
#include <sstream>
#include <math.h>
#include <stdexcept>
#include <iostream>

namespace mitk {

UIDGenerator::UIDGenerator(const char* prefix, unsigned int lengthOfRandomPart)
:m_Prefix(prefix),
 m_LengthOfRandomPart(lengthOfRandomPart)
{
  if (lengthOfRandomPart < 5)
  {
    LOG_ERROR << lengthOfRandomPart << " are not really unique, right?" << std::endl;
    throw std::invalid_argument("To few digits requested");
  }

  srand((unsigned int) time( (time_t *)0 ));
}

std::string UIDGenerator::GetUID()
{
  std::ostringstream s;
  s << m_Prefix;
  time_t tt = time(0);
  tm* t = gmtime(&tt);

  if (t)
  {
    s << t->tm_year + 1900;
    
    if (t->tm_mon < 9) s << "0"; // add a 0 for months 1 to 9
    s << t->tm_mon + 1;

    if (t->tm_mday < 10) s << "0"; // add a 0 for days 1 to 9
    s << t->tm_mday;

    if (t->tm_hour < 10) s << "0"; // add a 0 for hours 1 to 9
    s << t->tm_hour;

    if (t->tm_min < 10) s << "0"; // add a 0 for minutes 1 to 9
    s << t->tm_min;
    
    if (t->tm_sec < 10) s << "0"; // add a 0 for seconds 1 to 9
    s << t->tm_sec;
    
    std::ostringstream rs;
    rs << (long int)( pow(10.0, double(m_LengthOfRandomPart)) / double(RAND_MAX) * double(rand()) );

    for (size_t i = rs.str().length(); i < m_LengthOfRandomPart; ++i)
    {
      s << "X";
    }

    s << rs.str();
  }

  return s.str();
}

}

