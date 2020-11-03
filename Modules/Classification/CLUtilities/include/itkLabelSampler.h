/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itLabelSampler_h
#define itLabelSampler_h

#include "itkImageToImageFilter.h"

namespace itk
{
template< class TImage>
class LabelSampler:public ImageToImageFilter< TImage, TImage >
{
public:
  /** Standard class typedefs. */
  typedef LabelSampler             Self;
  typedef ImageToImageFilter< TImage, TImage > Superclass;
  typedef SmartPointer< Self >        Pointer;
  typedef std::map<int,int> outmap;

  /** Method for creation through the object factory. */
  itkNewMacro(Self); /** Run-time type information (and related methods). */
  itkTypeMacro(Self, ImageToImageFilter);
  itkSetMacro(AcceptRate, double);
  itkSetMacro(Label, int);
  itkGetMacro(LabelVoxelCountMap, outmap);
  itkGetMacro(NumberOfSampledVoxels,int);

  int GetLabelVoxelCount(int label)
  {
    if(m_LabelVoxelCountMap.empty())
      return 0;
    return m_LabelVoxelCountMap[label];
  }

protected:
  LabelSampler()
  :m_Label(-1)
  ,m_AcceptRate(1.0)
  ,m_UseLabelVoxelCount(false)
  {
    this->SetNumberOfRequiredInputs(1);
    this->SetNumberOfRequiredOutputs(1);
  }

  ~LabelSampler(){}

  /** Does the real work. */
  virtual void GenerateData();

private:
  LabelSampler(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  int m_Label;
  int m_NumberOfSampledVoxels;
  double m_AcceptRate;
  bool m_UseLabelVoxelCount;
  std::map<int,int> m_LabelVoxelCountMap;
};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include <../src/Algorithms/itkLabelSampler.cpp>
#endif


#endif // itLabelSampler_h
