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

#include "mitkSimpleHistogram.h"

#include "mitkSimpleUnstructuredGridHistogram.h"
#include "mitkUnstructuredGrid.h"
#include "mitkImageReadAccessor.h"

namespace mitk {

void SimpleImageHistogram::ComputeFromBaseData( BaseData* src )
{
  valid = false;

  //check if input is valid
  if (src==nullptr) return;
  Image* source = dynamic_cast<Image*>(src);
  if (source==nullptr) return;
  else if (source->IsEmpty()) return;



  // dummy histogram
  {
    min=0;
    max=1;
    first=0;
    last=1;
  }

   {
    int typInt=0;
    {
      const int typ=source->GetPixelType().GetComponentType();
      if     (typ == itk::ImageIOBase::UCHAR) typInt=0;
      else if(typ == itk::ImageIOBase::CHAR) typInt=1;
      else if(typ == itk::ImageIOBase::USHORT) typInt=2;
      else if(typ == itk::ImageIOBase::SHORT) typInt=3;
      else if(typ == itk::ImageIOBase::INT) typInt=4;
      else if(typ == itk::ImageIOBase::UINT) typInt=5;
      else if(typ == itk::ImageIOBase::LONG) typInt=6;
      else if(typ == itk::ImageIOBase::ULONG) typInt=7;
      else if(typ == itk::ImageIOBase::FLOAT) typInt=8;
      else if(typ == itk::ImageIOBase::DOUBLE) typInt=9;
      else
      {
        MITK_WARN << "Pixel type not supported by SimpleImageHistogram";
        return;
      }
    }


    first=-32768; last=65535; // support at least full signed and unsigned short range

    if(histogram)
      delete histogram;

    histogram = new CountType[last-first+1];
    memset(histogram,0,sizeof(CountType)*(last-first+1));
    highest = 0;
    max = first - 1;
    min = last + 1;

    unsigned int num=1;
    for(unsigned int r=0;r<source->GetDimension();r++)
      num*=source->GetDimension(r);

   // MITK_INFO << "building histogramm of integer image: 0=" << source->GetDimension(0) << " 1=" << source->GetDimension(1) << " 2=" << source->GetDimension(2) << " 3=" <<  source->GetDimension(3);

    ImageReadAccessor sourceAcc(source);
    const void *src = sourceAcc.GetData();

    do
    {
      int value = 0;

      switch(typInt)
      {
        case 0: { unsigned char  *t=(unsigned char *)src; value=*t++; src=(void*)t; } break;
        case 1: {   signed char  *t=(  signed char *)src; value=*t++; src=(void*)t; } break;
        case 2: { unsigned short *t=(unsigned short*)src; value=*t++; src=(void*)t; } break;
        case 3: {   signed short *t=(  signed short*)src; value=*t++; src=(void*)t; } break;
        case 4: {   signed int *t=(  signed int*)src; value=*t++; src=(void*)t; } break;
        case 5: { unsigned int *t=(unsigned int*)src; value=*t++; src=(void*)t; } break;
        case 6: {   signed long *t=(  signed long*)src; value=*t++; src=(void*)t; } break;
        case 7: { unsigned long *t=(unsigned long*)src; value=*t++; src=(void*)t; } break;
        case 8: {   float *t=( float*)src; value=*t++; src=(void*)t; } break;
        case 9: {   double *t=( double*)src; value=*t++; src=(void*)t; } break;
      }

      if(value >= first && value <= last)
      {
        if(value < min) min = value;
        if(value > max) max = value;
        CountType tmp = ++histogram[value-first];
        if(tmp > highest) highest = tmp;
      }
    }
    while(--num);

    MITK_INFO << "histogramm computed: min=" << min << " max=" << max << " highestBin=" << highest << " samples=" << num;
  }

  invLogHighest = 1.0/log(double(highest));
  valid = true;
}

bool SimpleImageHistogram::GetValid()
  {
  return valid;
  }

float SimpleImageHistogram::GetRelativeBin( double left, double right ) const
{
  if( !valid )
    return 0.0f;

  int iLeft = floorf(left);
  int iRight = ceilf(right);

 /*
  double sum = 0;

  for( int r = 0 ; r < 256 ; r++)
  {
    int pos = left + (right-left) * r/255.0;
    int posInArray = floorf(pos+0.5f) - first;
    sum += float(log(double(histogram[posInArray])));
  }

  sum /= 256.0;
  return float(sum*invLogHighest);
 */

  CountType maximum = 0;

  for( int i = iLeft; i <= iRight ; i++)
  {
    int posInArray = i - first;
    if( histogram[posInArray] > maximum ) maximum = histogram[posInArray];
  }

  return float(log(double(maximum))*invLogHighest);
}

class ImageHistogramCacheElement : public SimpleHistogramCache::Element
{
  public:

    void ComputeFromBaseData(BaseData* baseData) override
    {
      histogram.ComputeFromBaseData(baseData);
    }

    SimpleHistogram* GetHistogram() override
    {
      return &histogram;
    }

    SimpleImageHistogram histogram;
};

class UnstructuredGridHistogramCacheElement : public SimpleHistogramCache::Element
{
  public:

    void ComputeFromBaseData(BaseData* baseData) override
    {
      histogram.ComputeFromBaseData(baseData);
    }

    SimpleHistogram* GetHistogram() override
    {
      return &histogram;
    }

    SimpleUnstructuredGridHistogram histogram;
};

SimpleHistogram* SimpleHistogramCache::operator[](BaseData::Pointer sp_BaseData)
{
  BaseData *p_BaseData = sp_BaseData.GetPointer();

  if(!p_BaseData)
  {
    MITK_WARN << "SimpleHistogramCache::operator[] with null base data called";
    return nullptr;
  }

  Element *elementToUpdate = nullptr;

  bool first = true;

  for(auto iter = cache.begin(); iter != cache.end(); iter++)
  {
    Element *e = *iter;
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
        elementToUpdate = e;
        goto recomputeElement;
      }

      //MITK_INFO << "using a cached histogram";

      return e->GetHistogram();
    }

    first = false;
  }

  if (dynamic_cast<Image*>(p_BaseData))
  {
    elementToUpdate = new ImageHistogramCacheElement();
  }
  else if (dynamic_cast<UnstructuredGrid*>(p_BaseData))
  {
    elementToUpdate = new UnstructuredGridHistogramCacheElement();
  }
  else
  {
    MITK_WARN << "not supported: " << p_BaseData->GetNameOfClass();
  }

  elementToUpdate->baseData = p_BaseData;
  cache.push_front(elementToUpdate);
  TrimCache();

  recomputeElement:

  //MITK_INFO << "computing a new histogram";

  elementToUpdate->ComputeFromBaseData(p_BaseData);
  elementToUpdate->m_LastUpdateTime.Modified();
  return elementToUpdate->GetHistogram();
}

SimpleHistogramCache::Element::~Element()
{
}

}
