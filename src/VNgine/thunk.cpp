#include <VNgine/thunk.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>
#include <thread>


namespace VNgine
{

void* volatile ThunkHeap::bytecode_heap = nullptr;
std::mutex ThunkHeap::heap_mutex;

ThunkHeap::ThunkHeap()
{
	/* This code will only run once. It is a mechanism to create a heap in a thread-safe way. */

	// Double checked locking, guaranteed by Acquire/Release-semantics in Microsoft's
	// volatile-implementation.
	if (!bytecode_heap)
	{
		std::scoped_lock lock{ heap_mutex };
		if (!bytecode_heap)
		{
			// Make a heap with execute permissions to store generated machine code in
			bytecode_heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
			if (!bytecode_heap)
			{
				assert(!"Failed to create heap.");
			}

			// Schedule the heap to be destroyed when the application terminates.
			// Until then, it will manage its own size.
			atexit(cleanup);
		}
	}
}

void* ThunkHeap::alloc(std::size_t size)
{
  return HeapAlloc(bytecode_heap, 0, size);
}

void ThunkHeap::dealloc(void* mem)
{
  HeapFree(bytecode_heap, 0, mem);
}

void ThunkHeap::cleanup()
{
	HeapDestroy(bytecode_heap);
}

}