# create the mitkControllers libraries
SET(WRAP_SOURCES_TEST
	#Algorithms:
	#wrap_itkImportMitkImageContainer //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_itkLocalVariationImageFilter //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_itkMITKScalarImageToHistogramGenerator //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_itkTotalVariationDenoisingImageFilter //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_itkTotalVariationSingleIterationImageFilter //Template: can be wrapped; TODO: instantiate the needed templates
  wrap_mitkBaseDataSource
	wrap_mitkBaseProcess
	wrap_mitkCoreObjectFactory
	wrap_mitkCoreObjectFactoryBase
	wrap_mitkDataNodeFactory
	wrap_mitkDataNodeSource
	wrap_mitkGeometry2DDataToSurfaceFilter
	wrap_mitkHistogramGenerator
	#wrap_mitkImageAccessByItk //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_mitkImageCast //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_mitkImageCastPart1 //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_mitkImageCastPart2 //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_mitkImageCastPart3 //Template: can be wrapped; TODO: instantiate the needed templates
	#wrap_mitkImageCastPart4 //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkImageChannelSelector
	wrap_mitkImageSliceSelector
	wrap_mitkImageSource
	wrap_mitkImageTimeSelector
	wrap_mitkImageToImageFilter
	wrap_mitkImageToItk
	#wrap_mitkInstantiateAccessFunctions // Macros
	wrap_mitkITKImageImport
	wrap_mitkPointSetSource
	wrap_mitkRGBToRGBACastImageFilter
	wrap_mitkSubImageSelector
	wrap_mitkSurfaceSource
	wrap_mitkSurfaceToSurfaceFilter
	wrap_mitkUIDGenerator
	wrap_mitkVolumeCalculator
	
	#Controllers:
	wrap_mitkBaseController
	wrap_mitkCallbackFromGUIThread
	wrap_mitkCameraController
	wrap_mitkCameraRotationController
	wrap_mitkFocusManager
	wrap_mitkLimitedLinearUndo
	wrap_mitkOperationEvent
	wrap_mitkProgressBar
	wrap_mitkProgressBarImplementation
	wrap_mitkReferenceCountWatcher
	wrap_mitkRenderingManager
	wrap_mitkRenderingManagerFactory
	wrap_mitkSliceNavigationController
	wrap_mitkSlicesCoordinator
	wrap_mitkSlicesRotator
	wrap_mitkSlicesSwiveller
	wrap_mitkStatusBar
	wrap_mitkStatusBarImplementation
	wrap_mitkStepper
	#wrap_mitkTestingMacros //Macros
	wrap_mitkTestManager
	wrap_mitkUndoController
	wrap_mitkUndoModel
	wrap_mitkVerboseLimitedLinearUndo
	wrap_mitkVtkInteractorCameraController
	wrap_mitkVtkLayerController
	
	#DataManagement:
	#wrap_itkVtkAbstractTransform //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkAbstractTransformGeometry
	wrap_mitkAnnotationProperty
	wrap_mitkApplicationCursor
	wrap_mitkBaseData
	wrap_mitkBaseDataTestImplementation
	wrap_mitkBaseProperty
	wrap_mitkClippingProperty
	wrap_mitkColorProperty
	#wrap_mitkCommon //Macros
	wrap_mitkDataNode
	wrap_mitkDataStorage
	wrap_mitkDisplayGeometry
	wrap_mitkEnumerationProperty
	wrap_mitkGenericLookupTable
	#wrap_mitkGenericProperty //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkGeometry2D
	wrap_mitkGeometry2DData
	wrap_mitkGeometry3D
	wrap_mitkGeometryData
	wrap_mitkGroupTagProperty
	wrap_mitkImage
	wrap_mitkImageDataItem
	#wrap_mitkItkMatrixHack //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkLandmarkBasedCurvedGeometry
	wrap_mitkLandmarkProjector
	wrap_mitkLandmarkProjectorBasedCurvedGeometry
	wrap_mitkLevelWindow
	wrap_mitkLevelWindowManager
	wrap_mitkLevelWindowPreset
	wrap_mitkLevelWindowProperty
	#wrap_mitkLine //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkLookupTable
	wrap_mitkLookupTables
	wrap_mitkMaterial
	#wrap_mitkMatrixConvert //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkMemoryUtilities
	wrap_mitkModalityProperty
	wrap_mitkModeOperation
	wrap_mitkNodePredicateAND
	wrap_mitkNodePredicateBase
	wrap_mitkNodePredicateCompositeBase
	wrap_mitkNodePredicateData
	wrap_mitkNodePredicateDataType
	wrap_mitkNodePredicateDimension
	wrap_mitkNodePredicateFirstLevel
	wrap_mitkNodePredicateNOT
	wrap_mitkNodePredicateOR
	wrap_mitkNodePredicateProperty
	wrap_mitkNodePredicateSource
	wrap_mitkPlaneOrientationProperty
	wrap_mitkPlaneGeometry
	wrap_mitkPlaneOperation
	wrap_mitkPointOperation
	wrap_mitkPointSet
	wrap_mitkProperties
	wrap_mitkPropertyList
	wrap_mitkResliceMethodProperty
	wrap_mitkRotationOperation
	wrap_mitkShaderProperty
	wrap_mitkSlicedData
	wrap_mitkSlicedGeometry3D
	wrap_mitkSmartPointerProperty
	wrap_mitkStandaloneDataStorage
	wrap_mitkStateTransitionOperation
	wrap_mitkStringProperty
	wrap_mitkSurface
	wrap_mitkSurfaceOperation
	wrap_mitkThinPlateSplineCurvedGeometry
	wrap_mitkTimeSlicedGeometry
	wrap_mitkTransferFunction
	wrap_mitkTransferFunctionProperty
	#wrap_mitkVector //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkVtkInterpolationProperty
	wrap_mitkVtkRepresentationProperty
	wrap_mitkVtkResliceInterpolationProperty
	wrap_mitkVtkScalarModeProperty
	wrap_mitkVtkVolumeRenderingProperty
	#wrap_mitkWeakPointer //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkWeakPointerProperty
	
	#Interactions:
	wrap_mitkAction
	wrap_mitkAffineInteractor
	wrap_mitkCoordinateSupplier
	wrap_mitkDisplayCoordinateOperation
	wrap_mitkDisplayInteractor
	wrap_mitkDisplayPositionEvent
	wrap_mitkDisplayVectorInteractor
	wrap_mitkEvent
	wrap_mitkEventDescription
	wrap_mitkEventMapper
	wrap_mitkGlobalInteraction
	wrap_mitkInteractor
	wrap_mitkKeyEvent
	#wrap_mitkMessage //Macros;Templates: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkMouseMovePointSetInteractor
	wrap_mitkPointSetInteractor
	wrap_mitkPositionEvent
	wrap_mitkPositionTracker
	wrap_mitkState
	wrap_mitkStateEvent
	wrap_mitkStateMachine
	wrap_mitkStateMachineFactory
	wrap_mitkTransition
	wrap_mitkWheelEvent
	
	#IO:
	wrap_mitkBaseDataIOFactory
	wrap_mitkDicomSeriesReader
	wrap_mitkFileReader
	wrap_mitkFileSeriesReader
	wrap_mitkFileWriter
	wrap_mitkFileWriterWithInformation
	wrap_mitkImageWriter
	wrap_mitkImageWriterFactory
	#wrap_mitkIOAdapter //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkItkImageFileIOFactory
	wrap_mitkItkImageFileReader
	#wrap_mitkItkPictureWrite //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkLog
	wrap_mitkLookupTableProperty
	wrap_mitkOperation
	wrap_mitkOperationActor
	wrap_mitkPicFileIOFactory
	wrap_mitkPicFileReader
	wrap_mitkPicFileWriter
	wrap_mitkPicHelper
	wrap_mitkPicVolumeTimeSeriesIOFactory
	wrap_mitkPicVolumeTimeSeriesReader
	wrap_mitkPixelType
	wrap_mitkPointSetIOFactory
	wrap_mitkPointSetReader
	wrap_mitkPointSetWriter
	wrap_mitkPointSetWriterFactory
	#wrap_mitkRawImageFileReader // need to be modified; wrapper class tries to convert from 'std::string' to 'char *'
	wrap_mitkStandardFileLocations
	wrap_mitkSTLFileIOFactory
	wrap_mitkSTLFileReader
	#wrap_mitkSurfaceVtkWriter //Template: can be wrapped; TODO: instantiate the needed templates
	wrap_mitkSurfaceVtkWriterFactory
	wrap_mitkVtiFileIOFactory
	wrap_mitkVtiFileReader
	wrap_mitkVtkImageIOFactory
	wrap_mitkVtkImageReader
	wrap_mitkVtkSurfaceIOFactory
	wrap_mitkVtkSurfaceReader
	wrap_vtkPointSetXMLParser
	
	#Rendering:
	#wrap_mitkBaseRenderer //leads to a crash of cswig; after bug fixed in cswig -> circular import error
	wrap_mitkVtkMapper2D
	wrap_mitkVtkMapper3D
	wrap_mitkGeometry2DDataMapper2D
	wrap_mitkGeometry2DDataVtkMapper3D
	wrap_mitkGLMapper2D
	wrap_mitkGradientBackground
	wrap_mitkImageMapperGL2D
	wrap_mitkMapper
	wrap_mitkMapper2D
	wrap_mitkMapper3D
	wrap_mitkPointSetGLMapper2D
	wrap_mitkPointSetVtkMapper3D
	wrap_mitkPolyDataGLMapper2D
	wrap_mitkShaderRepository
	wrap_mitkSurfaceGLMapper2D
	wrap_mitkSurfaceVtkMapper3D
	wrap_mitkVolumeDataVtkMapper3D
	wrap_mitkVtkPropRenderer
	wrap_mitkVtkWidgetRendering
	wrap_vtkMitkRectangleProp
	wrap_vtkMitkRenderProp
	wrap_vtkMitkThickSlicesFilter
	wrap_mitkManufacturerLogo
	
	wrap_mitkCommonSuperclasses
	wrap_mitkImageCaster
	#wrap_qmitkFunctionality
)

