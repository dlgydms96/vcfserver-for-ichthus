//------------------------------------------------------------------------------
// Taken from https://www.codeproject.com/Articles/1169105/Cplusplus-std-thread-Event-Loop-with-Message-Queue
//------------------------------------------------------------------------------

#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "Fault.h"

using namespace std;

struct UserMsg {
  UserMsg() { arg1 = 0; arg2 = 0; }
  UserMsg(int a1, int a2, char *s) { arg1 = a1; arg2 = a2; msg = string(s); }
  int arg1;
  int arg2;
  string msg;
};


struct ThreadMsg {
  ThreadMsg(int i, const void* m) { id = i; msg = m; }
  int id;
  const void* msg;
};


class VThread {
public:
  /// Constructor
  VThread(const char* threadName,
	  void (*func1)(VThread *, ThreadMsg *),
	  void (*func2)(VThread *));

  /// Destructor
  ~VThread();

  /// Called once to create the worker thread
  /// @return TRUE if thread is created. FALSE otherwise. 
  bool CreateThread();

  /// Called once a program exit to exit the worker thread
  void ExitThread();

  /// Get the ID of this thread instance
  /// @return The worker thread ID
  std::thread::id GetThreadId();

  /// Get the ID of the currently executing thread
  /// @return The current thread ID
  static std::thread::id GetCurrentThreadId();

  /// Add a message to thread queue. 
  /// @param[in] data - thread specific information created on the heap using operator new.
  void PostMsg(const UserMsg* data);

  /// added by khkim
  const char* get_thread_name() { return thread_name; }

 private:
  VThread(const VThread&);
  VThread& operator=(const VThread&);

  /// Entry point for the worker thread
  void Process();

  std::thread* m_thread;
  std::queue<ThreadMsg*> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  const char* thread_name;

  /// added by khkim
  void (*normal_handler)(VThread *t, ThreadMsg *msg);
  void (*exit_handler)(VThread *t);
};


#define MSG_EXIT_THREAD			1
#define MSG_POST_USER_MSG		2

VThread::VThread(const char* threadName,
		 void (*func1)(VThread *, ThreadMsg *),
		 void (*func2)(VThread *)) {
  m_thread = 0;
  thread_name = threadName;
  normal_handler = func1;
  exit_handler = func2;  
}

VThread::~VThread() {	ExitThread(); }

bool VThread::CreateThread() {
  if (!m_thread)
    m_thread = new thread(&VThread::Process, this);
  return true;
}

std::thread::id VThread::GetThreadId() {
  ASSERT_TRUE(m_thread != 0);
  return m_thread->get_id();
}

std::thread::id VThread::GetCurrentThreadId() {
  return this_thread::get_id();
}

void VThread::ExitThread() {
  if (!m_thread) return;

  // Create a new ThreadMsg
  ThreadMsg* threadMsg = new ThreadMsg(MSG_EXIT_THREAD, 0);

  // Put exit thread message into the queue
  {
    lock_guard<mutex> lock(m_mutex);
    m_queue.push(threadMsg);
    m_cv.notify_one();
  }

  m_thread->join();
  delete m_thread;
  m_thread = 0;
}

void VThread::PostMsg(const UserMsg* data) {
  ASSERT_TRUE(m_thread);

  ThreadMsg* threadMsg = new ThreadMsg(MSG_POST_USER_MSG, data);

  // Add user data msg to queue and notify worker thread
  std::unique_lock<std::mutex> lk(m_mutex);
  m_queue.push(threadMsg);
  m_cv.notify_one();
}

void VThread::Process()
{
  while (1) {
    ThreadMsg* msg = 0;
    {
      // Wait for a message to be added to the queue
      std::unique_lock<std::mutex> lk(m_mutex);
      while (m_queue.empty())
	m_cv.wait(lk);

      if (m_queue.empty())
	continue;

      msg = m_queue.front();
      m_queue.pop();
    }

    switch (msg->id) {
    case MSG_POST_USER_MSG: {
      ASSERT_TRUE(msg->msg != NULL);

      // Convert the ThreadMsg void* data back to a UserMsg* 
      const UserMsg* umsg = static_cast<const UserMsg*>(msg->msg);
      normal_handler(this, msg);
	
      // Delete dynamic data passed through message queue
      delete umsg;
      delete msg;
      break;
    }

    case MSG_EXIT_THREAD: {
      delete msg;
      std::unique_lock<std::mutex> lk(m_mutex);
      while (!m_queue.empty()){
	msg = m_queue.front();
	m_queue.pop();

	switch (msg->id) {
	case MSG_POST_USER_MSG: {
	  ASSERT_TRUE(msg->msg != NULL);

	  // Convert the ThreadMsg void* data back to a UserMsg* 
	  const UserMsg* umsg = static_cast<const UserMsg*>(msg->msg);
	  normal_handler(this, msg);
	
	  // Delete dynamic data passed through message queue
	  delete umsg;
	  delete msg;
	  break;
	}
	default:
	  ASSERT();
	}
      }
      exit_handler(this);
      return;
    }

    default:
      ASSERT();
    }
  }
}
