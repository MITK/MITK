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


#ifndef MITKVirtualTrackingTool_H_HEADER_INCLUDED_
#define MITKVirtualTrackingTool_H_HEADER_INCLUDED_

#include <mitkInternalTrackingTool.h>
#include <MitkIGTExports.h>
#include <mitkNumericTypes.h>
#include <itkFastMutexLock.h>
#include <mitkItkNonUniformBSpline.h>

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
    itkFactorylessNewMacro(Self)
    typedef itk::NonUniformBSpline<3> SplineType; ///< spline type used for tool path interpolation

    itkGetMacro(SplineLength, mitk::ScalarType);
    itkSetMacro(SplineLength, mitk::ScalarType);

    itkGetMacro(Velocity, mitk::ScalarType);
    itkSetMacro(Velocity, mitk::ScalarType);

    itkGetObjectMacro(Spline, SplineType);


  protected:
    itkCloneMacro(Self)
    VirtualTrackingTool();
    virtual ~VirtualTrackingTool();

    SplineType::Pointer m_Spline;
    mitk::ScalarType m_SplineLength;
    mitk::ScalarType m_Velocity;
  };
} // namespace mitk
#endif /* MITKVirtualTrackingTool_H_HEADER_INCLUDED_ */
