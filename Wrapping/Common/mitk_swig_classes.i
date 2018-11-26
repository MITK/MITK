
//
// Defining some Macros that make problems with SWIG as the
// corresponding definitions are not included by default.
// Luckely, these includes are not necessary for SWIG.
//
#define ITK_NOEXCEPT
#define ITKCommon_EXPORT
#define ITK_OVERRIDE
#define MITKCORE_EXPORT
#define MITKCLCORE_EXPORT
#define MITKCLUTILITIES_EXPORT
#define ITKCommon_EXPORT
#define MITKMATCHPOINTREGISTRATION_EXPORT
#define MAPDeployment_EXPORT
#define MAPAlgorithms_EXPORT
#define MITKSEGMENTATION_EXPORT
#define MITKMULTILABEL_EXPORT

#define ITKCommon_EXPORT
#define ITK_FORWARD_EXPORT
#define ITK_OVERRIDE
#define ITK_NOEXCEPT


%include <itkMacro.h>
%include <mitkCommon.h>
%include <itkSmartPointer.h>
%include <mitkPixelType.h>
%include <mitkChannelDescriptor.h>
%include <mitkIOUtil.h>


#define DEPRECATED(func) func
#undef ITK_DISALLOW_COPY_AND_ASSIGN
#define ITK_DISALLOW_COPY_AND_ASSIGN(TypeName)

%pythoncode %{
 convertion_list = {}
%}

SWIG_ADD_NONOBJECT_CLASS(Indent, itkIndent.h, itk)
SWIG_ADD_MITK_CLASS(LightObject, itkLightObject.h, itk)
SWIG_ADD_MITK_CLASS(Object, itkObject.h, itk)
SWIG_ADD_MITK_CLASS(DataObject, itkDataObject.h, itk)

SWIG_ADD_MITK_CLASS(TimeGeometry, mitkTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(ArbitraryTimeGeometry, mitkArbitraryTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(ProportionalTimeGeometry, mitkProportionalTimeGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(BaseGeometry, mitkBaseGeometry.h, mitk)
SWIG_ADD_MITK_CLASS(Geometry3D, mitkGeometry3D.h, mitk)
SWIG_ADD_MITK_CLASS(SlicedGeometry3D, mitkSlicedGeometry3D.h, mitk)
SWIG_ADD_MITK_CLASS(PlaneGeometry , mitkPlaneGeometry.h, mitk)

SWIG_ADD_NONOBJECT_NOVECTOR_CLASS(BoundingBox, mitkBaseGeometry.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(TimeBounds, mitkBaseGeometry.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(FixedArrayType, mitkBaseGeometry.h, mitk)

SWIG_ADD_NONOBJECT_CLASS(Point2D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point3D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point4D, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point2I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point3I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Point4I, mitkPoint.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(VnlVector, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector2D, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector3D, mitkVector.h, mitk)
SWIG_ADD_NONOBJECT_CLASS(Vector4D, mitkVector.h, mitk)

SWIG_ADD_MITK_CLASS(BaseData, mitkBaseData.h, mitk)
SWIG_ADD_MITK_CLASS(SlicedData, mitkSlicedData.h, mitk)
SWIG_ADD_MITK_CLASS(Image, mitkImage.h, mitk)
SWIG_ADD_MITK_CLASS(LabelSetImage, mitkLabelSetImage.h, mitk)
SWIG_ADD_MITK_CLASS(PointSet, mitkPointSet.h, mitk)
%{
using mitk::Message;
%}

//
// Phenotyping Related Classes
//
SWIG_ADD_MITK_CLASS(AbstractGlobalImageFeature, mitkAbstractGlobalImageFeature.h, mitk)
SWIG_ADD_MITK_CLASS(GIFImageDescriptionFeatures, mitkGIFImageDescriptionFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFFirstOrderStatistics, mitkGIFFirstOrderStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFFirstOrderHistogramStatistics, mitkGIFFirstOrderHistogramStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFVolumetricStatistics, mitkGIFVolumetricStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFVolumetricDensityStatistics, mitkGIFVolumetricDensityStatistics.h, mitk)
SWIG_ADD_MITK_CLASS(GIFCooccurenceMatrix2, mitkGIFCooccurenceMatrix2.h, mitk)
SWIG_ADD_MITK_CLASS(GIFNeighbouringGreyLevelDependenceFeature, mitkGIFNeighbouringGreyLevelDependenceFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelRunLength, mitkGIFGreyLevelRunLength.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelSizeZone, mitkGIFGreyLevelSizeZone.h, mitk)
SWIG_ADD_MITK_CLASS(GIFGreyLevelDistanceZone, mitkGIFGreyLevelDistanceZone.h, mitk)
SWIG_ADD_MITK_CLASS(GIFLocalIntensity, mitkGIFLocalIntensity.h, mitk)
SWIG_ADD_MITK_CLASS(GIFIntensityVolumeHistogramFeatures, mitkGIFIntensityVolumeHistogramFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFNeighbourhoodGreyToneDifferenceFeatures, mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h, mitk)
SWIG_ADD_MITK_CLASS(GIFCurvatureStatistic, mitkGIFCurvatureStatistic.h, mitk)

//
// Conversion and Segmentation based Classes
//
SWIG_ADD_MITK_CLASS(ContourModelSetToImageFilter, mitkContourModelSetToImageFilter.h, mitk)
SWIG_ADD_NONOBJECT_NOVECTOR_CLASS(BooleanOperation, mitkBooleanOperation.h, mitk)
SWIG_ADD_NONOBJECT_NOVECTOR_CLASS(MorphologicalOperations, mitkMorphologicalOperations.h, mitk)
%{
  #include <itkProcessObject.h>
  typedef itk::DataObject::DataObjectIdentifierType DataObjectIdentifierType;
  typedef itk::ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;
%}

//
// MatchPoint Related Classes
//
%ignore DLLHandle::LibraryHandleType;

%{
#include <mapDeploymentDLLHandle.h>
namespace core
{
  typedef std::string String;
}
typedef map::deployment::DLLHandle::LibraryHandleType LibraryHandleType;
typedef map::deployment::DLLHandle DLLHandle;
%}
namespace core
{
  typedef std::string String;
}

%nodefaultctor LibraryHandleType;
struct LibraryHandleType {};

%include <mapUID.h>
SWIG_ADD_MITK_CLASS(UID,mapUID.h, map::algorithm)
SWIG_ADD_MITK_CLASS(DLLInfo, mapDeploymentDLLInfo.h, map::deployment)
//SWIG_ADD_MITK_CLASS_VECTORFREE(DLLHandle, mapDeploymentDLLHandle.h, map::deployment)

SWIG_ADD_MITK_CLASS_VECTORFREE(DLLDirectoryBrowser, mapDeploymentDLLDirectoryBrowser.h, ::map::deployment)
MITKSWIG_ADD_HEADERFILE(mapDeploymentDLLAccess.h)
%{
 DLLHandle const *  ConvertDLLHandleSmartPointerToPointer(itk::SmartPointer<map::deployment::DLLHandle> p)
 {
   return p.GetPointer();
 }
%}
DLLHandle const * ConvertDLLHandleSmartPointerToPointer(DLLHandle::Pointer p);

MITKSWIG_ADD_CLASS(MITKAlgorithmHelper, mitkAlgorithmHelper.h, mitk)
MITKSWIG_ADD_CLASS(RegistrationType, mitkImageMappingHelper.h, mitk::ImageMappingHelper)
MITKSWIG_ADD_CLASS(MITKRegistrationType, mitkImageMappingHelper.h, mitk::ImageMappingHelper)

// SWIG_ADD_MITK_CLASS(FastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm, mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(LevelSetMotionMultiResDefaultRegistrationAlgorithm, mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(MultiModalAffineDefaultRegistrationAlgorithm, mitkMultiModalAffineDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(MultiModalRigidDefaultRegistrationAlgorithm, mitkMultiModalRigidDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(MultiModalTransDefaultRegistrationAlgorithm, mitkMultiModalTransDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(RigidClosedFormPointsDefaultRegistrationAlgorithm, mitkRigidClosedFormPointsDefaultRegistrationAlgorithm.h, mitk)
// SWIG_ADD_MITK_CLASS(RigidICPDefaultRegistrationAlgorithm, mitkRigidICPDefaultRegistrationAlgorithm.h, mitk)