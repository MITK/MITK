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

#ifndef MITKUSTelemedProbe_H_HEADER_INCLUDED_
#define MITKUSTelemedProbe_H_HEADER_INCLUDED_

#include "mitkUSProbe.h"

struct IProbe;
struct IUsgDataView;

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
    mitkNewMacro2Param(Self, IProbe*, IUsgDataView*);

    /**
      * Returns the Telemed API probe connected with this object.
      * This method is just for being called by mitk::USTelemedProbesControls.
      */
    IProbe* GetUsgProbe();

    /**
      * Returns the Telemed API data view connected with this object.
      * This method is just for being called by mitk::USTelemedProbesControls.
      */
    IUsgDataView* GetUsgDataView();

  protected:
    /**
      * Constructs mitk::USTelemedProbe object with given API objects.
      *
      * \param probe API probe object which should be represented by the constructed object
      * \param dataView API data view object conected to this probe object
      */
    USTelemedProbe(IProbe* probe, IUsgDataView* dataView);
    virtual ~USTelemedProbe();

    IProbe*                         m_UsgProbe;
    IUsgDataView*                   m_UsgDataView;
  };
} // namespace mitk

#endif // MITKUSTelemedProbe_H_HEADER_INCLUDED_