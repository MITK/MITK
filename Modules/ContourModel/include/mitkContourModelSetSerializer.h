/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetSerializer_h
#define mitkContourModelSetSerializer_h

#include <MitkContourModelExports.h>
#include <mitkBaseDataSerializer.h>

namespace mitk
{
  /** \brief Serializer for mitk::ContourModelSet objects.
   *
   * Writes a ContourModelSet to a .cnt_set file using ContourModelSetWriter.
   * Used by the scene serialization framework to persist contour set data.
   *
   * \sa ContourModelSet, BaseDataSerializer, ContourModelSerializer
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSetSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ContourModelSetSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Serialize the associated ContourModelSet to a .cnt_set file.
       * \return The filename of the serialized file, or an empty string on failure.
       */
      std::string Serialize() override;

  protected:
    ContourModelSetSerializer();
    ~ContourModelSetSerializer() override;
  };
}

#endif
