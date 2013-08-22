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
  class USTelemedProbe : public USProbe
  {
  public:
    mitkClassMacro(USTelemedProbe, USProbe);
    mitkNewMacro2Param(Self, IProbe*, IUsgDataView*);

    IProbe* GetUsgProbe();
    IUsgDataView* GetUsgDataView();

  protected:
    USTelemedProbe(IProbe*, IUsgDataView*);
    virtual ~USTelemedProbe();

    IProbe*                         m_UsgProbe;
    IUsgDataView*                   m_UsgDataView;
  };
}

#endif // MITKUSTelemedProbe_H_HEADER_INCLUDED_