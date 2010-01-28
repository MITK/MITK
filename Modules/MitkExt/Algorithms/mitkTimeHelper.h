/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTIMEHELPER_H_HEADER_INCLUDED_C1C2FCD2
#define MITKTIMEHELPER_H_HEADER_INCLUDED_C1C2FCD2

namespace mitk
{

//## @brief convert the start- and end-index-time of output-region in 
//## start- and end-index-time of input-region via millisecond-time
template <class TOutputRegion, class TInputRegion>
void ITK_EXPORT GenerateTimeInInputRegion(const mitk::TimeSlicedGeometry *outputTimeGeometry, const TOutputRegion& outputRegion, const mitk::TimeSlicedGeometry *inputTimeGeometry, TInputRegion& inputRegion)
{
  assert(outputTimeGeometry!=NULL);
  assert(inputTimeGeometry!=NULL);

  // convert the start-index-time of output in start-index-time of input via millisecond-time
  ScalarType timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3));
  int timestep = inputTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( inputTimeGeometry->IsValidTime( timestep ) ) )
    inputRegion.SetIndex( 3, timestep );
  else
    inputRegion.SetIndex( 3, 0 );
  // convert the end-index-time of output in end-index-time of input via millisecond-time
  timeInMS = outputTimeGeometry->TimeStepToMS(outputRegion.GetIndex(3)+outputRegion.GetSize(3)-1);
  timestep = inputTimeGeometry->MSToTimeStep( timeInMS );
  if( ( timeInMS > ScalarTypeNumericTraits::NonpositiveMin() ) && ( outputTimeGeometry->IsValidTime( timestep ) ) )
    inputRegion.SetSize( 3, timestep - inputRegion.GetIndex(3) + 1 );
  else
    inputRegion.SetSize( 3, 1 );
}

//##Documentation
//## @brief convert the start- and end-index-time of output in 
//## start- and end-index-time of input1 and input2 via millisecond-time
template <class TOutputData, class TInputData>
void ITK_EXPORT GenerateTimeInInputRegion(const TOutputData* output, TInputData* input)
{
  assert(output!=NULL);
  assert(input!=NULL);

  const typename TOutputData::RegionType& outputRegion = output->GetRequestedRegion();
  typename TInputData::RegionType inputRegion;

  if(outputRegion.GetSize(3)<1)
  {
    typename TInputData::RegionType::SizeType inputsize;
    inputsize.Fill(0);
    inputRegion.SetSize(inputsize);
    input->SetRequestedRegion( &inputRegion );
  }

  // convert the start-index-time of output in start-index-time of input via millisecond-time
  inputRegion = input->GetRequestedRegion();
  GenerateTimeInInputRegion(output->GetTimeSlicedGeometry(), outputRegion, input->GetTimeSlicedGeometry(), inputRegion);
  input->SetRequestedRegion( &inputRegion  );
}

} // end namespace mitk

//#ifndef ITK_MANUAL_INSTANTIATION
//#include "mitkTimeHelper.txx"
#include "MitkExtExports.h"
//#endif

#endif // MITKTIMEHELPER_H_HEADER_INCLUDED_C1C2FCD2
