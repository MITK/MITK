// std
#include <string>

// itk includes
#include <itkMinimumMaximumImageCalculator.h>

// mitk includes
#include <mitkBoolProperty.h>

class CommonFunctionality
{

public:
  
  /** \brief compute min and max 
  */
  template < typename TImageType >
  static void MinMax(typename TImageType::Pointer image, float &min, float &max)
  {
    typedef itk::MinimumMaximumImageCalculator<TImageType> MinMaxCalcType;
    MinMaxCalcType::Pointer minmax = MinMaxCalcType::New();
    minmax->SetImage( image );
    minmax->ComputeMinimum();
    minmax->ComputeMaximum();

    min = (float) minmax->GetMinimum();
    max = (float) minmax->GetMaximum();
  }


  /** 
   * \brief converts the itk image to mitk image, creates a datatreenode and adds the node to 
   * the referenced datatree
   */
  template < typename TImageType >
  static void AddItkImageToDataTree(typename TImageType::Pointer itkImage, mitk::DataTreeIterator* iterator, std::string str)
  {
  mitk::DataTreeIterator* it=iterator->clone();

  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk(itkImage.GetPointer());
  image->SetVolume(itkImage->GetBufferPointer());

  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetData(image);
  it->add(node);

  mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(str.c_str());
  node->SetProperty("fileName",nameProp);

	mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
  mitk::LevelWindow levelWindow;
  levelWindow.SetAuto( image->GetPic() );
  levWinProp->SetLevelWindow(levelWindow);
  node->GetPropertyList()->SetProperty("levelwindow",levWinProp);

  float extrema[2];
  extrema[0] = 0;
  extrema[1] = 4096;

  CommonFunctionality::MinMax<TImageType>(itkImage,extrema[0],extrema[1]);
  if (extrema[0] == 0 && extrema[1] ==1)
  {
    mitk::BoolProperty::Pointer binProp = new mitk::BoolProperty(true);
    node->GetPropertyList()->SetProperty("binary",binProp);
  }
  delete it;
}

};
