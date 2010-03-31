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

#ifndef MITK_UID_GENERATOR_INDCLUDED_FASAWE
#define MITK_UID_GENERATOR_INDCLUDED_FASAWE

#include<string>
#include "mitkCommon.h"

namespace mitk {

/*!
  \brief Generated unique IDs

  Creates (somehow most of the time) unique IDs from a given prefix, 
  the current date/time and a random part.

  The prefix is given to the constructor, together with the desired
  length of the random part (minimum 5 digits). 

  You will get another quite unique ID each time you call GetUID.
*/
class MITK_CORE_EXPORT UIDGenerator
{
  public:
    UIDGenerator(const char* prefix = "UID_", unsigned int lengthOfRandomPart = 8);

    std::string GetUID();
  private:
    std::string m_Prefix;
    unsigned int m_LengthOfRandomPart;
};

}

#endif
