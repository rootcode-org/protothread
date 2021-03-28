/* Copyright is waived. No warranty is provided. Unrestricted use and modification is permitted. */

#include <cstring>
#include "protothread.h"

ProtoThread::ProtoThread(ProtoThread::Function f, void* _vars, size_t var_size)
{
	function = f;
	vars = nullptr;

	// Make a copy of the passed variables
	if (_vars)
	{
		vars = new char[var_size];
		memcpy (vars, _vars, var_size);
	}
}


ProtoThread::~ProtoThread (void)
{
	// Free the private copy of the variables
	if (vars) delete vars;
}
