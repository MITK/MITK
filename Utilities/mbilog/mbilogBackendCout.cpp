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

#include <list>
#include <ctime>
#include <iomanip>

#include "mbilogBackendCout.h"



mbilog::BackendCout::BackendCout()
{
  m_useFullOutput=false;
}

mbilog::BackendCout::~BackendCout()
{
}

void mbilog::BackendCout::SetFull(bool full)
{
  m_useFullOutput = full;
}

void mbilog::BackendCout::ProcessMessage(const mbilog::LogMessage& l)
{
  if(m_useFullOutput)
    FormatFull(l);
  else
    FormatSmart(l);
}

mbilog::OutputType mbilog::BackendCout::GetOutputType() const
{
  return mbilog::Console;
}




