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


#ifndef MITKSTATUSBARIMPLEMENTATION_H
#define MITKSTATUSBARIMPLEMENTATION_H
#include <MitkCoreExports.h>
#include <mitkCommon.h>

#include <mitkPoint.h>
#include <mitkNumericConstants.h>
#include <itkIndex.h>

namespace mitk {
//##Documentation
//## @brief GUI indepentent Interface for all Gui depentent implementations of a StatusBar.
class MITKCORE_EXPORT StatusBarImplementation
{
public:
  mitkClassMacroNoParent(StatusBarImplementation)

  //##Documentation
  //## @brief Constructor
  StatusBarImplementation(){};
  //##Documentation
  //## @brief Destructor
  virtual ~StatusBarImplementation(){};

  //##Documentation
  //## @brief Send a string to the applications StatusBar
  virtual void DisplayText(const char* t)=0;

  //##Documentation
  //## @brief Send a string with a time delay to the applications StatusBar
  virtual void DisplayText(const char* t, int ms) = 0;
  virtual void DisplayErrorText(const char *t) = 0;
  virtual void DisplayWarningText(const char *t) = 0;
  virtual void DisplayWarningText(const char *t, int ms) = 0;
  virtual void DisplayGenericOutputText(const char *t) = 0;
  virtual void DisplayDebugText(const char *t) = 0;
  virtual void DisplayGreyValueText(const char *t) = 0;
  virtual void DisplayImageInfo(mitk::Point3D point, itk::Index<3> index, mitk::ScalarType time, mitk::ScalarType pixelValue) = 0;
  virtual void DisplayImageInfo(mitk::Point3D point, itk::Index<3> index, mitk::ScalarType time, const char* pixelValue) = 0;
  virtual void DisplayImageInfoInvalid() = 0;


  //##Documentation
  //## @brief removes any temporary message being shown.
  virtual void Clear() = 0;

  //##Documentation
  //## @brief Set the SizeGrip of the window
  //## (the triangle in the lower right Windowcorner for changing the size)
  //## to enabled or disabled
  virtual void SetSizeGripEnabled(bool enable) = 0;
};

}// end namespace mitk
#endif /* define MITKSTATUSBARIMPLEMENTATION_H */
