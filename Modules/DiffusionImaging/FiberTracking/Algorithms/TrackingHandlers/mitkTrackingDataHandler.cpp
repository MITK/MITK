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

#include "mitkTrackingDataHandler.h"

namespace mitk
{
TrackingDataHandler::TrackingDataHandler()
  : m_RngItk(ItkRngType::New())
  , m_NeedsDataInit(true)
{

}
}
