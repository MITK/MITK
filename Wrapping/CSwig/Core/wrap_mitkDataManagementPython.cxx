#define MITK_WRAP_PACKAGE "mitkDataManagementPython"
//#include "wrap_MITKAlgorithms.cxx"
/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: wrap_ITKAlgorithms.cxx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifdef CABLE_CONFIGURATION

namespace _cable_
{
  const char* const package = MITK_WRAP_PACKAGE;
  const char* const groups[] =
  {
    //"VtkAbstractTransform",
      "AbstractTransformGeometry",
      "AnnotationProperty",
      "ApplicationCursor",
      "BaseData",
      "BaseDataImplementation",
      "BaseProperty",
      "ClippingProperty",
      "ColorProperty",
      //"Common",
      "DataNode",
      "DataStorage",
      "DisplayGeometry",
      "EnumerationProperty",
      "GenericLookupTable",
      //"GenericProperty",
      "Geometry2D",
      "Geometry2DData",
      "Geometry3D",
      "GeometryData",
      "GroupTagProperty",
      "Image",
      "ImageDataItem",
      //"ItkMatirxHack",
      "LandmarkBasedCurvedGeometry",
      "LandmarkProjector",
      "LandmarkProjectorBasedCurvedGeometry",
      "LevelWindow",
      "LevelWindowManager",
      "LevelWindowPreset",
      "LevelWindowProperty",
      //Line",
      "LookupTable",
      //"LookupTables",
      "Material",
      //"MatrixConvert",
      "ModalityProperty",
      "NodePredicateAND",
      "NodePredicateBase",
      "NodePredicateCompositeBase",
	  "NodePredicateData",
      "NodePredicateDataType",
      "NodePredicateDimension",
      "NodePredicateFirstLevel",
      "NodePredicateNOT",
      "NodePredicateOR",
      "NodePredicateProperty",
      "NodePredicateSource",
      "PlaneDecorationProperty",
      "PlaneGeometry",
      "PlaneOperation",
      //"PointOperation",
      "PointSet",
      //"Properties",
      "PropertyList",
      "ResliceMethodEnumProperty",
      "RotationOperation",
      "ShaderEnumProperty",
      "SlicedData",
      //"SlicedGeometry3D",
      "SmartPointerProperty",
      "StandaloneDataStorage",
      "StateTransitionOperation",
      "StringProperty",
      "Surface",
      "ThinPlateSplineCurvedGeometry",
      "TimeSlicedGeometry",
      "TransferFunction",
      "TransferFunctionProperty",
      //"Vector",
      "VtkInterpolationProperty",
      "VtkRepresentationProperty",
      "VtkResliceInterpolationProperty",
      "VtkScalarModeProperty",
      "VtkVolumeRenderingProperty",
      //"WeakPointer",
      "WeakPointerProperty",
  };
}
#endif
