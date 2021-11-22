///////////////////////////////////////////////////////////////////////////////
//	Circular Buffer
//	- The circular buffer is a fixed size (and expandable/contractable) queue. 
//	- Queue elements are user defined. 
//	- operator [] () is the primary overload. The queue is meant to be indexed
//	  into as thouth it were an array.
//	- Storage is allocated as a fixed block for processor caching optimization.
//	- When storage is resized, data elements maintain the same indexes
///////////////////////////////////////////////////////////////////////////////

#pragma once
class CirBuffer
{
public:
	CirBuffer();
	~CirBuffer();
};

