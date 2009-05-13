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

#ifndef _MITK_POINT_LOCATOR__H__
#define _MITK_POINT_LOCATOR__H__

#include <itkObject.h>

#include "mitkPointSet.h"

#include <vtkPoints.h>


//forward declarations
class vtkPointSet;
class ANNkd_tree;

namespace mitk
{  
  
/**
 * Convenience wrapper around ANN to provide fast nearest neighbour searches.
 * Usage: set your points via SetPoints( vtkPointSet* Points ) or SetPoints(mitk::PointSet*).
 * Then, you may query the closest point to an arbitrary coordinate by FindClosestPoint(). 
 * There is no further call to update etc. needed.
 * Currently only calls for 1 nearest neighbour are supported. Feel free to add functions
 * for K nearest neighbours.
 * NOTE: At least 1 point must be contained in the point set.
 */

class MITKEXT_CORE_EXPORT PointLocator : public itk::Object
{
public:
    
  mitkClassMacro( PointLocator, Object );

  itkNewMacro( Self );
  
  typedef int IdType;
  typedef float DistanceType;

  /**
   * Sets the point which will be used for nearest-neighbour searches. Note
   * there must be at least one point in the point set.
   * @param points the point set containing points for nearest neighbours searches.
   */
  void SetPoints( vtkPointSet* points );
  
  /**
   * Sets the point which will be used for nearest-neighbour searches. Note
   * there must be at least one point in the point set.
   * @param points the point set containing points for nearest neighbours searches.
   */
  void SetPoints( mitk::PointSet* points );
  
  /**
   * Finds the nearest neighbour in the point set previously defined by SetPoints().
   * The Id of the point is returned. Please note, that there is no case, in which
   * no point is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param point the query point, for whom the nearest neighbour will be determined
   * @returns the id of the nearest neighbour of the given point. The id corresponds to the id
   * which is given in the original point set.
   */
  IdType FindClosestPoint( const vtkFloatingPointType point[3] );
  
  /**
   * Finds the nearest neighbour in the point set previously defined by SetPoints().
   * The Id of the point is returned. Please note, that there is no case, in which
   * no point is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param x the x coordinated of the query point, for whom the nearest neighbour will be determined
   * @param y the x coordinated of the query point, for whom the nearest neighbour will be determined
   * @param z the x coordinated of the query point, for whom the nearest neighbour will be determined
   * @returns the id of the nearest neighbour of the given point. The id corresponds to the id
   * which is given in the original point set.
   */
  IdType FindClosestPoint( vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z );
  
  /**
   * Finds the nearest neighbour in the point set previously defined by SetPoints().
   * The Id of the point is returned. Please note, that there is no case, in which
   * no point is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param point the query point, for whom the nearest neighbour will be determined
   * @returns the id of the nearest neighbour of the given point. The id corresponds to the id
   * which is given in the original point set.
   */
  IdType FindClosestPoint( mitk::PointSet::PointType point );

  /**
   * Finds the nearest neighbour in the point set previously defined by SetPoints().
   * The minimal distance between this point and the closest point of the point set is returned. 
   * Please note, that there is no case, in which
   * no point is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param point the query point, for whom the minimal distancew will be determined
   * @returns the distance in world coordinates between the nearest point in point set and the given point
   */
  DistanceType GetMinimalDistance( mitk::PointSet::PointType point );

  /**
  * Finds the nearest neighbour in the point set previously defined by SetPoints().
  * The Index and minimal distance between this point and the closest point of the point set is returned. 
  * Please note, that there is no case, in which
  * no point is found, since as a precondition at least one point has to be contained
  * in the point set.
  * @param point the query point, for whom the minimal distancew will be determined
  * @returns the index of and distance (in world coordinates) between the nearest point in point set and the given point
  */
  bool FindClosestPointAndDistance( mitk::PointSet::PointType point, IdType* id, DistanceType* dist);
  
protected: 
  //
  // Definition of a vector of ids
  //
  typedef std::vector<IdType> IdVectorType;

  //
  // ANN related typedefs, to prevent ANN from being in the global include path.
  // Please note, that these line are prone to failure, if the point type in
  // ANN changes. Please note also, that these typedefs are only used in the header
  // file. The implementation always refers to the original types
  //
  typedef float*      MyANNpoint;
  typedef int         MyANNidx;
  typedef float       MyANNdist;
  typedef MyANNpoint* MyANNpointArray;
  typedef MyANNidx*   MyANNidxArray;
  typedef MyANNdist*  MyANNdistArray;
  
  /** 
   * constructor
   */
  PointLocator();
  
  /**
   * destructor
   */
  ~PointLocator();
  
  /**
   * Initializes the ann search tree using previously defined points
   */
  void InitANN();
  
  /**
   * releases all memory occupied by the ann search tree and internal point set arrays
   */
  void DestroyANN();  
  
  /**
   * Finds the nearest neighbour in the point set previously defined by SetPoints().
   * The Id of the point is returned. Please note, that there is no case, in which
   * no point is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param point the query point, for whom the nearest neighbour will be determined
   * @returns the id of the nearest neighbour of the given point. The id corresponds to the id
   * which is given in the original point set.
   */
  IdType FindClosestPoint( const MyANNpoint& point);
  
  /**
   * Finds the minimal distance between the given point and a point in the previously defined point set.
   * The distance is returned. Please note, that there is no case, in which
   * no distance is found, since as a precondition at least one point has to be contained
   * in the point set.
   * @param point the query point, for whom the minimal distance to a point in the previously defined point set will be determined
   * @returns the distance in world coordinates between the given point and the nearest neighbour. 
   */
  DistanceType GetMinimalDistance( const MyANNpoint& point);
  
  bool m_SearchTreeInitialized;
  
  IdVectorType m_IndexToPointIdContainer;

  vtkPoints*       m_VtkPoints;
  
  
  //
  // ANN related variables
  //
  unsigned int     m_ANNK;
  unsigned int     m_ANNDimension;
  double           m_ANNEpsilon;
  MyANNpointArray  m_ANNDataPoints;
  MyANNpoint       m_ANNQueryPoint;
  MyANNidxArray    m_ANNPointIndexes;
  MyANNdistArray   m_ANNDistances;
  ANNkd_tree*      m_ANNTree;
};

}

#endif
