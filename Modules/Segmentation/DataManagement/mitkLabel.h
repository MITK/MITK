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
  bool GetLocked() const;

  void SetVisible(bool visible);
  bool GetVisible() const;

  void SetOpacity(float opacity);
  float GetOpacity() const;

  void SetName(const std::string &name);
  std::string GetName() const;

  void SetCenterOfMassIndex(const mitk::Point3D& center);
  mitk::Point3D GetCenterOfMassIndex() const;

  void SetCenterOfMassCoordinates(const mitk::Point3D& center);
  mitk::Point3D GetCenterOfMassCoordinates() const;

  void SetColor(const mitk::Color&);
  const mitk::Color& GetColor() const;

  void SetValue(int pixelValue);
  int GetValue() const;

  void SetLayer(int layer);
  int GetLayer() const;

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

} // namespace mitk

#endif // __mitkLabel_H_
