import mitk
import itk

def base2mitk(baseData):
  mitkImage = mitk.Image_New()
  mitk.ImageCaster.CastBaseData(baseData, mitkImage)
  return mitkImage

def mitk2itk(mitkImage,itkType):
  itkImage = itkType.New()
  mitk.ImageCaster.CastToItkImage(mitkImage.GetPointer(),itkImage)
  return itkImage

def base2itk(baseData,itkType):
  mitkImage = base2mitk(baseData)
  itkImage = mitk2itk(mitkImage,itkType)
  return itkImage

def node2itk(node,itkType):
  itkImage = base2itk(node.GetData(),itkType)
  return itkImage

def itk2mitk(itkImage):
  mitkImage = mitk.Image_New()
  mitk.ImageCaster.CastToMitkImage( itkImage.GetPointer(), mitkImage )
  return mitkImage

def mitk2node(mitkImage, name):
  newNode = mitk.DataNode_New()
  newNode.SetData( mitkImage.GetPointer() )
  newNode.SetName(name)
  return newNode

def itk2node(itkImage,name):
  mitkImage = itk2mitk(itkImage)
  newNode = mitk2node(mitkImage, name)
  return newNode

def threshold(node,min,max):
  itkImage = node2itk(node,itk.Image.F3)
  filter = itk.BinaryThresholdImageFilter[itkImage,itkImage].New(itkImage,LowerThreshold=min,UpperThreshold=max)
  filter.Update()
  itkImage = filter.GetOutput()
  node = itk2node(itkImage)
  return node
