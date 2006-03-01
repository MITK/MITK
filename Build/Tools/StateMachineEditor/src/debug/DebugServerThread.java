package debug;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.swing.JOptionPane;

import stateMachinesList.StateMachinesList;

import dom.DOMGetInstance;

/**
 * Because Eclipse can not do anything else while listening to sockets we have to
 * implement a thread which overtakes the job for listening.
 * 
 * @author Daniel
 */
public class DebugServerThread extends Thread {
	
	private java.util.List m_MessageList = new Vector();

	private Map eventsToMachine = new HashMap();
	private static BufferedReader in = null;
	private static Socket clientSocket = null;
	private static ServerSocket serverSocket = null;
	private static final int OPEN_CONNECTION = 1;
	private static final int NEW_STATE_MACHINE = 2;
	private static final int EVENT = 3;
	private static final int TRANSITION = 4;
	private static final int ACTION = 5;
	private static final int DELETE_STATE_MACHINE = 6;
	
	public DebugServerThread() {
		super("ServerThread");	
	}
	
	public EventsForStatemachine getEventsForStatemachine(String instanceAddress) {
		if (eventsToMachine.containsKey(instanceAddress)) {
			return (EventsForStatemachine) eventsToMachine.get(instanceAddress);
		}
		return null;
	}	

	/**
	 * 
	 * @param messages
	 * @return false: no massages, true new mesaages
	 */
	synchronized boolean getNewMessages( java.util.List messages ) {

		if ( m_MessageList.size() == 0 )
			return false;
		
		messages.clear();
		messages.addAll( m_MessageList );
		m_MessageList.clear();		
		return true;
	}
	
	synchronized void addNewMessage( Object message ) {
		m_MessageList.add( message );
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
	
	public void run() {
		DebugStateMachinesList.setServer(this);
		try {
			serverSocket = new ServerSocket(34768);
		} catch (IOException e) {
			System.err.println("Could not listen on port: 34768.");
			System.exit(1);
		}
		
		try {
			clientSocket = serverSocket.accept();
			in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
			
			boolean exitLoop = false;
			
			while (true) {
				
				int messageType = (int) readUnsignedInteger( in ); 
				long bodysize = readUnsignedInteger( in );
				
				System.out.print("\nMITK Interaction Debugger: MessageType: ");
				System.out.print( messageType );
				System.out.print(" BodySize: ");
				System.out.print( bodysize );			
				
				switch ( messageType ) {
				
				case OPEN_CONNECTION: {
					long hashValue = readUnsignedInteger( in );
					long stringlength = readUnsignedInteger( in );
					String name = readString( in, stringlength );
					if (!(name.equals(DOMGetInstance.getFilename()))) {
						JOptionPane.showMessageDialog(null,
								"Used xml file is not the same! Exit Debug-Mode.", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						exitLoop = true;
					}
					System.out.println(hashValue + name);
					break;
				}
				case NEW_STATE_MACHINE: {
					long instanceAddress = readUnsignedInteger( in );
					long stringlength = readUnsignedInteger( in );
					String name = readString( in, stringlength );
					EventsForStatemachine eventObject = new EventsForStatemachine();
					eventsToMachine.put(Integer.toString((int) instanceAddress), eventObject);
					Message message = new Message();
					message.setInstanceAddress(Integer.toString((int) instanceAddress));
					message.setName(name);
					addNewMessage(message);
					//DebugStateMachinesList.addStateMachineToList();
					
					//DebugStateMachinesList.addStateMachineToList2(name);
					System.out.println(instanceAddress + name);
					break;
				}
				case EVENT: {
					long instanceAddress = readUnsignedInteger( in );
					long eventId = readUnsignedInteger( in );	
					EventsForStatemachine eventObject = (EventsForStatemachine) eventsToMachine.get(Integer.toString((int) instanceAddress));
					eventObject.addEvent(Integer.toString((int) eventId));
					/*if (DebugEventsList.getInstanceAddress().equals(Integer.toString(instanceAddress))) {
					 DebugEventsList.addEventToList(Integer.toString(eventId));
					 }*/
					System.out.println(instanceAddress + eventId);
					break;
				}
				case TRANSITION: {
					long instanceAddress = readUnsignedInteger( in );
					long stringlength = readUnsignedInteger( in );
					String name = readString( in, stringlength );
					System.out.println(instanceAddress + name);
					break;
				}
				case ACTION: {
					long instanceAddress = readUnsignedInteger( in );
					long stringlength = readUnsignedInteger( in );
					String name = readString( in, stringlength );
					System.out.println(instanceAddress + name);
					break;
				}
				
				case DELETE_STATE_MACHINE: {
					long instanceAddress = readUnsignedInteger( in );
					//DebugStateMachinesList.removeStateMachineFromList(Integer.toString(instanceAddress));
					eventsToMachine.remove(Integer.toString((int) instanceAddress));
					System.out.println(instanceAddress);
					break;
				}
				case 7: {
					long counter = readUnsignedInteger( in );
					System.out.print(" -->>Counter: ");
					System.out.print( counter );
					break;
				}
				default:
					System.out.println(messageType);
					System.out.println("unknown messagetype!!");
				}
				if (exitLoop) {
					System.out.println("exit");
					break;
				}
			}
			//PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
			//DebugStateMachinesList.closeView();
			StateMachinesList.setDebugMode(false);
			in.close();
			clientSocket.close();
			serverSocket.close();
		}
		catch (IOException e) {
			System.err.println(e);
		}
		finally {
			//PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
			//DebugStateMachinesList.closeView();
			StateMachinesList.setDebugMode(false);
		}
	}
}
