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
	long readUnsignedInteger( BufferedReader in ) throws IOException {
	
		long value = 0;
		char buffer[] = new char[4];	
		in.read( buffer, 0, 4 );	
		
		value = (long)(buffer[0] & 0xff);
		value |= ((long)(buffer[1] & 0xff))<<8;
		value |= ((long)(buffer[2] & 0xff))<<16;
		value |= ((long)(buffer[3] & 0xff))<<24;		
		
		return value;
	}

	/**
	 * 
	 * @param in
	 * @param size length of the string
	 * @return string
	 * @throws IOException
	 */
	String readString( BufferedReader in, long length ) throws IOException {
	
		char buffer[] = new char[255];
		in.read( buffer, 0, (int)length );
		String string = new String( buffer, 0, (int)length );
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
				
				int messageType = (int) readUnsignedInteger( m_In ); 
				long bodysize = readUnsignedInteger( m_In );
				
				System.out.print("\nMITK Interaction Debugger: MessageType: ");
				System.out.print( messageType );
				System.out.print(" BodySize: ");
				System.out.print( bodysize );				
				
				switch ( messageType){
				
					case 1: { // OPEN_CONNECTION
						long hashValue = readUnsignedInteger( m_In );
						long stringlength = readUnsignedInteger( m_In );
						String name = readString( m_In, stringlength );
						
						System.out.print("\tOPEN_CONNECTION: ");
						System.out.print("\tHashValue: ");
						System.out.print( hashValue );
						System.out.print("\tFilename: " + name );						
						break;
						}
					case 2: { // NEW_STATE_MACHINE
						long instanceAddress = readUnsignedInteger( m_In );
						long stringlength = readUnsignedInteger( m_In );
						String name = readString( m_In, stringlength );						

						System.out.print("\tNEW_STATE_MACHINE: ");
						System.out.print("\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\tType Name: " + name );
						break;
						}
					case 3: { // EVENT
						long instanceAddress = readUnsignedInteger( m_In );
						long eventId = readUnsignedInteger( m_In );						
						
						System.out.print("\tEVENT: ");
						System.out.print("\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\tEventId: " );
						System.out.print( eventId );						
						break;
						}						
					case 4: { // TRANSITION
						long instanceAddress = readUnsignedInteger( m_In );
						long stringlength = readUnsignedInteger( m_In );
						String name = readString( m_In, stringlength );
						
						System.out.print("\tTRANSITION: ");
						System.out.print("\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\tTransition Name: " + name );						
						break;
					}
					case 5: { // ACTION
						long instanceAddress = readUnsignedInteger( m_In );
						long stringlength = readUnsignedInteger( m_In );
						String name = readString( m_In, stringlength );						
						
						System.out.print("\tACTION: ");
						System.out.print("\tInstanceAddress: ");
						System.out.print( instanceAddress );
						System.out.print("\tAction Name: " + name );					
						
						break;
					}
						case 6: { // DELETE_STATE_MACHINE
							long instanceAddress = readUnsignedInteger( m_In );
							
							System.out.print("\tDELETE_STATE_MACHINE: ");
							System.out.print("\tInstanceAddress: ");
							System.out.print( instanceAddress );							
						break;
						}
					case 7: {
						long counter = readUnsignedInteger( m_In );
						System.out.print(" -->>Counter: ");
						System.out.print( counter );
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
