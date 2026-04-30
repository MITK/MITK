/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkResliceMethodProperty_h
#define mitkResliceMethodProperty_h

#include <mitkEnumerationProperty.h>

namespace mitk
{
  /**
   * \brief Encapsulates the thick slices reslice method as an enumeration property.
   *
   * Valid enumeration values are:
   * - "disabled" (0)
   * - "mip" (1) -- Maximum Intensity Projection
   * - "sum" (2)
   * - "weighted" (3)
   * - "minip" (4) -- Minimum Intensity Projection
   * - "mean" (5)
   *
   * \sa EnumerationProperty
   * \ingroup DataManagement
   */
  class MITKCORE_EXPORT ResliceMethodProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(ResliceMethodProperty, EnumerationProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(ResliceMethodProperty, const IdType&);
    mitkNewMacro1Param(ResliceMethodProperty, const std::string &);

    using BaseProperty::operator=;

  protected:
    /** \brief Default constructor. Sets reslice method to "disabled". */
    ResliceMethodProperty();

    /** \brief Copy constructor. */
    ResliceMethodProperty(const ResliceMethodProperty &other);

    /** \brief Constructor. Sets reslice method to the given enumeration value.
     *
     * \param value the enumeration ID of the reslice method.
     */
    ResliceMethodProperty(const IdType &value);

    /** \brief Constructor. Sets reslice method to the given string value.
     *
     * \param value the string name of the reslice method.
     */
    ResliceMethodProperty(const std::string &value);

    /** \brief Adds the available thick slices reslice method types to the enumeration. */
    void AddThickSlicesTypes();

    mitkCloneMacro(ResliceMethodProperty);

  private:
    // purposely not implemented
    ResliceMethodProperty &operator=(const ResliceMethodProperty &);
  };


} // end of namespace mitk

#endif
