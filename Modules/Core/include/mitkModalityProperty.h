/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModalityProperty_h
#define mitkModalityProperty_h

#include <mitkEnumerationProperty.h>
#include <MitkCoreExports.h>
#include <itkObjectFactory.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Enumeration property for known imaging modalities.
   *
   * Pre-populates the enumeration with standard medical imaging modalities
   * (e.g., CT, MR, US, XA, etc.) in its constructor. Can be initialized
   * by modality name or by id.
   *
   * \ingroup DataManagement
   *
   * \sa EnumerationProperty
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT ModalityProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(ModalityProperty, EnumerationProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(ModalityProperty, const IdType&);
    mitkNewMacro1Param(ModalityProperty, const std::string &);

    using BaseProperty::operator=;

  protected:
    /** \brief Default constructor. Populates the enumeration with standard modality types. */
    ModalityProperty();

    /** \brief Copy constructor. */
    ModalityProperty(const ModalityProperty &other);

    /**
     * \brief Construct with a modality id.
     *
     * If the given id is not valid, the modality defaults to the first entry (undefined).
     *
     * \param value The enumeration id of the modality.
     */
    ModalityProperty(const IdType &value);

    /**
     * \brief Construct with a modality name string.
     *
     * If the given string is not a valid modality name, the modality defaults to "undefined".
     *
     * \param value The modality name string (e.g. "CT", "MR", "US").
     */
    ModalityProperty(const std::string &value);

    ~ModalityProperty() override;

    /** \brief Populates the enumeration with standard DICOM modality types (CT, MR, US, etc.). */
    virtual void AddEnumerationTypes();

    mitkCloneMacro(ModalityProperty);

  private:
    // purposely not implemented
    const ModalityProperty &operator=(const ModalityProperty &);
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace

#endif
