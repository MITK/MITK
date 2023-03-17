/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkVirtualTrackingTool_h
#define mitkVirtualTrackingTool_h

#include <mitkTrackingTool.h>
#include <MitkIGTExports.h>
#include <mitkNumericTypes.h>
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
  class MITKIGT_EXPORT VirtualTrackingTool : public TrackingTool
  {
  public:
    mitkClassMacro(VirtualTrackingTool, TrackingTool);
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
    ~VirtualTrackingTool() override;

    SplineType::Pointer m_Spline;
    mitk::ScalarType m_SplineLength;
    mitk::ScalarType m_Velocity;
  };
} // namespace mitk
#endif
