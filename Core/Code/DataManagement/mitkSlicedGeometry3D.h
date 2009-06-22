/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"

namespace mitk {

class SliceNavigationController;
class NavigationController;

/** \brief Describes the geometry of a data object consisting of slices.
 *
 * A Geometry2D can be requested for each slice. In the case of
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
 * SlicedGeometry3D and the associated Geometry2Ds have to be initialized in
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
class MITK_CORE_EXPORT SlicedGeometry3D : public mitk::Geometry3D
{
public:
  mitkClassMacro(SlicedGeometry3D, Geometry3D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /**
   * \brief Re-calculate the hull of the contained geometries.
   *
   * The transforms, bounding-box and time-bounds of this
   * geometry (stored in members of the super-class Geometry3D)
   * are re-calculated from the contained geometries.
   */
  void UpdateInformation();

  
  /**
   * \brief Returns the Geometry2D of the slice (\a s).
   *
   * If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
   * for the requested slice, and (c) the first slice (s=0) 
   * is a PlaneGeometry instance, then we calculate the geometry of the
   * requested as the plane of the first slice shifted by m_Spacing[3]*s
   * in the direction of m_DirectionVector.
   *
   * \warning The Geometry2Ds are not necessarily up-to-date and not even
   * initialized. 
   *
   * The Geometry2Ds have to be initialized in the method
   * GenerateOutputInformation() of BaseProcess (or CopyInformation /
   * UpdateOutputInformation of BaseData, if possible, e.g., by analyzing
   * pic tags in Image) subclasses. See also
   *
   * \sa itk::ProcessObject::GenerateOutputInformation(),
   * \sa itk::DataObject::CopyInformation() and
   * \sa itk::DataObject::UpdateOutputInformation().
   */
  virtual mitk::Geometry2D* GetGeometry2D( int s ) const;


  /**
   * \brief Set Geometry2D of slice \a s.
   */
  virtual bool SetGeometry2D( mitk::Geometry2D *geometry2D, int s );

  virtual void SetTimeBounds( const mitk::TimeBounds& timebounds );


  virtual const mitk::BoundingBox* GetBoundingBox() const;

  /**
   * \brief Get the number of slices
   */
  itkGetConstMacro( Slices, unsigned int );


  /**
   * \brief Check whether a slice exists
   */
  virtual bool IsValidSlice( int s = 0 ) const;

  virtual void SetReferenceGeometry( Geometry3D *referenceGeometry );

  /**
   * \brief Set the spacing (m_Spacing), in direction of the plane normal.
   *
   * INTERNAL METHOD.
   */
  virtual void SetSpacing( const mitk::Vector3D &aSpacing );

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
   * If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored for
   * the requested slice, and (c) the first slice (s=0) is a PlaneGeometry
   * instance, then we calculate the geometry of the requested as the plane
   * of the first slice shifted by m_Spacing.z * s in the direction of
   * m_DirectionVector.
   *
   * \sa GetGeometry2D
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

  virtual AffineGeometryFrame3D::Pointer Clone() const;

  static const std::string SLICES;
  const static std::string DIRECTION_VECTOR;
  const static std::string EVENLY_SPACED;

  /**
   * \brief Tell this instance how many Geometry2Ds it shall manage. Bounding
   * box and the Geometry2Ds must be set additionally by calling the respective
   * methods!
   *
   * \warning Bounding box and the 2D-geometries must be set additionally: use
   * SetBounds(), SetGeometry().
   */
  virtual void Initialize( unsigned int slices );

  /**
   * \brief Completely initialize this instance as evenly-spaced with slices
   * parallel to the provided Geometry2D that is used as the first slice and
   * for spacing calculation.
   *
   * Initializes the bounding box according to the width/height of the
   * Geometry2D and \a slices. The spacing is calculated from the Geometry2D.
   */
  virtual void InitializeEvenlySpaced( mitk::Geometry2D *geometry2D,
    unsigned int slices, bool flipped=false );

  /**
   * \brief Completely initialize this instance as evenly-spaced with slices
   * parallel to the provided Geometry2D that is used as the first slice and
   * for spacing calculation (except z-spacing).
   *
   * Initializes the bounding box according to the width/height of the
   * Geometry2D and \a slices. The x-/y-spacing is calculated from the
   * Geometry2D.
   */
  virtual void InitializeEvenlySpaced( mitk::Geometry2D *geometry2D,
    mitk::ScalarType zSpacing, unsigned int slices, bool flipped=false );

  /**
   * \brief Completely initialize this instance as evenly-spaced plane slices 
   * parallel to a side of the provided Geometry3D and using its spacing
   * information.
   *
   * Initializes the bounding box according to the width/height of the
   * Geometry3D and the number of slices according to
   * Geometry3D::GetExtent(2).
   *
   * \param planeorientation side parallel to which the slices will be oriented
   * \param top if \a true, create plane at top, otherwise at bottom 
   * (for PlaneOrientation Transversal, for other plane locations respectively)
   * \param frontside defines the side of the plane (the definition of
   * front/back is somewhat arbitrary)
   *
   * \param rotate rotates the plane by 180 degree around its normal (the
   * definition of rotated vs not rotated is somewhat arbitrary)
   */
  virtual void InitializePlanes( const mitk::Geometry3D *geometry3D,
    mitk::PlaneGeometry::PlaneOrientation planeorientation, bool top=true,
    bool frontside=true, bool rotated=false );


  virtual void SetImageGeometry(const bool isAnImageGeometry);

  virtual void ExecuteOperation(Operation* operation); 


protected:
  SlicedGeometry3D();

  virtual ~SlicedGeometry3D();

  
  /**
   * Reinitialize plane stack after rotation. More precisely, the first plane
   * of the stack needs to spatially aligned, in two respects:
   *
   * 1. Re-alignment with respect to the dataset center; this is necessary 
   *    since the distance from the first palne to the center could otherwise
   *    continuously decrease or increase.
   * 2. Re-alignment with respect to a given reference point; the reference
   *    point is a location which the user wants to be exactly touched by one
   *    plane of the plane stack. The first plane is minimally shifted to
   *    ensure this touching. Usually, the reference point would be the
   *    point around which the geometry is rotated.
   */
  virtual void ReinitializePlanes( const Point3D &center, 
    const Point3D &referencePoint );


  ScalarType GetLargestExtent( const Geometry3D *geometry );


  virtual void InitializeGeometry(Self * newGeometry) const;

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
  mutable std::vector<Geometry2D::Pointer> m_Geometry2Ds;


  /**
   * If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
   * for the requested slice, and (c) the first slice (s=0) 
   * is a PlaneGeometry instance, then we calculate the geometry of the
   * requested as the plane of the first slice shifted by m_Spacing.z*s
   * in the direction of m_DirectionVector.
   *
   * \sa GetGeometry2D
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

  /** Underlying Geometry3D for this SlicedGeometry */
  mitk::Geometry3D *m_ReferenceGeometry;

  /** SNC correcsponding to this geometry; used to reflect changes in the
   * number of slices due to rotation. */
  //mitk::NavigationController *m_NavigationController;
  mitk::SliceNavigationController *m_SliceNavigationController;
};

} // namespace mitk

#endif /* MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
