#ifndef _DATA_TREE_NODE_FACTORY_H_
#define _DATA_TREE_NODE_FACTORY_H_

#include "mitkDataTreeNodeSource.h"
#include "mitkFileReader.h"

#include <string>

namespace mitk
{
/**
 * @brief Factory, which creates instances of mitk::DataTreeNodes filled with
 *        data read from a given file 
 * @ingroup Process
 * This class reads files, creates an appropriate mitk::BaseData and adds the
 * BaseData to a mitk::DataTreeNode. This filter may produce one or more outputs
 * (i.e. mitk::DataTreeNodes). The number of generated nodes can be retrieved by a 
 * call of GetNumberOfOutputs().
 *
 * If you want to add a new file type, you have to create a protected 
 * ReadFileTypeXYZ() function which implements the process of reading
 * the base data, adding it to a node and filling the node with properties
 * as appropriate. Second, you have to add an additional condition to
 * GenerateData which identifies your file type.
 */
class DataTreeNodeFactory : public DataTreeNodeSource, public FileReader
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
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePrefix );

    /**
     * @warning multiple load not (yet) supported
     */
    itkSetStringMacro( FilePattern );

    /**
     * @warning multiple load not (yet) supported
     */
    itkGetStringMacro( FilePattern );
    
protected:

    /**
     * Constructor.
     */
    DataTreeNodeFactory();
    
    /**
     * Virtual destructor.
     */
    virtual ~DataTreeNodeFactory();
    
    /**
     * Determines of which file type a given file is and calls the 
     * appropriate reader function.
     */
    virtual void GenerateData();
  
    /**
     * Checks if the file name m_FileName ends with the given name.
     * Currently, this check is done by a dumb search for name in 
     * the filename.
     * @param name the extension of the file
     * @returns true, if the filename contains name.
     */
    virtual bool FileNameEndsWith(const std::string& name);
    
    /**
     * @returns the plain filename, that is, without any directory.
     */
    virtual std::string GetBaseFileName();
    
    /**
     * @returns the directory of the file name m_FileName. 
     */
    virtual std::string GetDirectory();

    virtual void ReadFileTypeSTL();
    
    virtual void ReadFileTypeVTK();
    
    virtual void ReadFileTypePIC();
    
    virtual void ReadFileTypePAR();
    
    virtual void ReadFileTypePVTK();
    
    virtual void ReadFileTypeDCM();
    
#ifdef MBI_INTERNAL
    
    virtual void ReadFileTypeVES();
    
    virtual void ReadFileTypeUVG();
    
    virtual void ReadFileTypeHPSONOS();
    
#endif
    
    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;
    
};
   
}


#endif //#ifndef __DATA_TREE_NODE_FACTORY_H
