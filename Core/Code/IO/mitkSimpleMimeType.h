/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKSIMPLEMIMETYPE_H
#define MITKSIMPLEMIMETYPE_H

#include "mitkIMimeType.h"

namespace mitk {

class MITK_CORE_EXPORT SimpleMimeType : public IMimeType
{

public:

  virtual std::string GetMagicPatterns() const;

};

}

#endif // MITKSIMPLEMIMETYPE_H
