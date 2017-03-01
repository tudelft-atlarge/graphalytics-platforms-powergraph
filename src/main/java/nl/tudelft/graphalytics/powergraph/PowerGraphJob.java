package nl.tudelft.graphalytics.powergraph;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.ProcessBuilder.Redirect;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.configuration.Configuration;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

abstract public class PowerGraphJob {
	private static final Logger LOG = LogManager.getLogger(PowerGraphJob.class);

	private String jobId;
	private String verticesPath;
	private String edgesPath;
	private boolean graphDirected;
	private File outputFile;
	private Configuration config;

	public PowerGraphJob(Configuration config, String verticesPath, String edgesPath, boolean graphDirected, String jobId) {
		this.config = config;
		this.verticesPath = verticesPath;
		this.edgesPath = edgesPath;
		this.graphDirected = graphDirected;
		this.jobId = jobId;
	}

	abstract protected void addJobArguments(List<String> args);

	public void setOutputFile(File file) {
		outputFile = file;
	}

	public void run() throws IOException, InterruptedException {
		List<String> args = new ArrayList<>();
		args.add(verticesPath);
		args.add(edgesPath);
		args.add(graphDirected ? "1" : "0");
		addJobArguments(args);

		if (outputFile != null) {
			args.add("--output-file");
			args.add(outputFile.getAbsolutePath());
		}


		args.add("--job-id");
		args.add(jobId);

		String argsString = "";

		for (String arg: args) {
			argsString += arg += " ";
		}

		String cmdFormat = config.getString("powergraph.command", "%s %s");
		String cmd = String.format(cmdFormat,"./" + PowerGraphPlatform.POWERGRAPH_BINARY_NAME, argsString);

		LOG.info("executing command: " + cmd);

		ProcessBuilder pb = new ProcessBuilder(cmd.split(" "));
		pb.redirectErrorStream(true);


		Process process = pb.start();
		InputStreamReader isr = new InputStreamReader(process.getInputStream());
		BufferedReader br = new BufferedReader(isr);
		String line;
		while ((line = br.readLine()) != null) {
			System.out.println(line);
		}

		int exit = process.waitFor();

		if (exit != 0) {
			throw new IOException("unexpected error code");
		}
	}
}