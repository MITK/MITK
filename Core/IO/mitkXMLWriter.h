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

	public:

		/**
		 * Construktor
		 */
		XMLWriter( const char* filename, int space = 3)
			:m_Out(NULL), m_Increase(0), m_Space(space), m_NodeCount(0) , m_File(true), m_FirstNode(true), m_NodeIsClosed(true), m_NewNode(false) 
		{		
			m_Out = new std::ofstream( filename );			
		}

		/**
		 * Construktor
		 */
		XMLWriter( std::ostream& out, int space = 3 )
			:m_Out(&out), m_Increase(0), m_Space(space), m_NodeCount(0), m_File(false), m_FirstNode(true), m_NodeIsClosed(true), m_NewNode(false) 
		{}

		/**
		 * Destruktor
		 */
		~XMLWriter() 
		{
			if ( m_File ) {
				std::ofstream* file = static_cast<std::ofstream*>(m_Out);
				file->close();
			}
		}

		void BeginNode( const char* name ) 
		{		
			if ( m_FirstNode ) 
			{
				*m_Out << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
				m_FirstNode = false;
			}

			if ( !m_NodeIsClosed ) 
			{
				*m_Out << ">\n";
				m_NodeIsClosed = true;
			}

			int steps = m_Space * m_Increase;

			for(int i=0; i < steps; i++) 
				*m_Out << ' '; 

			*m_Out << '<' << name << ' ';

			m_NodeIsClosed = false;
			m_NewNode = true;
			m_Stack.push( name );
			m_Increase++;
			m_NodeCount++;
		}

		/**
		 *
		 */
		void EndNode( )
		{			
			m_Increase--;

			if ( m_NewNode )
				*m_Out << "/>\n";
			else 
			{
				int steps = m_Space * m_Increase;

				for(int i=0; i < steps; i++) 
					*m_Out << ' '; 
				*m_Out << "</" << m_Stack.top() << ">\n";				
			}

			m_Stack.pop();
			m_NodeIsClosed = true;
			m_NewNode = false;
		}

		/**
		 * Write Property
		 */
		void WriteProperty( const char* key, const char* value ) const
		{
			*m_Out << ConvertString( key );
			*m_Out << "=\"" << ConvertString( value ) << "\" ";
		}

		/**
		 * Write comment
		 */
		void WriteComment( const char* text ) 
		{
			if ( m_FirstNode ) 
			{
				*m_Out <<"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
				m_FirstNode = false;
			}

			int steps = m_Space * m_Increase;

			for(int i=0; i < steps; i++) 
				*m_Out << ' '; 

			*m_Out << "<!-- " << ConvertString( text ) <<  " -->\n";
		}
	
		/**
		* retun the current deph
		*/
		int GetCurrentDeph() const
		{
			return m_Stack.size();
		}

		/**
		*
		*/
		int GetNodeCount() const
		{
			return m_NodeCount;
		}

		/**
		* Get the space
		*/
		int GetSpace() const
		{
			return m_Space;
		}

		/**
		*
		*/
		void SetSpace( int space )
		{
			m_Space = space;
		}

		private:

		const char* ConvertString( const char* string ) const
		{
			static std::char_traits<char>::char_type buffer[255];
			int length = std::char_traits<char>::length( string );
			std::char_traits<char>::copy ( buffer, string, length + 1 );
			const char* pos = buffer;
			
			while ( pos != NULL ) 
			{
				pos = std::char_traits<char>::find ( buffer, length , '<');

				if( pos != NULL )
					*(const_cast<char*>(pos)) = '{';
			}

			pos = buffer;
			
			while ( pos != NULL ) 
			{
				pos = std::char_traits<char>::find ( buffer, length , '>');

				if( pos != NULL )
					*(const_cast<char*>(pos)) = '}';
			}

			return buffer;
		}
	};
}
#endif
