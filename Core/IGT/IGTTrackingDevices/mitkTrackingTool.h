/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKTRACKINGTOOL_H_HEADER_INCLUDED_
#define MITKTRACKINGTOOL_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <mitkCommon.h>
#include <mitkVector.h>
#include <itkFastMutexLock.h>

namespace mitk {


  //##Documentation
  //## \brief Interface for all Tracking Tools
  //##
  //## Defines the methods that are common for all tracking tools.
  //## A User should never directly instantiate one of its subclasses.
  //## Instead, he should always instantiate an apropriate Configurator 
  //## object and get a reference of the type TrackingTool from it.
  //## 
  //## @ingroup Tracking
  class TrackingTool : public itk::Object
  {
  public:
    mitkClassMacro(TrackingTool, itk::Object);

    virtual void GetPosition(mitk::Point3D& position) const = 0;          // returns the current position of the tool as an array of three floats
    virtual void GetQuaternion(mitk::Quaternion& orientation) const = 0;  // returns the current orientation of the tool as a quaternion in a mitk::Point4D
    //  virtual const float* GetMatrix() const;        // returns current transformation of the tool as an array of 4x4 floats TODO: which format is most usefull? return an itk transform instead?
    virtual bool Enable() = 0;                       // enable the tool, so that it will be tracked
    virtual bool Disable() = 0;                      // disable the tool, so that it will not be tracked anymore
    virtual bool IsEnabled() const = 0;              // returns wether the tool is enabled or disabled
    virtual bool IsDataValid() const = 0;            // returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
    virtual float GetTrackingError() const = 0;      // return one value that corresponds to the overall tracking error.
    virtual const char* GetToolName() const;         // every tool has a name that can be used to identify it. 
    virtual void SetToolName(const char* _arg);      // ToDo: should this be only changeable in subclasses like errormessage? ToDo: only if trackingthread reads or writes it, it has to be mutex protected, else we can use itkSetStringMacro...
    virtual const char* GetErrorMessage() const;     // if the data is not valid, ErrorMessage should contain a string explaining why it is invalid (the Set-method should be implemented in subclasses, it should not be accessible by the user)
  protected:
    TrackingTool();
    virtual ~TrackingTool();
    std::string m_ToolName;
    std::string m_ErrorMessage;
    itk::FastMutexLock::Pointer m_MyMutex;
  };
} // namespace mitk

#endif /* MITKTRACKINGTOOL_H_HEADER_INCLUDED_ */
