/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-11 15:15:02 +0200 (Di, 11 Aug 2009) $
Version:   $Revision: 18516 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKVirtualTrackingTool_H_HEADER_INCLUDED_
#define MITKVirtualTrackingTool_H_HEADER_INCLUDED_

#include <mitkInternalTrackingTool.h>
#include <MitkIGTExports.h>
#include <mitkVector.h>
#include <itkFastMutexLock.h>

#if ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR == 18 && ITK_VERSION_PATCH == 0
  #include <mitkItkNonUniformBSpline.h>  // fixed version of a class that is defect in ITK 3.18
#else
  #include <itkNonUniformBSpline.h>
#endif


namespace mitk {


  /**Documentation
  * \brief implements TrackingTool interface
  *
  * This class is a complete TrackingTool implementation. It can either be used directly by
  * TrackingDevices, or be subclassed for more specific implementations.
  * mitk::MicroBirdTrackingDevice uses this class to manage its tools. Other tracking devices
  * uses specialized versions of this class (e.g. mitk::NDITrackingTool)
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT VirtualTrackingTool : public InternalTrackingTool
  {
  public:
    mitkClassMacro(VirtualTrackingTool, InternalTrackingTool);
    friend class VirtualTrackingDevice;
    typedef itk::NonUniformBSpline<3> SplineType; ///< spline type used for tool path interpolation
    
    itkGetMacro(SplineLength, mitk::ScalarType);
    itkSetMacro(SplineLength, mitk::ScalarType);

    itkGetMacro(Velocity, mitk::ScalarType);
    itkSetMacro(Velocity, mitk::ScalarType);

    itkGetObjectMacro(Spline, SplineType);

  protected:
    itkNewMacro(Self);
    VirtualTrackingTool();
    virtual ~VirtualTrackingTool();
    
    SplineType::Pointer m_Spline;
    mitk::ScalarType m_SplineLength;
    mitk::ScalarType m_Velocity;
  };
} // namespace mitk
#endif /* MITKVirtualTrackingTool_H_HEADER_INCLUDED_ */
