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

#ifndef MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2

#include "mitkBaseData.h"
#include "mitkGeometryData.h"
#include "mitkPlaneGeometry.h"
#include <MitkCoreExports.h>

namespace mitk
{
  class PlaneGeometryData;
  /** \deprecatedSince{2014_10} This class is deprecated. Please use PlaneGeometryData instead. */
  DEPRECATED(typedef PlaneGeometryData Geometry2DData);
  //##Documentation
  //## @brief Data class containing PlaneGeometry objects
  //## @ingroup Geometry
  //##
  class MITKCORE_EXPORT PlaneGeometryData : public GeometryData
  {
  public:
    mitkClassMacro(PlaneGeometryData, GeometryData);

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      //##Documentation
      //## @brief Set the reference to a PlaneGeometry that is stored
      //## by the object
      //##
      //## @warning Accepts only instances of PlaneGeometry or sub-classes.
      void SetGeometry(mitk::BaseGeometry *geometry) override;

    //##Documentation
    //## @brief Set the reference to the PlaneGeometry that is stored
    //## by the object
    virtual void SetPlaneGeometry(mitk::PlaneGeometry *geometry2d);
    /**
    * \deprecatedSince{2014_10} Please use SetPlaneGeometry
    */
    DEPRECATED(void SetGeometry2D(PlaneGeometry *geo)) { SetPlaneGeometry(geo); };
    //##Documentation
    //## @brief Get the reference to the PlaneGeometry that is stored
    //## by the object
    virtual mitk::PlaneGeometry *GetPlaneGeometry() const { return static_cast<mitk::PlaneGeometry *>(GetGeometry()); };
    /**
    * \deprecatedSince{2014_10} Please use GetPlaneGeometry
    */
    DEPRECATED(const PlaneGeometry *GetGeometry2D()) { return GetPlaneGeometry(); };
    void UpdateOutputInformation() override;

    void SetRequestedRegionToLargestPossibleRegion() override;

    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    bool VerifyRequestedRegion() override;

    void SetRequestedRegion(const itk::DataObject *data) override;

    void CopyInformation(const itk::DataObject *data) override;

  protected:
    PlaneGeometryData();

    ~PlaneGeometryData() override;
  };
} // namespace mitk
#endif /* MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2 */
