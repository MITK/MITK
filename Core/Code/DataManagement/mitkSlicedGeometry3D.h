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

#ifndef MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkBaseGeometry.h"
#include "mitkPlaneGeometry.h"

namespace mitk {
  class SliceNavigationController;
  class NavigationController;

  /** \brief Describes the geometry of a data object consisting of slices.
  *
  * A PlaneGeometry can be requested for each slice. In the case of
  * \em evenly-spaced, \em plane geometries (m_EvenlySpaced==true),
  * only the 2D-geometry of the first slice has to be set (to an instance of
  * PlaneGeometry). The 2D geometries of the other slices are calculated
  * by shifting the first slice in the direction m_DirectionVector by
  * m_Spacing.z * sliceNumber. The m_Spacing member (which is only
  * relevant in the case m_EvenlySpaced==true) descibes the size of a voxel
  * (in mm), i.e., m_Spacing.x is the voxel width in the x-direction of the
  * plane. It is derived from the reference geometry of this SlicedGeometry3D,
  * which usually would be the global geometry describing how datasets are to
  * be resliced.
  *
  * By default, slices are oriented in the direction of one of the main axes
  * (x, y, z). However, by means of rotation, it is possible to realign the
  * slices in any possible direction. In case of an inclined plane, the spacing
  * is derived as a product of the (regular) geometry spacing and the direction
  * vector of the plane.
  *
  * SlicedGeometry3D and the associated PlaneGeometries have to be initialized in
  * the method GenerateOutputInformation() of BaseProcess (or CopyInformation /
  * UpdateOutputInformation of BaseData, if possible, e.g., by analyzing pic
  * tags in Image) subclasses. See also
  *
  * \sa itk::ProcessObject::GenerateOutputInformation(),
  * \sa itk::DataObject::CopyInformation() and
  * \a itk::DataObject::UpdateOutputInformation().
  *
  * Rule: everything is in mm (or ms for temporal information) if not
  * stated otherwise.
  *
  * \warning The hull (i.e., transform, bounding-box and
  * time-bounds) is only guaranteed to be up-to-date after calling
  * UpdateInformation().
  *
  * \ingroup Geometry
  */
  class MITK_CORE_EXPORT SlicedGeometry3D : public mitk::BaseGeometry
  {
  public:
    mitkClassMacro(SlicedGeometry3D, BaseGeometry);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
      * \brief Returns the PlaneGeometry of the slice (\a s).
      *
      * If (a) m_EvenlySpaced==true, (b) we don't have a PlaneGeometry stored
      * for the requested slice, and (c) the first slice (s=0)
      * is a PlaneGeometry instance, then we calculate the geometry of the
      * requested as the plane of the first slice shifted by m_Spacing[3]*s
      * in the direction of m_DirectionVector.
      *
      * \warning The PlaneGeometries are not necessarily up-to-date and not even
      * initialized.
      *
      * The PlaneGeometries have to be initialized in the method
      * GenerateOutputInformation() of BaseProcess (or CopyInformation /
      * UpdateOutputInformation of BaseData, if possible, e.g., by analyzing
      * pic tags in Image) subclasses. See also
      *
      * \sa itk::ProcessObject::GenerateOutputInformation(),
      * \sa itk::DataObject::CopyInformation() and
      * \sa itk::DataObject::UpdateOutputInformation().
      */
      virtual mitk::PlaneGeometry* GetPlaneGeometry( int s ) const;
        /**
    * \deprecatedSince{2014_06} Please use GetPlaneGeometry
    */
    DEPRECATED(const PlaneGeometry* GetGeometry2D(int s)){return GetPlaneGeometry(s);};


    /**
    * \deprecatedSince{2014_06} Please use SetPlaneGeometry
    */
    DEPRECATED(void SetGeometry2D(PlaneGeometry* geo, int s)){SetPlaneGeometry(geo, s);};

    //##Documentation
    //## @brief When switching from an Image Geometry to a normal Geometry (and the other way around), you have to change the origin as well (See Geometry Documentation)! This function will change the "isImageGeometry" bool flag and changes the origin respectively.
    virtual void ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry );

  //virtual void SetTimeBounds( const mitk::TimeBounds& timebounds );
    virtual const mitk::BoundingBox* GetBoundingBox() const;

    /**
    * \brief Get the number of slices
    */
    itkGetConstMacro( Slices, unsigned int );

    /**
    * \brief Set PlaneGeometry of slice \a s.
    */
    virtual bool SetPlaneGeometry( mitk::PlaneGeometry *geometry2D, int s );


    /**
    * \brief Check whether a slice exists
    */
    virtual bool IsValidSlice( int s = 0 ) const;

    virtual void SetReferenceGeometry( BaseGeometry *referenceGeometry );

    /**
    * \brief Set the SliceNavigationController corresponding to this sliced
    * geometry.
    *
    * The SNC needs to be informed when the number of slices in the geometry
    * changes, which can occur whenthe slices are re-oriented by rotation.
    */
    virtual void SetSliceNavigationController(
      mitk::SliceNavigationController *snc );
    mitk::SliceNavigationController *GetSliceNavigationController();

    /**
    * \brief Set/Get whether the SlicedGeometry3D is evenly-spaced
    * (m_EvenlySpaced)
    *
    * If (a) m_EvenlySpaced==true, (b) we don't have a PlaneGeometry stored for
    * the requested slice, and (c) the first slice (s=0) is a PlaneGeometry
    * instance, then we calculate the geometry of the requested as the plane
    * of the first slice shifted by m_Spacing.z * s in the direction of
    * m_DirectionVector.
    *
    * \sa GetPlaneGeometry
    */
    itkGetConstMacro(EvenlySpaced, bool);

    virtual void SetEvenlySpaced(bool on = true);

    /**
    * \brief Set/Get the vector between slices for the evenly-spaced case
    * (m_EvenlySpaced==true).
    *
    * If the direction-vector is (0,0,0) (the default) and the first
    * 2D geometry is a PlaneGeometry, then the direction-vector will be
    * calculated from the plane normal.
    *
    * \sa m_DirectionVector
    */
    virtual void SetDirectionVector(const mitk::Vector3D& directionVector);
    itkGetConstMacro(DirectionVector, const mitk::Vector3D&);

    virtual itk::LightObject::Pointer InternalClone() const;

    static const std::string SLICES;
    const static std::string DIRECTION_VECTOR;
    const static std::string EVENLY_SPACED;

    /**
    * \brief Tell this instance how many PlaneGeometries it shall manage. Bounding
    * box and the PlaneGeometries must be set additionally by calling the respective
    * methods!
    *
    * \warning Bounding box and the 2D-geometries must be set additionally: use
    * SetBounds(), SetGeometry().
    */
    virtual void InitializeSlicedGeometry( unsigned int slices );

    /**
    * \brief Completely initialize this instance as evenly-spaced with slices
    * parallel to the provided PlaneGeometry that is used as the first slice and
    * for spacing calculation.
    *
    * Initializes the bounding box according to the width/height of the
    * PlaneGeometry and \a slices. The spacing is calculated from the PlaneGeometry.
    */
    virtual void InitializeEvenlySpaced( mitk::PlaneGeometry *geometry2D,
      unsigned int slices, bool flipped=false );

    /**
    * \brief Completely initialize this instance as evenly-spaced with slices
    * parallel to the provided PlaneGeometry that is used as the first slice and
    * for spacing calculation (except z-spacing).
    *
    * Initializes the bounding box according to the width/height of the
    * PlaneGeometry and \a slices. The x-/y-spacing is calculated from the
    * PlaneGeometry.
    */
    virtual void InitializeEvenlySpaced( mitk::PlaneGeometry *geometry2D,
      mitk::ScalarType zSpacing, unsigned int slices, bool flipped=false );

    /**
    * \brief Completely initialize this instance as evenly-spaced plane slices
    * parallel to a side of the provided BaseGeometry and using its spacing
    * information.
    *
    * Initializes the bounding box according to the width/height of the
    * BaseGeometry and the number of slices according to
    * BaseGeometry::GetExtent(2).
    *
    * \param planeorientation side parallel to which the slices will be oriented
    * \param top if \a true, create plane at top, otherwise at bottom
    * (for PlaneOrientation Axial, for other plane locations respectively)
    * \param frontside defines the side of the plane (the definition of
    * front/back is somewhat arbitrary)
    *
    * \param rotate rotates the plane by 180 degree around its normal (the
    * definition of rotated vs not rotated is somewhat arbitrary)
    */
    virtual void InitializePlanes( const mitk::BaseGeometry *geometry3D,
      mitk::PlaneGeometry::PlaneOrientation planeorientation, bool top=true,
      bool frontside=true, bool rotated=false );

    virtual void SetImageGeometry(const bool isAnImageGeometry);

    virtual void ExecuteOperation(Operation* operation);

    static double CalculateSpacing( const mitk::Vector3D spacing, const mitk::Vector3D &d );

  protected:
    SlicedGeometry3D();

    SlicedGeometry3D(const SlicedGeometry3D& other);

    virtual ~SlicedGeometry3D();

    /**
    * Reinitialize plane stack after rotation. More precisely, the first plane
    * of the stack needs to spatially aligned, in two respects:
    *
    * 1. Re-alignment with respect to the dataset center; this is necessary
    *    since the distance from the first plane to the center could otherwise
    *    continuously decrease or increase.
    * 2. Re-alignment with respect to a given reference point; the reference
    *    point is a location which the user wants to be exactly touched by one
    *    plane of the plane stack. The first plane is minimally shifted to
    *    ensure this touching. Usually, the reference point would be the
    *    point around which the geometry is rotated.
    */
    virtual void ReinitializePlanes( const Point3D &center,
      const Point3D &referencePoint );

    ScalarType GetLargestExtent( const BaseGeometry *geometry );

    void PrintSelf(std::ostream& os, itk::Indent indent) const;

    /** Calculate "directed spacing", i.e. the spacing in directions
    * non-orthogonal to the coordinate axes. This is done via the
    * ellipsoid equation.
    */
    double CalculateSpacing( const mitk::Vector3D &direction ) const;

    /** The extent of the slice stack, i.e. the number of slices, depends on the
    * plane normal. For rotated geometries, the geometry's transform needs to
    * be accounted in this calculation.
    */
    mitk::Vector3D AdjustNormal( const mitk::Vector3D &normal ) const;

    /**
    * Container for the 2D-geometries contained within this SliceGeometry3D.
    */
    mutable std::vector<PlaneGeometry::Pointer> m_PlaneGeometries;

    /**
    * If (a) m_EvenlySpaced==true, (b) we don't have a PlaneGeometry stored
    * for the requested slice, and (c) the first slice (s=0)
    * is a PlaneGeometry instance, then we calculate the geometry of the
    * requested as the plane of the first slice shifted by m_Spacing.z*s
    * in the direction of m_DirectionVector.
    *
    * \sa GetPlaneGeometry
    */
    bool m_EvenlySpaced;

    /**
    * Vector between slices for the evenly-spaced case (m_EvenlySpaced==true).
    * If the direction-vector is (0,0,0) (the default) and the first
    * 2D geometry is a PlaneGeometry, then the direction-vector will be
    * calculated from the plane normal.
    */
    mutable mitk::Vector3D m_DirectionVector;

    /** Number of slices this SliceGeometry3D is descibing. */
    unsigned int m_Slices;

    /** Underlying BaseGeometry for this SlicedGeometry */
    mitk::BaseGeometry *m_ReferenceGeometry;

    /** SNC correcsponding to this geometry; used to reflect changes in the
    * number of slices due to rotation. */
    //mitk::NavigationController *m_NavigationController;
    mitk::SliceNavigationController *m_SliceNavigationController;
  private:

    virtual void PreSetBounds(const BoundsArrayType& /*bounds*/){};

        /**
    * \brief Set the spacing (m_Spacing), in direction of the plane normal.
    *
    */
    virtual void PreSetSpacing( const mitk::Vector3D &aSpacing );
  };
} // namespace mitk

#endif /* MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
