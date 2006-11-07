
#if !defined(MITK_PLANEFIT_H__INCLUDED_)
#define MITK_PLANEFIT_H__INCLUDED_

#include <mitkPointSet.h>
#include <mitkPlaneGeometry.h>
#include "mitkGeometryDataSource.h"

namespace mitk {

  //!  
  //  kind regards to dr. math!
  // function [x0, a, d, normd] = lsplane(X)
  // ---------------------------------------------------------------------
  // LSPLANE.M   Least-squares plane (orthogonal distance
  //             regression).
  //
  // Version 1.0    
  // Last amended   I M Smith 27 May 2002. 
  // Created        I M Smith 08 Mar 2002
  // ---------------------------------------------------------------------
  // Input    
  // X        Array [x y z] where x = vector of x-coordinates, 
  //          y = vector of y-coordinates and z = vector of 
  //          z-coordinates. 
  //          Dimension: m x 3. 
  // 
  // Output   
  // x0       Centroid of the data = point on the best-fit plane.
  //          Dimension: 3 x 1. 
  // 
  // a        Direction cosines of the normal to the best-fit 
  //          plane. 
  //          Dimension: 3 x 1.
  // 
  // <Optional... 
  // d        Residuals. 
  //          Dimension: m x 1. 
  // 
  // normd    Norm of residual errors. 
  //          Dimension: 1 x 1. 
  // ...>
  //
  // [x0, a <, d, normd >] = lsplane(X)
  // ---------------------------------------------------------------------

  class PlaneFit : public GeometryDataSource
  {
    public:

      mitkClassMacro( PlaneFit, GeometryDataSource);
      itkNewMacro(Self);

      typedef mitk::PointSet::PointDataType PointDataType;
      typedef mitk::PointSet::PointDataIterator PointDataIterator;

      virtual void GenerateOutputInformation();

      virtual void GenerateData();

      /*!Getter for point set.
       *
       */
      const mitk::PointSet *GetInput(void);

      /*! filter initialisation.
       *
       */
      virtual void SetInput(const mitk::PointSet* ps);

      /*! returns the center of gravity of the point set.
       *
       */
      virtual const mitk::Point3D &GetCentroid() const;

      /*! returns the plane geometry which represents the point set.
       *
       */
      virtual mitk::PlaneGeometry::Pointer GetPlaneGeometry();

      /*! returns the normal of the plane which represents the point set.
       *
       */
      virtual const mitk::Vector3D &GetPlaneNormal() const;

    protected:
      PlaneFit();
      virtual ~PlaneFit();

      /*! Calculates the centroid of the point set.
       * the center of gravity is calculated  through the mean value of the whole point set
       */
      void CalculateCentroid();

      /*! working with an SVD algorithm form matrix dataM.
       * ITK suplies the vnl_svd to solve an plan fit eigentvector problem
       * points are processed in the SVD matrix. The normal vector is the
       * singular vector of dataM corresponding to its smalest singular value.
       * The mehtod uses VNL library from ITK and at least the mehtod nullvector()
       * to extract the normalvector.
       */
      void ProcessPointSet();

      /*! Initialize Plane and configuration.
       *
       */
      void InitializePlane();


    private:

      /*!keeps a copy of the pointset.*/
      const mitk::PointSet* m_PointSet;
      /* output object - a plane geometry.*/
      mitk::PlaneGeometry::Pointer m_Plane;
      /*! the calculatet center point of all points in the point set.*/
      mitk::Point3D m_Centroid;
      /* the normal vector to descrie a plane gemoetry.*/
      mitk::Vector3D m_PlaneVector;
  };
}//namespace mitk
#endif //MITK_PLANFIT_INCLUDE_

