/*
 * Created on 05.07.2005
 */
package commands;

//import java.util.Iterator;

import org.eclipse.gef.commands.Command;

import model.Connection;
import model.States;

/**
 * @author Daniel
 */
public class ConnectionReconnectCommand extends Command {

	/** The connection instance to reconnect. */
	private Connection connection;

	/** The new source endpoint. */
	private States newSource;

	/** The new target endpoint. */
	private States newTarget;

	/** The original source endpoint. */
	private final States oldSource;

	/** The original target endpoint. */
	private final States oldTarget;

	/**
	 * Instantiate a command that can reconnect a Connection instance to a
	 * different source or target endpoint.
	 * 
	 * @param conn
	 *            the connection instance to reconnect (non-null)
	 * @throws IllegalArgumentException
	 *             if conn is null
	 */
	public ConnectionReconnectCommand(Connection conn) {
		if (conn == null) {
			throw new IllegalArgumentException();
		}
		this.connection = conn;
		this.oldSource = conn.getSource();
		this.oldTarget = conn.getTarget();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#canExecute()
	 */
	public boolean canExecute() {
		if (newSource != null) {
			return true;
		} else if (newTarget != null) {
			return true;
		}
		return false;
	}

	/**
	 * Reconnect the connection to newSource (if setNewSource(...) was invoked
	 * before) or newTarget (if setNewTarget(...) was invoked before).
	 */
	public void execute() {
		if (newSource != null) {
			connection.reconnect(newSource, oldTarget);
			
		} else if (newTarget != null) {
			connection.reconnect(oldSource, newTarget);
		} else {
			throw new IllegalStateException("Should not happen");
		}
	}

	/**
	 * Set a new source endpoint for this connection. When execute() is invoked,
	 * the source endpoint of the connection will be attached to the supplied
	 * State instance.
	 * <p>
	 * Note: Calling this method, deactivates reconnection of the <i>target</i>
	 * endpoint. A single instance of this command can only reconnect either the
	 * source or the target endpoint.
	 * </p>
	 * 
	 * @param connectionSource
	 *            a non-null State instance, to be used as a new source endpoint
	 * @throws IllegalArgumentException
	 *             if connectionSource is null
	 */
	public void setNewSource(States connectionSource) {
		if (connectionSource == null) {
			throw new IllegalArgumentException();
		}
		setLabel("move connection startpoint");
		newSource = connectionSource;
		newTarget = null;
	}

	/**
	 * Set a new target endpoint for this connection When execute() is invoked,
	 * the target endpoint of the connection will be attached to the supplied
	 * State instance.
	 * <p>
	 * Note: Calling this method, deactivates reconnection of the <i>source</i>
	 * endpoint. A single instance of this command can only reconnect either the
	 * source or the target endpoint.
	 * </p>
	 * 
	 * @param connectionTarget
	 *            a non-null State instance, to be used as a new target endpoint
	 * @throws IllegalArgumentException
	 *             if connectionTarget is null
	 */
	public void setNewTarget(States connectionTarget) {
		if (connectionTarget == null) {
			throw new IllegalArgumentException();
		}
		setLabel("move connection endpoint");
		newSource = null;
		newTarget = connectionTarget;
	}

	/**
	 * Reconnect the connection to its original source and target endpoints.
	 */
	public void undo() {
		connection.reconnect(oldSource, oldTarget);
	}

}
