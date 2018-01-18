package edu.duke.raft;

import java.util.Timer;

public class LeaderMode extends RaftMode {

	private Timer heartbeatTimer;
	private int heartbeatTimerID = 3;
	private int[] serverLogMaxIndex = new int[mConfig.getNumServers() + 1];
	private boolean isDead = false;
	private static int LOG_FAIL = -2;

	public void go() {
		synchronized (mLock) {
			int term = mConfig.getCurrentTerm();

			System.out.println("S" + mID + "." + term + ": switched to leader mode.");

			heartbeatTimer = scheduleTimer(HEARTBEAT_INTERVAL, heartbeatTimerID);

			for (int i = 1; i <= mConfig.getNumServers(); i++) {
				serverLogMaxIndex[i] = mLog.getLastIndex() + 1;
			}

			RaftResponses.clearAppendResponses(term);

			isDead = false;
			logRepair();

		}
	}

	public void logRepair() {
		synchronized (mLock) {
			int term = mConfig.getCurrentTerm();
			int numServers = mConfig.getNumServers();
			int lastIndex = mLog.getLastIndex();
			int lastTerm = mLog.getLastTerm();

			for (int i = 1; i <= numServers; i++) {

				if (i == mID)
					continue;

				int copyLength = lastIndex - serverLogMaxIndex[i] + 1;
				Entry[] entries = (copyLength >= 0) ? new Entry[copyLength] : new Entry[0];

				for (int j = 0; j < copyLength; j++) {
					entries[j] = mLog.getEntry(j + serverLogMaxIndex[i]);
				}

				int prevLogTerm = (mLog.getEntry(serverLogMaxIndex[i] - 1) != null)
						? mLog.getEntry(serverLogMaxIndex[i] - 1).term
						: mLog.getEntry(serverLogMaxIndex[i]).term;

				this.remoteAppendEntries(i, term, mID, serverLogMaxIndex[i], prevLogTerm, entries, mCommitIndex);
			}

		}
	}

	// @param candidate’s term
	// @param candidate requesting vote
	// @param index of candidate’s last log entry
	// @param term of candidate’s last log entry
	// @return 0, if server votes for candidate; otherwise, server's
	// current term
	public int requestVote(int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
		synchronized (mLock) {
			int term = mConfig.getCurrentTerm();
			int lastTerm = mLog.getLastTerm();
			if (candidateTerm <= term) {

				return term;
			}

			if (candidateTerm > term) {
				heartbeatTimer.cancel();
				if (!isDead) {
					isDead = true;
					RaftServerImpl.setMode(new FollowerMode());
				}
			}
			return 0;
		}
	}

	// @param leader’s term
	// @param current leader
	// @param index of log entry before entries to append
	// @param term of log entry before entries to append
	// @param entries to append (in order of 0 to append.length-1)
	// @param index of highest committed entry
	// @return 0, if server appended entries; otherwise, server's
	// current term
	public int appendEntries(int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries,
			int leaderCommit) {
		synchronized (mLock) {
			int term = mConfig.getCurrentTerm();

			if (leaderID == mID) {
				return 0;
			}

			if (leaderTerm < term) {
				return term;
			}

			if (mLog.getEntry(prevLogIndex) == null || mLog.getEntry(prevLogIndex).term != prevLogTerm) {
				mLog.append(entries);
			}

			if (entries.length == 0)
				return term;

			return 0;
		}
	}

	// @param id of the timer that timed out
	public void handleTimeout(int timerID) {
		synchronized (mLock) {

			int term = mConfig.getCurrentTerm();

			if (timerID == heartbeatTimerID) {
				//System.out.println("heartbeat timer");
				RaftResponses.setTerm(term);
				int[] results = RaftResponses.getAppendResponses(term);
				heartbeatTimer.cancel();

				for (int i = 1; i < results.length; i++) {
					if (results[i] == 0) {
						serverLogMaxIndex[i] = mLog.getLastTerm() + 1;
					} else if (results[i] == LOG_FAIL) {
						serverLogMaxIndex[i] -= 1;
					}
					else if (results[i] > term) {
						if(!isDead) {
							isDead = true;
							RaftServerImpl.setMode(new FollowerMode());
						}
						
						return;
					}
				}

				RaftResponses.clearAppendResponses(term);
				logRepair();
				heartbeatTimer = scheduleTimer(HEARTBEAT_INTERVAL, heartbeatTimerID);

			}

		}
	}
}
