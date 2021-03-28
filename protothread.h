/* Copyright is waived. No warranty is provided. Unrestricted use and modification is permitted. */

/*
	Lightweight co-operative threads

	A protothread is a function that can yield (return) at any location and resume execution from
	that point the next time the function is called. This is sometimes known as a Local Continuation.

	For example,

	bool myfunc (void)
	{
		PROTO_STATE;		// Defines the state variables for the protothread
		PROTO_BEGIN;		// Required macro at start of function
		...					// Code here is executed on the first call to the function...
		PROTO_YIELD;		// Now yield. This causes the function to return.
		...					// Code here is executed on the second call to the function...
		PROTO_END;			// Required macro at end of function
	}

	There are a number of limitations and requirements on a protothread;
	- A protothread function always returns a boolean result. If the function has yielded then it
	  returns false. If the function has ended (reached the PROTO_END macro) then it returns true.
	  The macros will take care of returning the appropriate values.
	- You may also return from the function with the 'return' statement. The next time the function
      is called execution will commence from the most recent yield. Please follow the convention of
      returning false if the function is still active, and true if the function has ended.
	- A protothread cannot be yielded from within a switch statement. The yielding functions are
      PROTO_YIELD, PROTO_SLEEP and PROTO_WAIT,  these cannot be used inside a switch.
	- The state of local variables are not preserved across calls to a protothread function,
      therefore local variables should not be used across a yield. Use static variables or a pointer
      to external variables when information must be preserved across a yield.

	The PROTO_STATE macro defines the protothread state as static variables. Hence there is only a single
	instance of the function (i.e. calling the function multiple times updates a single set of state variables).
	It is possible to have multiple instances of the function by passing in the state variables instead,
	for example;

	bool myfunc (ProtoState* ps)
	{
		PROTO_BEGIN;
		...

	Here the state is passed as a parameter.  It's important that the parameter name is exactly 'ps' as the
	macros rely on this. You can execute multiple instances of the function by maintaining multiple copies
	of the state variables and passing the appropriate one at each call. Note that you should not use the
	PROTO_STATE macro in this case.

	If you use other variables in the function then you should also pass in distinct copies of those too,
	for example;

	bool myfunc (ProtoState* ps, void* vars)
	{
		xyz* p = (xyz*) vars;		// Cast the pointer to a struct containing variables for this instance
		PROTO_BEGIN;
		...

	It's useful to note that any code prior to the PROTO_BEGIN macro is executed every time the function is
	called. Any code after the PROTO_END macro will *never* be executed. For example;

	bool myfunc (ProtoState* ps, void* vars)
	{
		// Code here will always be executed
		PROTO_BEGIN;
		...
		PROTO_END;
		// Code here will never be executed
	}

	Member functions may also be protothreads by placing the state variables in the object,
	for example;

	class myclass {
		ProtoState s;
	public:
		bool myfunc (void);
	}

	bool myclass::myfunc (void)
	{
		ProtoState* ps = &s;		// Create the necessary pointer to the state variables
		PROTO_BEGIN;				// proceed as usual
		...
	}

	Note in this case you don't have to pass in private variables, because the member variables already exist
	on a per-instance basis.
*/

#pragma once

// Protothread state variables
class ProtoState {
public:
	ProtoState(void) { state = 0; time = 0.0f; yield = false; }
	int   state;
	float time;
	bool  yield;
};

// Workaround for compiler error when using __LINE__ with 'database for edit and continue'
#define CAT(X,Y) X##Y
#define CAT2(X,Y) CAT(X,Y)
#define LINENO int(CAT2(__LINE__,U)) 

// Macros to implement protothreading in a function
#define PROTO_STATE    static ProtoState s; static ProtoState* ps = &s;											// Locally instance protothread state variables
#define PROTO_BEGIN    ps->yield = false; switch(ps->state) { case 0:											// Begin protothread processing
#define PROTO_YIELD    {ps->yield = true; ps->state = LINENO; case LINENO: if (ps->yield) return false;}			// Yield protothread until next update
#define PROTO_SLEEP(t) for (ps->time = t; ps->time > 0; ps->time -= Timer::GetElapsedTime()) PROTO_YIELD;		// Sleep protothread for specified time (in seconds)
#define PROTO_WAIT(c)  while (!(c)) PROTO_YIELD;																// Yield protothread until specified condition is true
#define PROTO_END      {ps->state = LINENO; case LINENO: break;}} return true;									// Mark end of protothread processing

class ProtoThread {
public:
	// Protothread function prototype
	typedef bool (*Function)(ProtoState* ps, void* vars);

	ProtoThread  (ProtoThread::Function f, void* vars, size_t var_size);
	~ProtoThread (void);

private:
	Function   function;	// Pointer to function to execute
	ProtoState state;		// State variables for this ProtoThread
	void*      vars;		// Pointer to user variables for ProtoThread
};
