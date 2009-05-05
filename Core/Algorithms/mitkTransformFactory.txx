/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTransformFactory.h"
#include "mitkTransformParameters.h"

#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkAffineTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkAzimuthElevationToCartesianTransform.h"
#include "itkCenteredTransformInitializer.h"
#include <itkRigid3DTransform.h>
#include <itkEuler3DTransform.h>
#include <itkCenteredEuler3DTransform.h>
#include <itkQuaternionRigidTransform.h>
#include <itkVersorTransform.h>
#include <itkVersorRigid3DTransform.h>
#include <itkScaleSkewVersor3DTransform.h>
#include <itkSimilarity3DTransform.h>
#include <itkRigid2DTransform.h>
#include <itkCenteredRigid2DTransform.h>
#include <itkEuler2DTransform.h>
#include <itkSimilarity2DTransform.h>
#include <itkCenteredSimilarity2DTransform.h>



namespace mitk {

  template < class TPixelType, unsigned int VImageDimension >
  TransformFactory< TPixelType, VImageDimension >::TransformFactory() : 
    m_TransformParameters(NULL),
    m_FixedImage(NULL),
    m_MovingImage(NULL),
    m_FixedImage2D(NULL),
    m_MovingImage2D(NULL),
    m_FixedImage3D(NULL),
    m_MovingImage3D(NULL)
  {

  }
  
  template < class TPixelType, unsigned int VImageDimension >
  void TransformFactory< TPixelType, VImageDimension >::SetFixedImage(FixedImageType* fixed)
  {
    if (VImageDimension == 2)
    {
      m_FixedImage2D = reinterpret_cast<FixedImage2DType*>(fixed);
      m_FixedImage = fixed;
    }
    else
    {
      m_FixedImage3D = reinterpret_cast<FixedImage3DType*>(fixed);;
      m_FixedImage = fixed;
    }
  }
  
  template < class TPixelType, unsigned int VImageDimension >
  void TransformFactory< TPixelType, VImageDimension >::SetMovingImage(MovingImageType* moving)
  {
    if (VImageDimension == 2)
    {
      m_MovingImage2D = reinterpret_cast<MovingImage2DType*>(moving);
      m_MovingImage = moving;
    }
    else
    {
      m_MovingImage3D = reinterpret_cast<MovingImage3DType*>(moving);
      m_MovingImage = moving;
    }
  }

  /*template < class TPixelType, unsigned int VImageDimension >
  typename ::itk::Transform< double, VImageDimension, VImageDimension >::Pointer 
  MakeRigidTransform();

  template <class TPixelType>
  typename ::itk::Transform< double, 3, 3 >::Pointer
  MakeRigidTransform<TPixelType, 3>()
  {
    itk::Rigid3DTransform< double >::Pointer transform = itk::Rigid3DTransform< double >::New();
    transform->SetIdentity();
    return transform.GetPointer();
  }

  template <class TPixelType>
  typename ::itk::Transform< double, 2, 2 >::Pointer
  MakeRigidTransform<TPixelType, 2>()
  {
    return NULL;
  }*/

  /*template < unsigned int VImageDimension >
  typename ::itk::Transform< double, VImageDimension, VImageDimension >::Pointer 
  MakeRigidTransform();

  template <>
  typename ::itk::Transform< double, 3, 3 >::Pointer
  MakeRigidTransform<3>()
  {
    itk::Rigid3DTransform< double >::Pointer transform = itk::Rigid3DTransform< double >::New();
    transform->SetIdentity();
    return transform.GetPointer();
  }

  template <>
  typename ::itk::Transform< double, 2, 2 >::Pointer
  MakeRigidTransform<2>()
  {
    return NULL;
  }*/
//
//template<class T, class U, int I> class X
//  { void f() { cout << "Primary template" << endl; } };
//template<class T> class X<int, T*, 10>
//  { void f() { cout << "Partial specialization 3" << endl;
//  } };
// 
//
//template < class TPixelType > class TransformFactory<TPixelType, 3>
//{
//  typedef typename itk::Transform< double, 3, 3 >    TransformType;
//  typedef typename TransformType::Pointer TransformPointer;
//
//  typename TransformPointer MakeRigidTransform()
//    {
//    }
//};


  //template <>
  //  typename TransformFactory< double, 3 >::TransformPointer
  //    TransformFactory< double, 3 >::MakeRigidTransform()
  //{
  //  itk::Rigid3DTransform< double >::Pointer transform = itk::Rigid3DTransform< double >::New();
  //  transform->SetIdentity();
  //  return transform.GetPointer();
  //}

  //template <>
  //  typename TransformFactory< double, 2 >::TransformPointer
  //    TransformFactory< double, 2 >::MakeRigidTransform()
  //{
  //  return NULL;
  //}

  template < class TPixelType, unsigned int VImageDimension >
    typename TransformFactory< TPixelType, VImageDimension >::TransformPointer
    TransformFactory< TPixelType, VImageDimension >::GetTransform()
  {
    int transform = m_TransformParameters->GetTransform();
    if (transform == TransformParameters::TRANSLATIONTRANSFORM)
    {
    typename itk::TranslationTransform< double, VImageDimension>::Pointer transformPointer = itk::TranslationTransform< double, VImageDimension>::New();
      transformPointer->SetIdentity();
    return transformPointer.GetPointer();
    }
    else if (transform == TransformParameters::SCALETRANSFORM)
    {
      typename itk::ScaleTransform< double, VImageDimension>::Pointer transformPointer = itk::ScaleTransform< double, VImageDimension>::New();
      transformPointer->SetIdentity();
      return transformPointer.GetPointer();
    }
    else if (transform == TransformParameters::SCALELOGARITHMICTRANSFORM)
    {
      typename itk::ScaleLogarithmicTransform< double, VImageDimension>::Pointer transformPointer = itk::ScaleLogarithmicTransform< double, VImageDimension>::New();
      transformPointer->SetIdentity();
      return transformPointer.GetPointer();
    }
    else if (transform == TransformParameters::AFFINETRANSFORM)
    {
      typename itk::AffineTransform< double, VImageDimension>::Pointer transformPointer = itk::AffineTransform< double, VImageDimension>::New();    
      transformPointer->SetIdentity();
      if (m_TransformParameters->GetTransformInitializerOn())
      {
        typedef typename itk::AffineTransform< double, VImageDimension >    AffineTransformType;
        typedef typename itk::CenteredTransformInitializer<AffineTransformType, FixedImageType, MovingImageType> TransformInitializerType;
        typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
        transformInitializer->SetFixedImage( m_FixedImage );
        transformInitializer->SetMovingImage( m_MovingImage );
        transformInitializer->SetTransform( transformPointer );
        if (m_TransformParameters->GetMomentsOn())
        {
          transformInitializer->MomentsOn();
        }
        else
        {
          transformInitializer->GeometryOn();
        }
        transformInitializer->InitializeTransform();        
      }
      m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
      m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
      m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
      return transformPointer.GetPointer();
    }
    else if (transform == TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      typedef typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension >    CenteredAffineTransformType;
      typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension>::Pointer transformPointer = itk::FixedCenterOfRotationAffineTransform< double, VImageDimension>::New();
      transformPointer->SetIdentity();
      if (m_TransformParameters->GetTransformInitializerOn())
      {
        typedef typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension >    FixedCenterOfRotationAffineTransformType;
        typedef typename itk::CenteredTransformInitializer<FixedCenterOfRotationAffineTransformType, FixedImageType, MovingImageType> TransformInitializerType;
        typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
        transformInitializer->SetFixedImage( m_FixedImage );
        transformInitializer->SetMovingImage( m_MovingImage );
        transformInitializer->SetTransform( transformPointer );
        if (m_TransformParameters->GetMomentsOn())
        {
          transformInitializer->MomentsOn();
        }
        else
        {
          transformInitializer->GeometryOn();
        }
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        transformInitializer->InitializeTransform();        
      }
      return transformPointer.GetPointer();
    }
    else if (transform == TransformParameters::RIGID3DTRANSFORM)
    {
      //return MakeRigidTransform<VImageDimension>();
      if (VImageDimension == 3)
      {
        typename itk::Rigid3DTransform< double >::Pointer transformPointer = itk::Rigid3DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::EULER3DTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::Euler3DTransform< double >::Pointer transformPointer = itk::Euler3DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::CENTEREDEULER3DTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::CenteredEuler3DTransform< double >::Pointer transformPointer = itk::CenteredEuler3DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::QUATERNIONRIGIDTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::QuaternionRigidTransform< double >::Pointer transformPointer = itk::QuaternionRigidTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::VERSORTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::VersorTransform< double >::Pointer transformPointer = itk::VersorTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::VersorRigid3DTransform< double >::Pointer transformPointer = itk::VersorRigid3DTransform< double >::New();
        transformPointer->SetIdentity();
        typedef typename itk::VersorRigid3DTransform< double >    VersorRigid3DTransformType;
        if (m_TransformParameters->GetTransformInitializerOn())
        {
          typedef typename itk::CenteredTransformInitializer<VersorRigid3DTransformType, FixedImage3DType, MovingImage3DType> TransformInitializerType;
          typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
          transformInitializer->SetFixedImage( m_FixedImage3D );
          transformInitializer->SetMovingImage( m_MovingImage3D );
          transformInitializer->SetTransform( transformPointer );
          if (m_TransformParameters->GetMomentsOn())
          {
            transformInitializer->MomentsOn();
          }
          else
          {
            transformInitializer->GeometryOn();
          }
          transformInitializer->InitializeTransform();
        }
        typedef VersorRigid3DTransformType::VersorType  VersorType;
        typedef VersorType::VectorType     VectorType;

        VersorType     rotation;
        VectorType     axis;
  
        axis[0] = 0.0;
        axis[1] = 0.0;
        axis[2] = 1.0;

        const double angle = 0;

        rotation.Set(  axis, angle  );

        transformPointer->SetRotation( rotation );
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::ScaleSkewVersor3DTransform< double >::Pointer transformPointer = itk::ScaleSkewVersor3DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::SIMILARITY3DTRANSFORM)
    {
      if (VImageDimension == 3)
      {
        typename itk::Similarity3DTransform< double >::Pointer transformPointer = itk::Similarity3DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        m_TransformParameters->SetTransformCenterZ(transformPointer->GetCenter()[2]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::RIGID2DTRANSFORM)
    {
      if (VImageDimension == 2)
      {
        typename itk::Rigid2DTransform< double >::Pointer transformPointer = itk::Rigid2DTransform< double >::New();
        transformPointer->SetIdentity();
        transformPointer->SetAngle( m_TransformParameters->GetAngle() );
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {
      if (VImageDimension == 2)
      {
        typename itk::CenteredRigid2DTransform< double >::Pointer transformPointer = itk::CenteredRigid2DTransform< double >::New();
        transformPointer->SetIdentity();
        if (m_TransformParameters->GetTransformInitializerOn())
        {
          typedef typename itk::CenteredRigid2DTransform< double >    CenteredRigid2DTransformType;
          typedef typename itk::CenteredTransformInitializer<CenteredRigid2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
          typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
          transformInitializer->SetFixedImage( m_FixedImage2D );
          transformInitializer->SetMovingImage( m_MovingImage2D );
          transformInitializer->SetTransform( transformPointer );
          if (m_TransformParameters->GetMomentsOn())
          {
            transformInitializer->MomentsOn();
          }
          else
          {
            transformInitializer->GeometryOn();
          }
        transformInitializer->InitializeTransform();
        }
        transformPointer->SetAngle( m_TransformParameters->GetAngle() );
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::EULER2DTRANSFORM)
    {
      if (VImageDimension == 2)
      {
        typename itk::Euler2DTransform< double >::Pointer transformPointer = itk::Euler2DTransform< double >::New();
        transformPointer->SetIdentity();
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::SIMILARITY2DTRANSFORM)
    {
      if (VImageDimension == 2)
      {
        typename itk::Similarity2DTransform< double >::Pointer transformPointer = itk::Similarity2DTransform< double >::New();
        transformPointer->SetIdentity();
        transformPointer->SetScale( m_TransformParameters->GetScale() );
        transformPointer->SetAngle( m_TransformParameters->GetAngle() );
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    else if (transform == TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      if (VImageDimension == 2)
      {
        typename itk::CenteredSimilarity2DTransform< double >::Pointer transformPointer = itk::CenteredSimilarity2DTransform< double >::New();
        transformPointer->SetIdentity();
        if (m_TransformParameters->GetTransformInitializerOn())
        {
          typedef typename itk::CenteredSimilarity2DTransform< double >    CenteredSimilarity2DTransformType;
          typedef typename itk::CenteredTransformInitializer<CenteredSimilarity2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
          typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
          transformInitializer->SetFixedImage( m_FixedImage2D );
          transformInitializer->SetMovingImage( m_MovingImage2D );
          transformInitializer->SetTransform( transformPointer );
          if (m_TransformParameters->GetMomentsOn())
          {
            transformInitializer->MomentsOn();
          }
          else
          {
            transformInitializer->GeometryOn();
          }
          transformInitializer->InitializeTransform();        
        }
        transformPointer->SetScale( m_TransformParameters->GetScale() );
        transformPointer->SetAngle( m_TransformParameters->GetAngle() );
        m_TransformParameters->SetTransformCenterX(transformPointer->GetCenter()[0]);
        m_TransformParameters->SetTransformCenterY(transformPointer->GetCenter()[1]);
        return reinterpret_cast<TransformType*>(transformPointer.GetPointer());
      }
    }
    return NULL;
  }
} // end namespace
