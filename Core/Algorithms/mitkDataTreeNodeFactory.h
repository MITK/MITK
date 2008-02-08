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


#ifndef _DATA_TREE_NODE_FACTORY_H_
#define _DATA_TREE_NODE_FACTORY_H_

#include "mitkDataTreeNodeSource.h"
#include "mitkFileSeriesReader.h"
#include "mitkColorProperty.h"

#include <vtkConfigure.h>
#include <string>

namespace mitk
{
/**
* @brief Factory, which creates instances of mitk::DataTreeNodes filled with
*        data read from a given file 
*
* This class reads files, creates an appropriate mitk::BaseData and adds the
* BaseData to a mitk::DataTreeNode. This filter may produce one or more outputs
* (i.e. mitk::DataTreeNodes). The number of generated nodes can be retrieved by a 
* call of GetNumberOfOutputs().
*
* If you want to add a new file type, you have to register the factory 
* of the file reader in the class mitk::BaseDataIOFactory.
* @ingroup IO
*/
class MITK_CORE_EXPORT DataTreeNodeFactory : public DataTreeNodeSource, public FileSeriesReader
{
public:

  mitkClassMacro( DataTreeNodeFactory, DataTreeNodeSource );

  itkNewMacro( Self );

  /**
  * Sets the filename of the file to read.
  * @param FileName the name of the file to read.
  */
  itkSetStringMacro( FileName );

  /**
  * @returns the name of the file to be read from disk.
  */
  itkGetStringMacro( FileName );

  /**
  * \brief Set prefix for multiple load
  */
  itkSetStringMacro( FilePrefix );

  /**
  * \brief Get prefix for multiple load
  */
  itkGetStringMacro( FilePrefix );

  /**
  * \brief Set pattern for multiple load
  */
  itkSetStringMacro( FilePattern );

  /**
  * \brief Get pattern for multiple load
  */
  itkGetStringMacro( FilePattern );

  /**
   * Sets default properties common to all image types
   */
  static void SetDefaultImageProperties(DataTreeNode::Pointer &node);
  
  /**
   * Sets default properties common to all data types representing a surface
   */
  static void SetDefaultSurfaceProperties(DataTreeNode::Pointer &node);

  /**
   * Sets default properties common to all data types representing a binary segmentation
   */
  static void SetDefaultSegmentationProperties(DataTreeNode::Pointer &node);
  
  /**
   * Sets default properties common to all data types representing a PointSet
   */
  static void SetDefaultPointSetProperties(DataTreeNode::Pointer &node);
  
  /**
    * Sets default properties common to all data types representing an unstructured grid
    */
  static void SetDefaultUnstructuredGridProperties(DataTreeNode::Pointer &node);

  /**
    * Nice default colors for segmentations of some "normal" organs.
    */
  static ColorProperty::Pointer DefaultColorForOrgan( const std::string& );

  /**
   * Sets default common properties
   */
  void SetDefaultCommonProperties(DataTreeNode::Pointer &node);

  /**
   * Sets default properties common to ultrasound content
   */
  void SetDefaultUltraSoundProperties(DataTreeNode::Pointer &node);

  /**
   * Sets default properties common to tus files
   */
  void SetDefaultTusProperties(int count, DataTreeNode::Pointer &node);

  /**
   * if true -> loaded image is part of a serie
   */
  void SetImageSerie(bool serie);

  static bool m_TextureInterpolationActive;
  
protected:

  /**
  * Constructor.
  */
  DataTreeNodeFactory();

  /**
  * Virtual destructor.
  */
  virtual ~DataTreeNodeFactory();

  bool m_Serie;

  /**
  * Determines of which file type a given file is and calls the 
  * appropriate reader function.
  */
  virtual void GenerateData();


  /**
  * Resizes the number of outputs of the factory.
  * The outputs are initialized by empty DataTreeNodes
  * @param num the new number of outputs
  */
  virtual void ResizeOutputs( const unsigned int& num );

  /**
  * Checks if the file name m_FileName ends with the given name.
  * Currently, this check is done by a dumb search for name in 
  * the filename.
  * @param name the extension of the file
  * @returns true, if the filename contains name.
  */
  virtual bool FileNameEndsWith(const std::string& name);

  /**
  * Checks if the file pattern m_FilePattern ends with the given name.
  * Currently, this check is done by a dumb search for name in 
  * the filename.
  * @param name the extension of the file
  * @returns true, if the filepattern contains name.
  */
  virtual bool FilePatternEndsWith(const std::string& name);

  /**
  * @returns the plain filename, that is, without any directory.
  */
  virtual std::string GetBaseFileName();

  /**
  * @returns the plain file prefix, that is, without any directory.
  */
  virtual std::string GetBaseFilePrefix();

  /**
  * @returns the directory of the file name m_FileName. 
  */
  virtual std::string GetDirectory();

#ifdef MBI_INTERNAL
  virtual void ReadFileTypeHPSONOS();
#ifdef HAVE_IPDICOM
  virtual void ReadFileTypeIPDCM();
#endif /* HAVE_IPDICOM */
#ifdef USE_TUS_READER
  virtual void ReadFileTypeTUS();
#endif
#endif /* MBI_INTERNAL */

  virtual void ReadFileSeriesTypeDCM();

  virtual void ReadFileSeriesTypeITKImageSeriesReader();
};

}


#endif //#ifndef __DATA_TREE_NODE_FACTORY_H
