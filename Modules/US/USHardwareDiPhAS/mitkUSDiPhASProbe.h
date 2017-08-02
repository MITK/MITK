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

#ifndef MITKUSDiPhASProbe_H_HEADER_INCLUDED_
#define MITKUSDiPhASProbe_H_HEADER_INCLUDED_

#include "mitkUSProbe.h"

namespace mitk
{
  /**
    * \brief Specialized mitk::USProbe for handling DiPhAS API probe objects.
    * It encapsulates a probe object from the DiPhAS API.
    *
    * This class should only be instantiated by mitk::USDiPhASProbesControls.
    * Every other object can get instances of mitk::USDiPhASProbe from there.
    */
  class USDiPhASProbe : public USProbe
  {
  public:
    mitkClassMacro(USDiPhASProbe, USProbe);
	mitkNewMacro1Param(Self, std::string);

  protected:
    /**
      * Constructs mitk::USDiPhASProbe object with given API objects.
      *
      * \param probe API probe object which should be represented by the constructed object
      * \param dataView API data view object conected to this probe object
      */
    USDiPhASProbe(std::string ProbeName);
    virtual ~USDiPhASProbe();

  };
} // namespace mitk

#endif // MITKUSDiPhASProbe_H_HEADER_INCLUDED_