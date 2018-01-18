package edu.duke.raft;

import java.util.Timer;

public class FollowerMode extends RaftMode {
	private Timer leaderFailTimer;
	private int failTimerID = 0;
	private boolean isDead = false;
	private static int LOG_FAIL = -2;

	public void go() {
		synchronized (mLock) {
			
			int term = mConfig.getCurrentTerm();
			//unique timer ID
			
			System.out.println ("S" + 
					  mID + 
					  "." + 
					  term + 
					  ": switched to follower mode.");
			
			long failTimer = ((mConfig.getTimeoutOverride() < 0) ? 
					(long) Math.random() * (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + ELECTION_TIMEOUT_MIN :
					mConfig.getTimeoutOverride());

			leaderFailTimer = scheduleTimer(failTimer, failTimerID);

			isDead = false;
			
		}
	}

	private void resetTimer() {
		leaderFailTimer.cancel();
		long failTimer = ((mConfig.getTimeoutOverride() < 0) ? 
				(long) Math.random() * (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + ELECTION_TIMEOUT_MIN :
				mConfig.getTimeoutOverride());
		leaderFailTimer = scheduleTimer(failTimer, failTimerID);
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
			int voted = mConfig.getVotedFor();
			int log = mLog.getLastTerm();
			
			resetTimer();

			// if candidate less up to date, don't vote
			if (candidateTerm < term) {
				return term;
			}

			if(candidateTerm > term) {
				if(lastLogTerm >= log) {
					
					mConfig.setCurrentTerm(candidateTerm, candidateID);
					return 0;
				}
				mConfig.setCurrentTerm(candidateTerm, 0);
				
				return term;
			}
			
			return term;
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
			// System.out.println(mID+"received heartbeat");
			resetTimer();

			if (leaderTerm < term) {
				return term;
			}

			if (leaderTerm > term) {
				mConfig.setCurrentTerm(leaderTerm, 0);
			}

			//check for problems with the log
			if(mLog.getEntry(prevLogIndex) == null) {
				resetTimer();
				return LOG_FAIL;
			}
			if(mLog.getEntry(prevLogIndex).term != prevLogTerm) {
				resetTimer();
				return LOG_FAIL;
			}
			
			//log is ok, store
			mLog.insert(entries, prevLogIndex, prevLogTerm);
			
			return 0;
		}
	}

	// @param id of the timer that timed out
	public void handleTimeout(int timerID) {
		synchronized (mLock) {
			if (timerID == failTimerID) {
				leaderFailTimer.cancel();

				if (isDead == false) {
					isDead = true;
					RaftServerImpl.setMode(new CandidateMode());
				}
			}

		}
	}
}
