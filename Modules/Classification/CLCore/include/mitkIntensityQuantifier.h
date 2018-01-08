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
  mitkClassMacro(IntensityQuantifier, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  IntensityQuantifier();

  void InitializeByMinimumMaximum(double minimum, double maximum, unsigned int bins);
  void InitializeByBinsizeAndBins(double minimum, unsigned int bins, double binsize);
  void InitializeByBinsizeAndMaximum(double minimum, double maximum, double binsize);
  void InitializeByImage(mitk::Image::Pointer image, unsigned int bins);
  void InitializeByImageAndMaximum(mitk::Image::Pointer image, double maximum, unsigned int bins);
  void InitializeByImageAndMinimum(mitk::Image::Pointer image, double minimum, unsigned int bins);
  void InitializeByImageRegion(mitk::Image::Pointer image, mitk::Image::Pointer mask, unsigned int bins);
  void InitializeByImageRegionAndMinimum(mitk::Image::Pointer image, mitk::Image::Pointer mask, double minimum, unsigned int bins);
  void InitializeByImageRegionAndMaximum(mitk::Image::Pointer image, mitk::Image::Pointer mask, double maximum, unsigned int bins);
  void InitializeByImageAndBinsize(mitk::Image::Pointer image, double binsize);
  void InitializeByImageAndBinsizeAndMinimum(mitk::Image::Pointer image, double minimum, double binsize);
  void InitializeByImageAndBinsizeAndMaximum(mitk::Image::Pointer image, double maximum, double binsize);
  void InitializeByImageRegionAndBinsize(mitk::Image::Pointer image, mitk::Image::Pointer mask, double binsize);
  void InitializeByImageRegionAndBinsizeAndMinimum(mitk::Image::Pointer image, mitk::Image::Pointer mask, double minimum, double binsize);
  void InitializeByImageRegionAndBinsizeAndMaximum(mitk::Image::Pointer image, mitk::Image::Pointer mask, double maximum, double binsize);

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

  virtual void SetRequestedRegionToLargestPossibleRegion() override {};
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  virtual bool VerifyRequestedRegion() override { return false; };
  virtual void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  virtual bool IsEmpty() const override
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

#endif //mitkIntensityQuantifier_h
