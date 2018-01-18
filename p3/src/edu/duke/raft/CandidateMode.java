package edu.duke.raft;

import java.util.Arrays;
import java.util.Timer;

public class CandidateMode extends RaftMode {

	private Timer electionTimer;
	private int electionTimerID = 1;
	private Timer voteTimer;
	private int voteTimerID = 2;
	private boolean isDead = false;

	public void go() {
		synchronized (mLock) {

			mConfig.setCurrentTerm(mConfig.getCurrentTerm() + 1, mID);

			int term = mConfig.getCurrentTerm();

			// get new timer IDs

			RaftResponses.setTerm(term);
			RaftResponses.clearVotes(term);

			long electionTimeOut = (mConfig.getTimeoutOverride() < 0
					? ((long) Math.random() * (ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + ELECTION_TIMEOUT_MIN)
					: mConfig.getTimeoutOverride());

			electionTimer = scheduleTimer(electionTimeOut, electionTimerID);
			voteTimer = scheduleTimer(25, voteTimerID);

			// vote for self
			//RaftResponses.setVote(mID, 0, term, 0);

			// request votes from other servers
			for (int i = 1; i <= mConfig.getNumServers(); i++) {
					remoteRequestVote(i, term, mID, mLog.getLastIndex(), mLog.getLastTerm());
			}

			if (isDead) {
				isDead = false;
			} else {
				System.out.println("S" + mID + "." + term + ": switched to candidate mode.");
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
			
			if (candidateID == mID) {
				//System.out.println("server" + mID + " Voted for " + candidateID);
				return 0;
			}

			if (candidateTerm > term) {
				if (lastLogTerm >= lastTerm) {
					mConfig.setCurrentTerm(candidateTerm, candidateID);
				} else {
					mConfig.setCurrentTerm(candidateTerm, 0);
				}

				electionTimer.cancel();
				voteTimer.cancel();

				if (!isDead) {
					isDead = true;
					RaftServerImpl.setMode(new FollowerMode());
				}
				return 0;
			}

			if (lastTerm < lastLogTerm) {
				electionTimer.cancel();
				voteTimer.cancel();

				if (!isDead) {
					isDead = true;
					RaftServerImpl.setMode(new FollowerMode());
				}
				
				return 0;
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

			if (leaderTerm < term) {
				return term;
			}

			else {
				//System.out.println(mID + "Received heartbeat");

				voteTimer.cancel();
				electionTimer.cancel();

				mConfig.setCurrentTerm(leaderTerm, leaderID);
				if (!isDead) {
					isDead = true;
					RaftServerImpl.setMode(new FollowerMode());
				}
				return -1;
			}
		}
	}

	// @param id of the timer that timed out
	public void handleTimeout(int timerID) {
		synchronized (mLock) {

			int term = mConfig.getCurrentTerm();

			// vote timer
			if (timerID == voteTimerID) {
				RaftResponses.setTerm(term);
				int[] votes = RaftResponses.getVotes(term);
				int numvotes = 0;

				// count votes
				for (int i = 1; i < votes.length; i++) {
					//System.out.println(votes[i]);

					if (votes[i] <= 0) {
						numvotes++;
					}

					if (votes[i] > term) {
						electionTimer.cancel();
						voteTimer.cancel();
						if (!isDead) {
							isDead = true;
							RaftServerImpl.setMode(new FollowerMode());
						}

						return;
					}
				}

				//System.out.println(numvotes + " out of " + mConfig.getNumServers() + " votes for " + mID);

				// check if elected
				if (numvotes > mConfig.getNumServers() / 2) {
					electionTimer.cancel();
					voteTimer.cancel();
					if (!isDead) {
						isDead = true;
						RaftServerImpl.setMode(new LeaderMode());
					}

					return;
				}

				// not elected, restart vote

				RaftResponses.clearVotes(term);
				for (int i = 1; i <= mConfig.getNumServers(); i++) {
					if (i != mID) {
						remoteRequestVote(i, term, mID, mLog.getLastIndex(), mLog.getLastTerm());
					}
				}
				voteTimer.cancel();
				voteTimer = scheduleTimer(25, voteTimerID);
				return;
			}

			// election timer
			if (timerID == electionTimerID) {
				electionTimer.cancel();
				voteTimer.cancel();
				if (!isDead) {
					isDead = true;
					go();
				}
			}
		}
	}
}
