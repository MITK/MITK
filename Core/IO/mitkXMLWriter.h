#ifndef MITK_XML_WRITER
#define MITK_XML_WRITER

#include <fstream>
#include <string>
#include <stack>

namespace mitk{

	class XMLReader{};

	class XMLWriter {

		std::stack<const char*> m_Stack;
		std::ostream* m_Out;
		int m_Increase;
		int m_Space;
		int m_NodeCount;
		bool m_File;
		bool m_FirstNode;
		bool m_NodeIsClosed;
		bool m_NewNode;

    std::string m_Filename;
    std::string m_SubFolder;
    int m_FileCounter;

	public:

		/**
		 * Construktor
		 */
		XMLWriter( const char* filename, int space = 3);

		/**
		 * Construktor
		 */
		XMLWriter( std::ostream& out, int space = 3 );

		/**
		 * Destruktor
		 */
		~XMLWriter();

		/*
		 * begin an new node
		 */
		void BeginNode( const char* name );

		/**
		 * close an open node
		 */
		void EndNode( );

		/**
		 * Write string Property
		 */
		void WriteProperty( const char* key, const char* value ) const;

		/**
		 * Write string Property
		 */
		void WriteProperty( const char* key, const std::string& value ) const;

		/**
		 * Write int Property
		 */
		void WriteProperty( const char* key, int value ) const;

		/**
		 * Write float Property
		 */
		void WriteProperty( const char* key, float value ) const;

		/**
		 * Write double Property
		 */
		void WriteProperty( const char* key, double value ) const;

		/**
		 * Write bool Property
		 */
		void WriteProperty( const char* key, bool value ) const;

		/**
		 * Write comment
		 */
		void WriteComment( const char* text );
	
		/**
		 * retun the current deph
		 */
		int GetCurrentDeph() const;

		/**
		 * Get the current count of nodes
		 */
		int GetNodeCount() const;

		/**
		 * Get the space
		 */
		int GetSpace() const;

		/**
		 *
		 */
		void SetSpace( int space );

    /**
     * get the subfolder of the xml-File. 
     */
    void setSubFolder( const char* subFolder );

    /**
     * get the subfolder of the xml-File. 
     */
    const char* getSubFolder();

    /*
     * get an new unique FileName in the subdirectory of the xml-File
     */
    const char* getNewFileName();

    /*
     * get an new unique FileName in the subdirectory of the xml-File
     */
    const char* getNewFilenameAndSubFolder();

		private:

		/**
		 * replace char < and > through { and }
		 */
		const char* ConvertString( const char* string ) const;
	};
}
#endif
