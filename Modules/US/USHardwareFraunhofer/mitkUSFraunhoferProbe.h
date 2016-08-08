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

#ifndef MITKUSFraunhoferProbe_H_HEADER_INCLUDED_
#define MITKUSFraunhoferProbe_H_HEADER_INCLUDED_

#include "mitkUSProbe.h"

namespace Usgfw2Lib {
  struct IProbe;
  struct IUsgDataView;
}

namespace mitk
{
  /**
    * \brief Specialized mitk::USProbe for handling Fraunhofer API probe objects.
    * It encapsulates a probe object from the Fraunhofer API.
    *
    * This class should only be instantiated by mitk::USFraunhoferProbesControls.
    * Every other object can get instances of mitk::USFraunhoferProbe from there.
    */
  class USFraunhoferProbe : public USProbe
  {
  public:
    mitkClassMacro(USFraunhoferProbe, USProbe);
	mitkNewMacro1Param(Self, std::string);

  protected:
    /**
      * Constructs mitk::USFraunhoferProbe object with given API objects.
      *
      * \param probe API probe object which should be represented by the constructed object
      * \param dataView API data view object conected to this probe object
      */
    USFraunhoferProbe(std::string ProbeName);
    virtual ~USFraunhoferProbe();

  };
} // namespace mitk

#endif // MITKUSFraunhoferProbe_H_HEADER_INCLUDED_