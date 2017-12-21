/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkDICOMFileReader_h
#define mitkDICOMFileReader_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "MitkDICOMReaderExports.h"

#include "mitkDICOMTagCache.h"

#include "mitkDICOMImageBlockDescriptor.h"

#include <unordered_map>

namespace mitk
{
// TODO Philips3D!
// TODO http://bugs.mitk.org/show_bug.cgi?id=11572 ?

/**
  \ingroup DICOMReaderModule
  \brief Interface for DICOM readers that produce mitk::Images.

  As described in \ref DICOMReaderModule, this class structures
  the reader's part in the process of analyzing a set of DICOM files
  and selecting the most appropriate reader.

  The overall loading process is as follows:
   - <b>Define input files</b>: a list of absolute filenames
   - <b>Analyze the potential output</b>: see what can be made of the input files, describe with
  DICOMImageBlockDescriptor%s
   - <b>Load pixel data</b>: an application will usually analyze files using multiple readers and only load
  with a single reader

  Sub-classes are required to implement a number of methods that
  reflect above structure. See mitk::DICOMITKSeriesGDCMReader for
  an example.

  To help applications in describing different readers to the user, each reader
  brings a number of methods that describe its configuration/specifics by
  means of a short label and a (longer) description.
*/
class MITKDICOMREADER_EXPORT DICOMFileReader : public itk::Object
{
public:
  mitkClassMacroItkParent( DICOMFileReader, itk::Object );

  /// Test whether a file is DICOM at all
  static bool IsDICOM( const std::string& filename );
  /// Indicate whether this reader can handle given file
  virtual bool CanHandleFile( const std::string& filename ) = 0;

  /// This input files
  void SetInputFiles( const StringList& filenames );
  /// This input files
  const StringList& GetInputFiles() const;

  /// Analyze input files
  virtual void AnalyzeInputFiles() = 0;

  /// Number of outputs, only meaningful after calling AnalyzeInputFiles()
  unsigned int GetNumberOfOutputs() const;
  /// Individual outputs, only meaningful after calling AnalyzeInputFiles(). \throws std::invalid_argument
  const DICOMImageBlockDescriptor& GetOutput( unsigned int index ) const;

  // void AllocateOutputImages(); TODO for later implementation of slice-by-slice loading

  /// Load the mitk::Image%s in our outputs, the DICOMImageBlockDescriptor. To be called only after
  /// AnalyzeInputFiles(). Take care of potential exceptions!
  virtual bool LoadImages() = 0;

  virtual DICOMTagPathList GetTagsOfInterest() const = 0;

  /// A way to provide external knowledge about files and tag values is appreciated.
  virtual void SetTagCache( const DICOMTagCache::Pointer& ) = 0;

  /// Short label/name to describe this reader
  void SetConfigurationLabel( const std::string& );
  /// Short label/name to describe this reader
  std::string GetConfigurationLabel() const;
  /// One-sentence description of the reader's loading "strategy"
  void SetConfigurationDescription( const std::string& );
  /// One-sentence description of the reader's loading "strategy"
  std::string GetConfigurationDescription() const;

  /// Print configuration description to given stream, for human reader
  void PrintConfiguration( std::ostream& os ) const;
  /// Print output description to given stream, for human reader
  void PrintOutputs( std::ostream& os, bool filenameDetails = false ) const;

  virtual bool operator==( const DICOMFileReader& other ) const = 0;

  /** Type specifies additional tags of interest. Key is the tag path of interest.
   * The value is an optional user defined name for the property that should be used to store the tag value(s).
   * Empty value is default and will imply to use the found DICOMTagPath as property name.*/
  typedef DICOMImageBlockDescriptor::AdditionalTagsMapType AdditionalTagsMapType;
  /**
  * \brief Set a list of DICOMTagPaths that specifiy all DICOM-Tags that will be copied into the property of the mitk::Image.
  *
  * This method can be used to specify a list of DICOM-tags that shall be available after the loading.
  * The value in the tagMap is an optional user defined name for the property key that should be used
  * when storing the property). Empty value is default and will imply to use the found DICOMTagPath
  * as property key.
  * By default the content of the DICOM tags will be stored in a StringLookupTable on the mitk::Image.
  * This behaviour can be changed by setting a different TagLookupTableToPropertyFunctor via
  * SetTagLookupTableToPropertyFunctor().
  */
  virtual void SetAdditionalTagsOfInterest(const AdditionalTagsMapType& tagList);

  /**
  * \brief Set a functor that defines how the slice-specific tag-values are stored in a Property.
  *
  * This method sets a functor that is given a StringLookupTable that contains the values of one DICOM tag
  * mapped to the slice index.
  * The functor is supposed to store these values in an mitk Property.
  *
  * By default, the StringLookupTable is stored in a StringLookupTableProperty except if all values are
  * identical. In this case, the unique value is stored only once in a StringProperty.
  */
  virtual void SetTagLookupTableToPropertyFunctor(
    mitk::DICOMImageBlockDescriptor::TagLookupTableToPropertyFunctor functor );

protected:
  DICOMFileReader();
  ~DICOMFileReader() override;

  DICOMFileReader( const DICOMFileReader& other );
  DICOMFileReader& operator=( const DICOMFileReader& other );

  void ClearOutputs();
  void SetNumberOfOutputs( unsigned int numberOfOutputs );
  void SetOutput( unsigned int index, const DICOMImageBlockDescriptor& output );

  /// non-const access to the DICOMImageBlockDescriptor
  DICOMImageBlockDescriptor& InternalGetOutput( unsigned int index );

  /// Configuration description for human reader, to be implemented by sub-classes
  virtual void InternalPrintConfiguration( std::ostream& os ) const = 0;

  virtual AdditionalTagsMapType GetAdditionalTagsOfInterest() const;

  mitk::DICOMImageBlockDescriptor::TagLookupTableToPropertyFunctor GetTagLookupTableToPropertyFunctor() const;


private:
  StringList m_InputFilenames;
  std::vector<DICOMImageBlockDescriptor> m_Outputs;

  std::string m_ConfigLabel;
  std::string m_ConfigDescription;

  AdditionalTagsMapType m_AdditionalTagsOfInterest;
  mitk::DICOMImageBlockDescriptor::TagLookupTableToPropertyFunctor m_TagLookupTableToPropertyFunctor;
};
}

#endif
