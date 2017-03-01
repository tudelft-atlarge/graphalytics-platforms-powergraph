package nl.tudelft.graphalytics.powergraph.algorithms.conn;

import java.util.List;

import org.apache.commons.configuration.Configuration;

import nl.tudelft.graphalytics.powergraph.PowerGraphJob;

public class ConnectedComponentsJob extends PowerGraphJob {

	public ConnectedComponentsJob(Configuration config, String verticesPath, String edgesPath, boolean graphDirected, String jobId) {
		super(config, verticesPath, edgesPath, graphDirected, jobId);
	}

	@Override
	protected void addJobArguments(List<String> args) {
		args.add("conn");
	}
}