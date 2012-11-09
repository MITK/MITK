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

#include "mitkConnectomicsHistogramCache.h"

mitk::ConnectomicsHistogramCache::ConnectomicsHistogramCache()
{
}

mitk::ConnectomicsHistogramCache::~ConnectomicsHistogramCache()
{
}

mitk::ConnectomicsHistogramsContainer * mitk::ConnectomicsHistogramCache::operator[]( mitk::ConnectomicsNetwork::Pointer sp_NetworkData )
{
  BaseData *p_BaseData = dynamic_cast< BaseData* >( sp_NetworkData.GetPointer() );

  if(!p_BaseData)
  {
    MITK_WARN << "ConnectomicsHistogramCache::operator[] with null connectomics network data called";
    return 0;
  }

  ConnectomicsHistogramsCacheElement *elementToUpdate = 0;

  bool first( true );

  bool found( false );

  for(CacheContainer::iterator iter = cache.begin(); iter != cache.end(); iter++)
  {
    ConnectomicsHistogramsCacheElement *e = dynamic_cast<ConnectomicsHistogramsCacheElement *>(*iter);
    BaseData *p_tmp = e->baseData.GetPointer();

    if(p_tmp == p_BaseData)
    {
      if(!first)
      {
        cache.erase(iter);
        cache.push_front(e);
      }
      if( p_BaseData->GetMTime() > e->m_LastUpdateTime.GetMTime())
      {
        // found but needs an update
        found = true;
        elementToUpdate = e;
        break;
      }

      // found but no update needed
      return dynamic_cast<ConnectomicsHistogramsContainer*>( e->GetHistograms() );
    }

    first = false;
  }

  if( !found )
  {
    if (dynamic_cast<ConnectomicsNetwork*>(p_BaseData))
    {
      elementToUpdate = new ConnectomicsHistogramsCacheElement();
    }
    else
    {
      MITK_WARN << "not supported: " << p_BaseData->GetNameOfClass();
      return NULL;
    }

    elementToUpdate->baseData = p_BaseData;
    cache.push_front(elementToUpdate);
    TrimCache();
  }

  if(elementToUpdate)
  {
    elementToUpdate->ComputeFromBaseData(p_BaseData);
    elementToUpdate->m_LastUpdateTime.Modified();
    return dynamic_cast<ConnectomicsHistogramsContainer*>( elementToUpdate->GetHistograms() );
  }
  return NULL;
}

