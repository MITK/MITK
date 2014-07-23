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


#ifndef __mitkLabel_H_
#define __mitkLabel_H_

#include "MitkSegmentationExports.h"
#include <mitkColorProperty.h>
#include <mitkVector.h>
#include <mitkPropertyList.h>

class TiXmlDocument;
class TiXmlElement;

namespace mitk
{

//##
//##Documentation
//## @brief A data structure describing a label.
//## @ingroup Data
//##
class MitkSegmentation_EXPORT Label : public PropertyList
{
public:
  mitkClassMacro( Label, mitk::PropertyList );

  itkNewMacro( Self );

  void SetLocked(bool locked);
  bool GetLocked();

  void SetVisible(bool visible);
  bool GetVisible();

  void SetOpacity(float opacity);
  float GetOpacity();

  void SetName(const std::string &name);
  std::string GetName();

  void SetCenterOfMassIndex(const mitk::Point3D& center);
  mitk::Point3D GetCenterOfMassIndex();

  void SetCenterOfMassCoordinates(const mitk::Point3D& center);
  mitk::Point3D GetCenterOfMassCoordinates();

  void SetColor(const mitk::Color&);
  const mitk::Color& GetColor();

  void SetValue(int pixelValue);
  int GetValue();

  void SetLayer(int layer);
  int GetLayer();

  void SetProperty(const std::string &propertyKey, BaseProperty *property);

  using itk::Object::Modified;
  void Modified(){Superclass::Modified();}

  Label();
  virtual ~Label();

protected:

  void PrintSelf(std::ostream &os, itk::Indent indent) const;

  Label(const Label& other);

private:

  virtual itk::LightObject::Pointer InternalClone() const;

};

/**
* @brief Equal A function comparing two labels for beeing equal in data
*
* @ingroup MITKTestingAPI
*
* Following aspects are tested for equality:
*  - Lebel equality via Equal-PropetyList
*
* @param rightHandSide An image to be compared
* @param leftHandSide An image to be compared
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return true, if all subsequent comparisons are true, false otherwise
*/
MITK_CORE_EXPORT bool Equal( const mitk::Label& leftHandSide, const mitk::Label& rightHandSide, ScalarType eps, bool verbose );


} // namespace mitk

#endif // __mitkLabel_H_
