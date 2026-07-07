/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSerializer_h
#define mitkContourModelSerializer_h

#include <MitkContourModelExports.h>
#include <mitkBaseDataSerializer.h>

namespace mitk
{
  /** \brief Serializer for mitk::ContourModel objects.
   *
   * Writes a ContourModel to a .cnt file using ContourModelWriter.
   * Used by the scene serialization framework to persist contour data.
   *
   * \sa ContourModel, BaseDataSerializer, ContourModelSetSerializer
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ContourModelSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Serialize the associated ContourModel to a .cnt file.
       * \return The filename of the serialized file, or an empty string on failure.
       */
      std::string Serialize() override;

  protected:
    ContourModelSerializer();
    ~ContourModelSerializer() override;
  };
}

#endif
