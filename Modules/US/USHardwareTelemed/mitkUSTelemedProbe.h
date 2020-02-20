/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSTelemedProbe_H_HEADER_INCLUDED_
#define MITKUSTelemedProbe_H_HEADER_INCLUDED_

#include "mitkUSProbe.h"

namespace Usgfw2Lib {
  struct IProbe;
  struct IUsgDataView;
}

namespace mitk
{
  /**
    * \brief Specialized mitk::USProbe for handling Telemed API probe objects.
    * It encapsulates a probe object from the Telemed API.
    *
    * This class should only be instantiated by mitk::USTelemedProbesControls.
    * Every other object can get instances of mitk::USTelemedProbe from there.
    */
  class USTelemedProbe : public USProbe
  {
  public:
    mitkClassMacro(USTelemedProbe, USProbe);
    mitkNewMacro2Param(Self, Usgfw2Lib::IProbe*, Usgfw2Lib::IUsgDataView*);

    /**
      * Returns the Telemed API probe connected with this object.
      * This method is just for being called by mitk::USTelemedProbesControls.
      */
    Usgfw2Lib::IProbe* GetUsgProbe();

    /**
      * Returns the Telemed API data view connected with this object.
      * This method is just for being called by mitk::USTelemedProbesControls.
      */
    Usgfw2Lib::IUsgDataView* GetUsgDataView();

  protected:
    /**
      * Constructs mitk::USTelemedProbe object with given API objects.
      *
      * \param probe API probe object which should be represented by the constructed object
      * \param dataView API data view object conected to this probe object
      */
    USTelemedProbe(Usgfw2Lib::IProbe* probe, Usgfw2Lib::IUsgDataView* dataView);
    virtual ~USTelemedProbe();

    Usgfw2Lib::IProbe*                         m_UsgProbe;
    Usgfw2Lib::IUsgDataView*                   m_UsgDataView;
  };
} // namespace mitk

#endif // MITKUSTelemedProbe_H_HEADER_INCLUDED_
