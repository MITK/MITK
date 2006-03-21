/*
 * Created on 05.07.2005
 */
package commands;

import org.eclipse.draw2d.geometry.Rectangle;

import org.eclipse.gef.RequestConstants;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.ChangeBoundsRequest;

import model.States;

/**
 * @author Daniel
 */
public class StatesSetConstraintCommand extends Command {
	/** Stores the new size and location. */
	private final Rectangle newBounds;

	/** Stores the old size and location. */
	private Rectangle oldBounds;

	/** A request to move/resize an edit part. */
	private final ChangeBoundsRequest request;

	/** State to manipulate. */
	private final States state;

	/**
	 * Create a command that can resize and/or move a state.
	 * 
	 * @param state
	 *            the state to manipulate
	 * @param req
	 *            the move and resize request
	 * @param newBounds
	 *            the new size and location
	 * @throws IllegalArgumentException
	 *             if any of the parameters is null
	 */
	public StatesSetConstraintCommand(States state, ChangeBoundsRequest req,
			Rectangle newBounds) {
		if (state == null || req == null || newBounds == null) {
			throw new IllegalArgumentException();
		}
		this.state = state;
		this.request = req;
		this.newBounds = newBounds.getCopy();
		setLabel("Move / Resize");
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#canExecute()
	 */
	public boolean canExecute() {
		Object type = request.getType();
		// make sure the Request is of a type we support:
		return (RequestConstants.REQ_MOVE.equals(type)
				|| RequestConstants.REQ_MOVE_CHILDREN.equals(type)
				|| RequestConstants.REQ_RESIZE.equals(type) || RequestConstants.REQ_RESIZE_CHILDREN
				.equals(type));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute() {
		oldBounds = new Rectangle(state.getLocation(), state.getSize());
		redo();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo() {
		state.setSize(newBounds.getSize());
		state.setLocation(newBounds.getLocation());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo() {
		state.setSize(oldBounds.getSize());
		state.setLocation(oldBounds.getLocation());
	}
}
