/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataSource_h
#define mitkGeometryDataSource_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkBaseDataSource.h>

namespace mitk
{
  class GeometryData;

  /**
   * \brief Superclass of all classes generating GeometryData as output.
   *
   * This class serves as the base for process objects that produce
   * mitk::GeometryData instances. In ITK and VTK, the generated result of a
   * ProcessObject is only guaranteed to be up-to-date when Update() is called
   * immediately before accessing the data. This also applies to subclasses of
   * this class.
   *
   * \sa GeometryData
   * \sa PlaneFit
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT GeometryDataSource : public BaseDataSource
  {
  public:
    mitkClassMacro(GeometryDataSource, BaseDataSource);
    itkNewMacro(Self);

      /** \brief The output type of this source. */
      typedef mitk::GeometryData OutputType;

    mitkBaseDataSourceGetOutputDeclarations

      /**
       * \brief Allocates a new output object and returns it.
       *
       * Currently the index \p idx is not evaluated; a GeometryData is always created.
       *
       * \param[in] idx The index of the output for which an object should be created.
       * \return The newly created GeometryData object.
       */
      itk::DataObject::Pointer
      MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
     * \brief Allocates a new output object by name.
     *
     * Default implementation to ensure subclasses have a valid MakeOutput.
     *
     * \param[in] name The identifier of the output.
     * \return The newly created GeometryData object.
     */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  protected:
    GeometryDataSource();
    ~GeometryDataSource() override;
  };

} // namespace mitk

#endif
