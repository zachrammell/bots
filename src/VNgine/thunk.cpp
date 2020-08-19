#include <VNgine/thunk.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>
#include <thread>


namespace VNgine
{

void* volatile ThunkBase::bytecode_heap = nullptr;
std::mutex ThunkBase::heap_mutex;

ThunkBase::ThunkBase(void* instance, void* function)
{
	/* This part will only run once. It is a mechanism to create a heap in a thread-safe way. */

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
			atexit(cleanupHeap);
		}
	}

	/* This part will run every time. It just allocates the bytecode object into executable memory. */

	// Grab the bytecode-sized chunk of executable memory from the heap
	bytecode_ = static_cast<Bytecode*>(HeapAlloc(bytecode_heap, 0, sizeof(Bytecode)));
	if (!bytecode_)
	{
		assert(!"Failed to allocate bytecode on heap.");
	}
	// Use placement new to construct the bytecode in the allocated executable heap memory
	new (bytecode_) Bytecode{ instance, function };
}

ThunkBase::~ThunkBase()
{
	// Explicitly call dtor because it was allocated with placement new
	bytecode_->~Bytecode();
	// Release the executable memory it was placed in
	HeapFree(bytecode_heap, 0, bytecode_);
}

ThunkBase::Bytecode* ThunkBase::getBytecode() const
{
	return bytecode_;
}

void ThunkBase::flushInstructionCache() const
{
	// Flush instruction cache. May be required on some architectures which
  // don't feature strong cache coherency guarantees, though not on neither
  // x86, x64 nor AMD64.
	FlushInstructionCache(GetCurrentProcess(), bytecode_, sizeof(Bytecode));
}

void ThunkBase::cleanupHeap()
{
	HeapDestroy(bytecode_heap);
}


}