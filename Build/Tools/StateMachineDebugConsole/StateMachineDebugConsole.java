/*
 * Created on 19.12.2005
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */

/**
 * @author mvetter
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
import java.io.*;
import java.net.*;


public class StateMachineDebugConsole {
	
	static int PORT = 34768;
	
	ServerSocket m_ServerSocket = null;
	Socket m_SlientSocket = null;
	BufferedReader m_In = null;
	
	/**
	 * 
	 * @param args
	 */
	public static void main(String[] args) {
		
		System.out.println("start debugger");
		StateMachineDebugConsole instance = new StateMachineDebugConsole();
		instance.initServer();
		instance.startDebugger();		
		System.out.println("stop debugger");
	}
	
	/**
	 * 
	 *
	 */
	void initServer() {
	
		try {
			m_ServerSocket = new ServerSocket(PORT);
			m_SlientSocket = m_ServerSocket.accept();			
			m_In = new BufferedReader( new InputStreamReader( m_SlientSocket.getInputStream() ) );			
			
		} catch (IOException e) {
		    System.out.println( "Could not listen on port: " + PORT );
		    System.exit(-1);
		}	
	}
	
	/**
	 * @param in
	 * @return
	 * @throws IOException
	 */
	int readInteger( BufferedReader in ) throws IOException {
	
		int value = 0;
		char buffer[] = new char[4];
		in.read( buffer, 0, 4 );
		
		value = (int) buffer[0];
		value |= ((int)buffer[1])<<8;
		value |= ((int)buffer[2])<<16;
		value |= ((int)buffer[3])<<24;		
	
		return value;
	}

	/**
	 * 
	 * @param in
	 * @param size length of the string
	 * @return string
	 * @throws IOException
	 */
	String readString( BufferedReader in, int length ) throws IOException {
	
		char buffer[] = new char[255];
		m_In.read( buffer, 0, length );
		String string = new String( buffer, 0, length );
		return string;
	}
	
	/**
	 *  
	 *
	 */
	void startDebugger(){
		
		char buffer[] = new char[255];
		String data;
		
		try {
			
			while ( true ) {
				
				int messageType = readInteger( m_In ); 
				int bodysize = readInteger( m_In );
				
				System.out.print("\n\nMITK Interaction Debugger: \n\tMessageType: ");
				System.out.print( messageType );
				System.out.print("\n\t BodySize: ");
				System.out.print( bodysize );				
				
				switch ( messageType){
				
					case 1: { // OPEN_CONNECTION
						int hashValue = readInteger( m_In );
						int stringlength = readInteger( m_In );
						String name = readString( m_In, stringlength );
						
						System.out.print("\n\tOPEN_CONNECTION: ");
						System.out.print("\n\tHashValue: ");
						System.out.print( hashValue );
						System.out.print("\n\tFilename: " + name );						
						break;
						}
					case 2: { // NEW_STATE_MACHINE
						int instanceAddress = readInteger( m_In );
						int stringlength = readInteger( m_In );
						String name = readString( m_In, stringlength );						

						System.out.print("\n\tNEW_STATE_MACHINE: ");
						System.out.print("\n\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\n\tType Name: " + name );
						break;
						}
					case 3: { // EVENT
						int instanceAddress = readInteger( m_In );
						int eventId = readInteger( m_In );						
						
						System.out.print("\n\tEVENT: ");
						System.out.print("\n\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\n\tEventId: " );
						System.out.print( eventId );						
						break;
						}						
					case 4: { // TRANSITION
						int instanceAddress = readInteger( m_In );
						int stringlength = readInteger( m_In );
						String name = readString( m_In, stringlength );
						
						System.out.print("\n\tTRANSITION: ");
						System.out.print("\n\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\n\tTransition Name: " + name );						
						break;
					}
					case 5: { // ACTION
						int instanceAddress = readInteger( m_In );
						int stringlength = readInteger( m_In );
						String name = readString( m_In, stringlength );						
						
						System.out.print("\n\tACTION: ");
						System.out.print("\n\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\n\tAction Name: " + name );					
						
						break;
					}
						case 6: { // DELETE_STATE_MACHINE
							int instanceAddress = readInteger( m_In );
							
							System.out.print("\n\tDELETE_STATE_MACHINE: ");
							System.out.print("\n\tInstanceAddress: ");
							System.out.print( instanceAddress );							
						break;
						}
					default:
						System.out.println("unbekannter Nachrichtentype!!");
				}				
			}
			
		}catch ( IOException e ){
			System.err.println("Fehler" + e);
		}			
	}
}
