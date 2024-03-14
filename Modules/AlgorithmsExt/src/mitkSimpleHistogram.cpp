/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSimpleHistogram.h"

#include "mitkImageReadAccessor.h"
#include "mitkSimpleUnstructuredGridHistogram.h"
#include "mitkUnstructuredGrid.h"
#include <mitkHistogramGenerator.h>

namespace mitk
{
  void SimpleImageHistogram::ComputeFromBaseData(BaseData *src)
  {
    valid = false;

    // check if input is valid
    if (src == nullptr)
      return;
    auto *source = dynamic_cast<Image *>(src);
    if (source == nullptr)
      return;
    else if (source->IsEmpty())
      return;

    auto generator = mitk::HistogramGenerator::New();
    generator->SetImage(source);
    generator->SetSize(nBins);
    generator->ComputeHistogram();
    histogram = static_cast<mitk::SimpleImageHistogram::HistogramType::ConstPointer> (generator->GetHistogram());

    CountType highest = 0;
    for (unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
      CountType count = histogram->GetFrequency(i);
      if (count > highest)
        highest = count;
    }
    invLogHighest = 1.0 / log(double(highest));
    valid = true;
  }

  bool SimpleImageHistogram::GetValid() { return valid; }
  float SimpleImageHistogram::GetRelativeBin(double left, double right) const
  {
    if (!valid)
      return 0.0f;

    CountType maximum = 0;

    for (unsigned int i = 0; i < nBins; i++)
    {
      auto binMin = histogram->GetBinMin(0, i);
      auto binMax = histogram->GetBinMax(0, i);
      if (left < binMax && right > binMin)
      {
        maximum = histogram->GetFrequency(i);
      }
    }
    return float(log(double(maximum)) * invLogHighest);
  }

  double SimpleImageHistogram::GetMin() const
  {
    if (!valid)
      return 0;

    return histogram->GetBinMin(0, 0);
  }

  double SimpleImageHistogram::GetMax() const
  {
    if (!valid)
      return 1;

    return histogram->GetBinMax(0, nBins - 1);
  }

  class ImageHistogramCacheElement : public SimpleHistogramCache::Element
  {
  public:
    void ComputeFromBaseData(BaseData *baseData) override { histogram.ComputeFromBaseData(baseData); }
    SimpleHistogram *GetHistogram() override { return &histogram; }
    SimpleImageHistogram histogram;
  };

  class UnstructuredGridHistogramCacheElement : public SimpleHistogramCache::Element
  {
  public:
    void ComputeFromBaseData(BaseData *baseData) override { histogram.ComputeFromBaseData(baseData); }
    SimpleHistogram *GetHistogram() override { return &histogram; }
    SimpleUnstructuredGridHistogram histogram;
  };

  SimpleHistogram *SimpleHistogramCache::operator[](BaseData::Pointer sp_BaseData)
  {
    BaseData *p_BaseData = sp_BaseData.GetPointer();

    if (!p_BaseData)
    {
      MITK_WARN << "SimpleHistogramCache::operator[] with null base data called";
      return nullptr;
    }

    Element *elementToUpdate = nullptr;

    bool first = true;

    for (auto iter = cache.begin(); iter != cache.end(); iter++)
    {
      Element *e = *iter;
      BaseData *p_tmp = e->baseData.Lock();

      if (p_tmp == p_BaseData)
      {
        if (!first)
        {
          cache.erase(iter);
          cache.push_front(e);
        }
        if (p_BaseData->GetMTime() > e->m_LastUpdateTime.GetMTime())
        {
          elementToUpdate = e;
          goto recomputeElement;
        }

        // MITK_INFO << "using a cached histogram";

        return e->GetHistogram();
      }

      first = false;
    }

    if (dynamic_cast<Image *>(p_BaseData))
    {
      elementToUpdate = new ImageHistogramCacheElement();
    }
    else if (dynamic_cast<UnstructuredGrid *>(p_BaseData))
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

    // MITK_INFO << "computing a new histogram";

    elementToUpdate->ComputeFromBaseData(p_BaseData);
    elementToUpdate->m_LastUpdateTime.Modified();
    return elementToUpdate->GetHistogram();
  }

  SimpleHistogramCache::Element::~Element() {}
}
