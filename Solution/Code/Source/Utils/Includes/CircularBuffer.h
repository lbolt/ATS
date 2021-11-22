///////////////////////////////////////////////////////////////////////////////
//	Circular Buffer Data Structure
//	- Used for Data Collection Buffering and Analytics Processing Data 
//	  Structures (works like a shift-register).
//
//
//	- Contain Context Block
//		The container context block is a wrapper for each instance of
//		the user defined element placed in the buffer. The container
//		begins at the origin block which is always the zeroth element.
//
//		Attributes:
//
//		Iterator - Integer value beginning at 0, where 0 corresponds to
//		origin (first) element block in container
//
//		uid - Integer value that uniquely identifies element and is not 
//		tied to iterator. The UID is necessary in order to find a
//		particular element between accesses to the buffer. The buffer may
//		change in size due to expansion or contraction of the container.
//
//		State - Enumerated type that identifies the operational state of the
//		buffer element. The element may be NEW, BUSY or EMPTY (not in use).
//		NEW items are waiting to be process starting at the front of the 
//		buffer. BUSY items are being processed by caller threads. EMPTY
//		item slots are ready for recovery.
//
//		Element - User defined type <T> 
//
//
//	- Notes
//		-	Public method access is synchronized via critical sections
//		-	Memory address proximity adventitious for processor for caching
//		-	Small footprint due to expand and contract capability
//		-	Buffer iteration leverages indexing using modulus calculation
//		-	**No template based iterator
//		-	Container expansion/contraction uses slot-by-slot memory copy
//		-	Implements context block state, unique identifier and insertion 
//			time in microseconds
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ATS_CODES.h"
#include "../Includes/SafeSyncX.h"
#include <cstring>
#include <chrono>

#pragma warning(disable:4503 4786)

using namespace ATS_CODES;



namespace ATS_DS {


#define CB_INITIAL_SIZE		100
#define CB_INCREMENT_SIZE	100
#define CB_MASTER_UID		1000000 // default master uid (unique identifier) range 1m up to 2**31 (signed)


	enum CIRCULAR_BUFFER_STATES
	{
		EMPTY = 0,
		NEW,
		BUSY
	};

	enum ATS_SYNC
	{
		ATS_WITH_NOSYNC = 0,
		ATS_WITH_SYNC
	};

	typedef unsigned int		Container_forward_iterator;
	typedef unsigned int		Container_element_uid;
	typedef unsigned __int64	Container_element_insert_tm;


	struct alignas(8) CONTAINER_ELEMENT_ATTR
	{
		unsigned __int64		time_inserted;
		CIRCULAR_BUFFER_STATES	state;
		unsigned int			uid;
	};


	template <typename T>
	struct alignas(8) CONTAINER_CTX_BLOCK
	{
		//CONTAINER_ELEMENT_ATTR	attr;
		unsigned __int64		time_inserted;	// Local time
		CIRCULAR_BUFFER_STATES	state;			// State of the buffer element
		unsigned int			uid;			// Unique buffer insertion stamp (copy ++master_uid)
		T						element;		// Buffer element
	};



	template <typename T, bool _SyncEnabled=ATS_WITH_NOSYNC>
	class CircularBuffer
	{
	public:
		CircularBuffer();
		CircularBuffer(int);
		CircularBuffer(int, int);
		~CircularBuffer();

		// Methods
		unsigned int size_of_container(void);	// Total number of slots in container
		unsigned int num_active_elements(void);	// Number of slots in container whose state != EMPTY
		ATS_CODE set_element_state(Container_forward_iterator&, CIRCULAR_BUFFER_STATES);
		ATS_CODE set_element_state_by_uid(Container_element_uid&, CIRCULAR_BUFFER_STATES);
		

		// Synchronized Access Methods
		Container_forward_iterator begin(void);
		Container_forward_iterator end(void);
		ATS_CODE insert_back(T*);
		ATS_CODE isActiveElement(Container_forward_iterator&, bool&);
		ATS_CODE invalidate_entry(Container_element_uid&);
		ATS_CODE empty(void);					// Invalidate all elements
		ATS_CODE trim_front(unsigned int);		// Invalidate n elements from front (oldest) of buffer

		Container_forward_iterator	container_iterator_incr(Container_forward_iterator&); // (Change to forward iterator)

		// Externally initiated lock
		int buffer_lock(void); 
		int buffer_release(void); 

		// Both Synchronization methods
		ATS_CODE find_element_by_uid(Container_element_uid&, Container_forward_iterator&);
		ATS_CODE find_successor_uid(Container_element_uid&, Container_element_uid&);
		ATS_CODE convert_forward_iterator_to_addr(Container_forward_iterator&, CONTAINER_CTX_BLOCK<T>*&);
		ATS_CODE convert_uid_to_addr(Container_element_uid&, CONTAINER_CTX_BLOCK<T>*&);

		// Operators
		CircularBuffer& operator++(void);	// Pre-increment (not implemented - use an iterator)
		CircularBuffer operator++(int);		// Post-increment (not implemented - use an iterator)

		AutoCriticalSection cb_sso;
		CS_Sync_External cb_ext_sso;

	private:
		// Hide
		CircularBuffer(CircularBuffer&);
		CircularBuffer(T&);

		// Methods
		int	get_element_size(void);	
		int get_buffer_size(void);
		int	get_T_size(void);

		ATS_CODE initialize_container(int, int); // Synchronized
		ATS_CODE get_origin_addr(CONTAINER_CTX_BLOCK<T>*&);
		ATS_CODE expand_container(void);
		ATS_CODE compress_container();

		ATS_CODE get_element_attrs(Container_forward_iterator&, CONTAINER_ELEMENT_ATTR&);
		ATS_CODE get_element_attrs_by_uid(Container_element_uid&, CONTAINER_ELEMENT_ATTR&);	// Not used
		ATS_CODE get_element_uid(Container_forward_iterator&, Container_element_uid&);
		CIRCULAR_BUFFER_STATES	get_element_state(Container_forward_iterator&);
		//ATS_CODE get_next_new(T*, Container_element_uid&);
		//ATS_CODE get_next_index(Container_forward_iterator&, Container_forward_iterator&);

		//	Data
		unsigned __int64 time_of_last_entry;
		unsigned __int64 element_timeout_period;

		Container_forward_iterator	buffer_begin;
		Container_forward_iterator	buffer_end;
		CONTAINER_CTX_BLOCK<T>* container = NULL;

		int initial_num_elements = CB_INITIAL_SIZE;
		int	expansion_num_elements = CB_INCREMENT_SIZE;
		int size_container;			// Total number of element slots in container
		int	size_active;			// Number of active (non EMPTY) elements slots in container
		unsigned int master_uid;	// Provides unique stamp for each buffer element (2**32 - CB_MASTER_UID)
		bool container_synchronization;

	};

	//	CTOR
	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled>::CircularBuffer() : container_synchronization(_SyncEnabled)
	{
		ATS_CODE rc;
		//	Uses default container size
		rc = initialize_container(initial_num_elements, CB_MASTER_UID);
	}

	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled>::CircularBuffer(int size) : container_synchronization(_SyncEnabled)
	{
		ATS_CODE rc;
		if (size >= 100 && (size%100) == 0)
			initial_num_elements = size;
		rc = initialize_container(initial_num_elements, CB_MASTER_UID);
	}

	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled>::CircularBuffer(int size, int master_uid_override) : container_synchronization(_SyncEnabled)
	{
		ATS_CODE rc;
		int starting_uid = CB_MASTER_UID;
		if (master_uid_override >= CB_MASTER_UID)
		{
			starting_uid = master_uid_override;
		}
		if (size >= 100 && (size % 100) == 0)
			initial_num_elements = size;
		rc = initialize_container(initial_num_elements, starting_uid);
	}

	//	DTOR
	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled>::~CircularBuffer()
	{
		//	Remove Heap Objects
		if (container != NULL)
			delete [] container;
	}



	//	METHODS
	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::initialize_container(int size, int uid_start_value)
	{
		if (container_synchronization)
		{
			// Using IMPLIED Synchronization
			AutoCriticalSectionLock cb_sso(cb_sso);
		}

		try
		{
			// Create first instance of container
			// This is a contiguous block of storage size amount of 
			// CONTAINER_CTX_BLOCK<T> elements
			int container_size_in_bytes = size * sizeof(CONTAINER_CTX_BLOCK<T>);
			container = new CONTAINER_CTX_BLOCK<T> [container_size_in_bytes];
			memset((void*)container, 0, container_size_in_bytes);
		}
		catch (...)
		{
			return (ATS_C_NOMEM);
		}

		buffer_begin = (Container_forward_iterator)0;
		buffer_end = (Container_forward_iterator)0;
		size_active = 0;
		size_container = size;
		master_uid = uid_start_value;
		return (ATS_C_SUCCESS);
	}

	
	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::insert_back(T* t)
	{
		if(container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		// Note:
		//	- buffer_end is the next available slot
		//	- If end == begin then we may need to expand
		//	- Elements copied to the new container begin at its origin; UID sequence for elements does not change
		//
		ATS_CODE rc;
		if ((buffer_end == buffer_begin) && (size_active != 0))
		{
			rc = expand_container();
		}
		CONTAINER_CTX_BLOCK<T>* container_addr;
		rc = convert_forward_iterator_to_addr(buffer_end, container_addr); // Insert at buffer end
		container_addr->uid = ++master_uid;
		container_addr->time_inserted = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		time_of_last_entry = container_addr->time_inserted;
		container_addr->state = CIRCULAR_BUFFER_STATES::NEW;
		// Copy new element data
		memcpy(&container_addr->element, (void*)t, get_T_size());
		++size_active;
		buffer_end = container_iterator_incr(buffer_end);
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::expand_container(void)
	{
		ATS_CODE rc= ATS_C_SUCCESS;
		int elem_size = get_element_size();
		int new_num_elements = size_of_container() + expansion_num_elements;
		int new_container_size_in_bytes = new_num_elements * elem_size;
		CONTAINER_CTX_BLOCK<T>* new_container=NULL;
		try
		{
			new_container = new CONTAINER_CTX_BLOCK<T> [new_container_size_in_bytes];
			memset((void*)new_container, 0, new_container_size_in_bytes);
		}
		catch (...)
		{
			return (ATS_C_NOMEM);
		}

		Container_forward_iterator src_it = buffer_begin;
		Container_forward_iterator src_it_end = buffer_end;
		Container_forward_iterator dst_it = src_it;	//	Start from the same place
		CONTAINER_CTX_BLOCK<T>* src_slot_addr;
		void* dst_slot_addr;
		unsigned int offset;

		for (int count = num_active_elements(); count > 0; count--)
		{
			//	Get container slot addresses
			convert_forward_iterator_to_addr(src_it, src_slot_addr);
			offset = (dst_it * elem_size);
			dst_slot_addr = new_container + offset;
			//	Copy slot to new container
			memcpy(dst_slot_addr, (void*)src_slot_addr, elem_size);
			src_it = container_iterator_incr(src_it);
			//	iterator increment must account for wrap-around
			dst_it = (dst_it + 1) % new_num_elements;
		}

		delete [] container;
		container = new_container;
		buffer_end = dst_it;
		size_container += expansion_num_elements;
		return (rc);
	}

	
	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::invalidate_entry(Container_element_uid& uid)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc;
		// Find element by UID return iterator
		Container_forward_iterator it;
		rc = find_element_by_uid(uid, it);
		if (rc != ATS_C_SUCCESS)
		{
			return (ATS_C_NONEXIST);
		}
		// Set element state to EMPTY
		rc = set_element_state(it, CIRCULAR_BUFFER_STATES::EMPTY);
		
		// Compress buffer
		rc = compress_container();

		return (rc);
	}


	template <typename T, bool _SyncEnabled>						// <-----------------------TODO: Test this method
	ATS_CODE CircularBuffer<T, _SyncEnabled>::empty(void)
	{
		ATS_CODE rc;
		// Iterate from begin to either first non-empty or end of buffer
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		while (it != it_end)
		{
			rc = set_element_state(it, CIRCULAR_BUFFER_STATES::EMPTY);
			it = container_iterator_incr(it);
		}
		// Compress buffer
		rc = compress_container();
		return (rc);
	}


	template <typename T, bool _SyncEnabled>						// <-----------------------TODO: Test this method
	ATS_CODE CircularBuffer<T, _SyncEnabled>::isActiveElement(Container_forward_iterator& it, bool& bRet)
	{
		ATS_CODE rc = ATS_C_SUCCESS;

		CIRCULAR_BUFFER_STATES state = get_element_state(it);
		if (state != CIRCULAR_BUFFER_STATES::EMPTY)
			bRet = TRUE;
		else
			bRet = FALSE;
		return (rc);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::trim_front(unsigned int num_slots)	// <-----------------------TODO: Retest this method
	{
		ATS_CODE rc = ATS_C_SUCCESS;
		// Sets front (n) active slots in buffer to EMPTY - does not count slots that are already empty
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		CONTAINER_CTX_BLOCK<T>* container_addr = NULL;
		int count = num_slots;
		if (num_slots > num_active_elements())
		{
			return ATS_C_RANGE;
		}
		while (it != it_end)
		{
			convert_forward_iterator_to_addr(it, container_addr);
			if (container_addr->state != CIRCULAR_BUFFER_STATES::EMPTY)
			{
				container_addr->state = CIRCULAR_BUFFER_STATES::EMPTY;
				--count;
			}
			if (count == 0)
			{
				break;
			}
			it = container_iterator_incr(it);
		}
		// Compress buffer
		rc = compress_container();
		return (rc);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::find_element_by_uid(Container_element_uid& uid, Container_forward_iterator& new_it)
	{
		// Find element with matching UID
		ATS_CODE rc = ATS_C_NONEXIST;
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		CONTAINER_CTX_BLOCK<T>* container_addr=NULL;
		while (it != it_end)
		{
			convert_forward_iterator_to_addr(it, container_addr);
			if (container_addr->uid == uid)
			{
				new_it = it;
				rc = ATS_C_SUCCESS;
				break;
			}
			if (container_addr->uid > uid)
			{
				rc = ATS_C_NONEXIST;
				break;
			}
			it = container_iterator_incr(it);
		}
		return (rc);
	}

	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::find_successor_uid(Container_element_uid& uid, Container_element_uid& successor_uid)
	{
		// Find next successive non-EMPTY uid
		ATS_CODE rc = ATS_C_NONEXIST;
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		CONTAINER_CTX_BLOCK<T>* container_addr = NULL;
		while (it != it_end)
		{
			convert_forward_iterator_to_addr(it, container_addr);
			if (container_addr->state != CIRCULAR_BUFFER_STATES::EMPTY)
			{
				if (container_addr->uid > uid)
				{
					successor_uid = container_addr->uid;
					rc = ATS_C_SUCCESS;
					break;
				}
			}
			it = container_iterator_incr(it);
		}
		return (rc);
	}

	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::compress_container()
	{
		// Iterate from begin to either first non-empty or end of buffer
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		while (it != it_end)
		{
			if (get_element_state(it) == CIRCULAR_BUFFER_STATES::EMPTY)
			{
				it = container_iterator_incr(it);
				buffer_begin = it;	// Move it up by one position
				--size_active;
			}
			else
			{
				break;
			}
		}
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	Container_forward_iterator CircularBuffer<T, _SyncEnabled>::begin(void)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		return(buffer_begin);

	}


	template <typename T, bool _SyncEnabled>
	Container_forward_iterator CircularBuffer<T, _SyncEnabled>::end(void)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		return(buffer_end);

	}


	template <typename T, bool _SyncEnabled>
	int CircularBuffer<T, _SyncEnabled>::get_element_size(void)
	{
		return (sizeof(CONTAINER_CTX_BLOCK<T>));
	}

	template <typename T, bool _SyncEnabled>
	int CircularBuffer<T, _SyncEnabled>::get_buffer_size(void)
	{
		return (size_container * get_element_size());
	}


	template <typename T, bool _SyncEnabled>
	int CircularBuffer<T, _SyncEnabled>::get_T_size(void)
	{
		return (sizeof(T));
	}


	template <typename T, bool _SyncEnabled>
	CIRCULAR_BUFFER_STATES CircularBuffer<T, _SyncEnabled>::get_element_state(Container_forward_iterator& it)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		rc = convert_forward_iterator_to_addr(it, container_addr);
		return (container_addr->state);
	}

	
	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::set_element_state(Container_forward_iterator& it, CIRCULAR_BUFFER_STATES state)
	{
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		rc = convert_forward_iterator_to_addr(it, container_addr);
		container_addr->state = state;
		return (ATS_C_SUCCESS);

#ifdef _UNUSED_
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		CIRCULAR_BUFFER_STATES current_state;

		rc = convert_forward_iterator_to_addr(it, container_addr);
		current_state = container_addr->state;

		if (current_state == CIRCULAR_BUFFER_STATES::EMPTY && state == CIRCULAR_BUFFER_STATES::EMPTY)
		{
			return ATS_C_SUCCESS;
		}

		if(current_state == CIRCULAR_BUFFER_STATES::EMPTY && state != CIRCULAR_BUFFER_STATES::EMPTY)
		{
			container_addr->state = state;
			++size_active;
		}
		else if (current_state != CIRCULAR_BUFFER_STATES::EMPTY && state == CIRCULAR_BUFFER_STATES::EMPTY)
		{
			container_addr->state = state;
			--size_active;
		}
		else if (current_state != CIRCULAR_BUFFER_STATES::EMPTY && state != CIRCULAR_BUFFER_STATES::EMPTY)
		{
			container_addr->state = state;
		}

		return ATS_C_SUCCESS;
#endif _UNUSED_
	}

	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::set_element_state_by_uid(Container_element_uid& uid, CIRCULAR_BUFFER_STATES state)	//<---------FIX: See above for active count management
	{
		ATS_CODE rc = ATS_C_SUCCESS;
		CONTAINER_CTX_BLOCK<T>* ctx_addr;
		// Convert uid to container address
		rc = convert_uid_to_addr(uid, ctx_addr);
		if (rc == ATS_C_SUCCESS)
		{
			ctx_addr->state = state;
		}
		return rc;
	}

	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_origin_addr(CONTAINER_CTX_BLOCK<T>*& container_block)
	{
		container_block = container;
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::convert_forward_iterator_to_addr(Container_forward_iterator& it, CONTAINER_CTX_BLOCK<T>*& new_ctx)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);

		ATS_CODE rc= ATS_C_SUCCESS;
		CONTAINER_CTX_BLOCK<T>* origin_addr;
		rc = get_origin_addr(origin_addr);
		if (rc == ATS_C_FAIL)
		{
			new_ctx = NULL;
			return (rc);
		}
		CONTAINER_CTX_BLOCK<T>* _origin_addr;
		unsigned int offset = it * get_element_size();
		_origin_addr = origin_addr + offset;
		new_ctx = _origin_addr;
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_element_uid(Container_forward_iterator& it, Container_element_uid& uid)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		rc = convert_forward_iterator_to_addr(it, container_addr);
		uid = container_addr->uid;
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_element_attrs(Container_forward_iterator& it, CONTAINER_ELEMENT_ATTR& attrs)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		// Validate the iterator range
		///Container_forward_iterator _it = CONTAINER_ITERATOR_MOD_CHECK(it);
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		rc = convert_forward_iterator_to_addr(it, container_addr);
		if (rc == ATS_C_FAIL)
			return(rc);
		attrs.uid = container_addr->uid;
		attrs.state = container_addr->state;
		attrs.time_inserted = container_addr->time_inserted;
		return (ATS_C_SUCCESS);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_element_attrs_by_uid(Container_element_uid& uid, CONTAINER_ELEMENT_ATTR& attrs) // fix: should be an overload
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		Container_forward_iterator it;
		ATS_CODE rc = ATS_C_SUCCESS;

		rc = find_element_by_uid(uid, it);
		if (rc != ATS_C_SUCCESS)
		{
			goto _get_element_attrs_by_uid;
		}

		rc = get_element_attrs(it, attrs);

	_get_element_attrs_by_uid:
		return (rc);
	}

	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::convert_uid_to_addr(Container_element_uid& uid, CONTAINER_CTX_BLOCK<T>*& ctx_addr) // fix: should be an overload
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc = ATS_C_NONEXIST;
		CONTAINER_CTX_BLOCK<T>* container_addr = NULL;
		Container_forward_iterator it = buffer_begin;
		Container_forward_iterator it_end = buffer_end;
		while (it != it_end)
		{
			convert_forward_iterator_to_addr(it, container_addr);
			if (container_addr->state != CIRCULAR_BUFFER_STATES::EMPTY)
			{
				if (container_addr->uid == uid)
				{
					ctx_addr = container_addr;
					rc = ATS_C_SUCCESS;
					break;
				}
			}
			it = container_iterator_incr(it);
		}
		return (rc);
	}

	template <typename T, bool _SyncEnabled>
	unsigned int CircularBuffer<T, _SyncEnabled>::size_of_container(void)
	{
		return (size_container);
	}


	template <typename T, bool _SyncEnabled>
	unsigned int CircularBuffer<T, _SyncEnabled>::num_active_elements(void)
	{
		return (size_active);
	}


	template <typename T, bool _SyncEnabled>
	int CircularBuffer<T, _SyncEnabled>::buffer_lock()
	{
		cb_ext_sso.lock();
		return (ATS_OK);
	}


	template <typename T, bool _SyncEnabled>
	int CircularBuffer<T, _SyncEnabled>::buffer_release()
	{
		cb_ext_sso.release();
		return (ATS_OK);
	}

	///////////////////////////////////////////////////////
	//	NOT CURRENTLY USED
	///////////////////////////////////////////////////////
#ifdef _UNUSED_
	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_next_new(T* element_buf, Container_element_uid& next_uid)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc;
		CONTAINER_CTX_BLOCK<T>* container_addr;
		Container_forward_iterator it = buffer_begin;
		int count;
		for (count = 0; count < size_active; count++)
		{
			if (get_element_state(it) == CIRCULAR_BUFFER_STATES::NEW)
			{
				rc = convert_forward_iterator_to_addr(it, container_addr);
				memcpy((void*)element_buf, (void*)&container_addr->element, get_T_size());
				container_addr->state = CIRCULAR_BUFFER_STATES::BUSY;
				next_uid = container_addr->uid;
				return (ATS_C_SUCCESS);
			}
			it = container_iterator_incr(it);
		}

		return (ATS_C_EMPTY);
	}


	template <typename T, bool _SyncEnabled>
	ATS_CODE CircularBuffer<T, _SyncEnabled>::get_next_index(Container_forward_iterator& current, Container_forward_iterator& next)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		ATS_CODE rc = ATS_C_SUCCESS;
		Container_forward_iterator it = current;
		// Validate iterator range
		if (current >= 0 && current <= (size_of_container - 1))
		{
			next = container_iterator_incr(it);	// No end check
		}
		else
		{
			rc = ATS_C_RANGE;
		}
		return (rc);
	}
#endif _UNUSED_
	
	template <typename T, bool _SyncEnabled>
	Container_forward_iterator CircularBuffer<T, _SyncEnabled>::container_iterator_incr(Container_forward_iterator& it)
	{
		if (container_synchronization)
			AutoCriticalSectionLock cb_sso(cb_sso);
		Container_forward_iterator _it = it;
		_it = (_it + 1) % size_container;
		return (_it);
	}

	//////////////////////////////////////////////////////////////////////////////
	//	Operators
	//////////////////////////////////////////////////////////////////////////////
	// Pre-increment operator <--------------------FIX
	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled>& CircularBuffer<T, _SyncEnabled>::operator++()
	{
		//++pToThis; 
		//return *this;
	}
	
	// Post-increment operator <--------------------FIX
	template <typename T, bool _SyncEnabled>
	CircularBuffer<T, _SyncEnabled> CircularBuffer<T, _SyncEnabled>::operator++(int)
	{
		//MyIterator tmp(*this); 
		//operator++(); 
		//return tmp;
	}



















} // end namespace ATS

