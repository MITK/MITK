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

#include "MitkMultilabelExports.h"
#include <mitkColorProperty.h>
#include <mitkPropertyList.h>
#include <mitkVector.h>

class TiXmlDocument;
class TiXmlElement;

namespace mitk
{
  //##
  //##Documentation
  //## @brief A data structure describing a label.
  //## @ingroup Data
  //##
  class MITKMULTILABEL_EXPORT Label : public PropertyList
  {
  public:
    mitkClassMacro(Label, mitk::PropertyList);

    itkNewMacro(Self);

    typedef unsigned short PixelType;

    /// The maximum value a label can get: Since the value is of type unsigned short MAX_LABEL_VALUE = 65535
    static const PixelType MAX_LABEL_VALUE;

    void SetLocked(bool locked);
    bool GetLocked() const;

    void SetVisible(bool visible);
    bool GetVisible() const;

    void SetOpacity(float opacity);
    float GetOpacity() const;

    void SetName(const std::string &name);
    std::string GetName() const;

    void SetCenterOfMassIndex(const mitk::Point3D &center);
    mitk::Point3D GetCenterOfMassIndex() const;

    void SetCenterOfMassCoordinates(const mitk::Point3D &center);
    mitk::Point3D GetCenterOfMassCoordinates() const;

    void SetColor(const mitk::Color &);
    const mitk::Color &GetColor() const;

    void SetValue(PixelType pixelValue);
    PixelType GetValue() const;

    void SetLayer(unsigned int layer);
    unsigned int GetLayer() const;

    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    using itk::Object::Modified;
    void Modified() { Superclass::Modified(); }
    Label();
    ~Label() override;

  protected:
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    Label(const Label &other);

  private:
    itk::LightObject::Pointer InternalClone() const override;
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
  MITKMULTILABEL_EXPORT bool Equal(const mitk::Label &leftHandSide,
                                   const mitk::Label &rightHandSide,
                                   ScalarType eps,
                                   bool verbose);

} // namespace mitk

#endif // __mitkLabel_H_
