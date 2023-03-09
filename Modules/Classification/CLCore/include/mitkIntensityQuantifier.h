/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIntensityQuantifier_h
#define mitkIntensityQuantifier_h

#include <MitkCLCoreExports.h>

#include <mitkBaseData.h>
#include <mitkImage.h>

namespace mitk
{
class MITKCLCORE_EXPORT IntensityQuantifier : public BaseData
{
public:
  mitkClassMacro(IntensityQuantifier, BaseData);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  IntensityQuantifier();

  void InitializeByMinimumMaximum(double minimum, double maximum, unsigned int bins);
  void InitializeByBinsizeAndBins(double minimum, unsigned int bins, double binsize);
  void InitializeByBinsizeAndMaximum(double minimum, double maximum, double binsize);
  void InitializeByImage(const Image* image, unsigned int bins);
  void InitializeByImageAndMaximum(const Image* image, double maximum, unsigned int bins);
  void InitializeByImageAndMinimum(const Image* image, double minimum, unsigned int bins);
  void InitializeByImageRegion(const Image* image, const Image* mask, unsigned int bins);
  void InitializeByImageRegionAndMinimum(const Image* image, const Image* mask, double minimum, unsigned int bins);
  void InitializeByImageRegionAndMaximum(const Image* image, const Image* mask, double maximum, unsigned int bins);
  void InitializeByImageAndBinsize(const Image* image, double binsize);
  void InitializeByImageAndBinsizeAndMinimum(const Image* image, double minimum, double binsize);
  void InitializeByImageAndBinsizeAndMaximum(const Image* image, double maximum, double binsize);
  void InitializeByImageRegionAndBinsize(const Image* image, const Image* mask, double binsize);
  void InitializeByImageRegionAndBinsizeAndMinimum(const Image* image, const Image* mask, double minimum, double binsize);
  void InitializeByImageRegionAndBinsizeAndMaximum(const Image* image, const Image* mask, double maximum, double binsize);

  unsigned int IntensityToIndex(double intensity);
  double IndexToMinimumIntensity(unsigned int index);
  double IndexToMeanIntensity(unsigned int index);
  double IndexToMaximumIntensity(unsigned int index);

  itkGetConstMacro(Initialized, bool);
  itkGetConstMacro(Bins, unsigned int);
  itkGetConstMacro(Binsize, double);
  itkGetConstMacro(Minimum, double);
  itkGetConstMacro(Maximum, double);

public:

//#ifndef DOXYGEN_SKIP

  void SetRequestedRegionToLargestPossibleRegion() override {};
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  bool VerifyRequestedRegion() override { return false; };
  void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  bool IsEmpty() const override
  {
    if(IsInitialized() == false)
      return true;
    const TimeGeometry* timeGeometry = const_cast<IntensityQuantifier*>(this)->GetUpdatedTimeGeometry();
    if(timeGeometry == nullptr)
      return true;
    return false;
  }


private:
  bool m_Initialized;
  unsigned int m_Bins;
  double m_Binsize;
  double m_Minimum;
  double m_Maximum;

};
}

#endif
