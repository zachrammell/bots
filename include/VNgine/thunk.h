#pragma once

#include <cassert>
#include <mutex>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>

#include <GLFW/glfw3.h>

#include <VNgine/helper.h>

namespace VNgine
{

class ThunkHeap : non_copyable<ThunkHeap>
{
protected:
  ThunkHeap();
  void* alloc(std::size_t size);
  void dealloc(void* mem);
private:
  static void* volatile bytecode_heap;
  static std::mutex heap_mutex;
  static void __cdecl cleanup();
};

template<std::size_t ArgCount>
class ThunkBase : ThunkHeap
{
protected:
  ThunkBase(void* instance, void* function)
  {
    // Grab the bytecode-sized chunk of executable memory from the heap
    bytecode_ = static_cast<Bytecode<ArgCount>*>(alloc(sizeof(Bytecode<ArgCount>)));
    if (!bytecode_)
    {
      assert(!"Failed to allocate bytecode on heap.");
    }
    // Use placement new to construct the bytecode in the allocated executable heap memory
    new (bytecode_) Bytecode<ArgCount>{ instance, function };
    std::cout << sizeof(Bytecode < ArgCount>) << " bytes of machine code\n";
  }

  ~ThunkBase()
  {
    // Explicitly call dtor because it was allocated with placement new
    bytecode_->~Bytecode<ArgCount>();
    // Release the executable memory it was placed in
    dealloc(bytecode_);
  }

  template <std::size_t ArgC>
  struct Bytecode
  {
    constexpr static uint32_t n = (ArgC - 4);
#pragma pack(push, 1)

    std::uint8_t mov_rax_rcx[3];              // store RCX value for later; RCX is needed for copy operation

    std::uint8_t mov_r10_rsi[3];              // store original RSI and RDI, calling convention says they are nonvolatile
    std::uint8_t mov_r11_rdi[3];

    // Let n = (argc - 4), the number of spilled arguments.
    // Copy $n QWORDs from [rsp + 40] to [rsp - ($n * 8)]
    // This will copy all previously spilled arguments to below the new shadow space
    std::uint8_t mov_rsi_rsp[3];              // SRC = rsp + 40
    std::uint8_t add_rsi_40[4];
    std::uint8_t mov_rdi_rsp[3];              // DST = rsp - (n * 8)
    std::uint8_t add_rdi[3];
    std::uint32_t rsp_copy_offset;
    std::uint8_t mov_rcx_N[3];                // N = n
    std::uint32_t N;
    std::uint8_t rep_movsq[3];                // memcpy(SRC, DST, N)

    std::uint8_t mov_rsi_r10[3];              // restore original RSI and RDI
    std::uint8_t mov_rdi_r11[3];

    std::uint8_t mov_qword_ptr_rsp_r9[4];     // spill the argument from R9 onto the stack
    std::uint32_t spill_offset;

    std::uint8_t sub_rsp[3];                  // create the shadow space for the member callback, plus argument space
    std::uint32_t rsp_grow_offset;            // (32 for shadow space + 8 for spilled arg from r9 + 8 for each arg past 4)

    std::uint8_t mov_r9_r8[3];                // shift the arguments from [RCX, RDX, R8] to [RDX, R8, R9]
    std::uint8_t mov_r8_rdx[3];               // to make room for the `this` ptr in RCX
    std::uint8_t mov_rdx_rax[3];              // RCX was previously stored in RAX because RCX is needed for string ops

    std::uint8_t mov_rcx[2];                  // store the `this` ptr in RCX
    void* this_ptr;

    std::uint8_t mov_rax[2];                  // store the pointer to member function in RAX
    void* function;                           // it may actually be a pointer to vtable but it will still work

    std::uint8_t call_rax[2];                 // execute the member function and allow it to return back here
    std::uint8_t add_rsp_48[4];               // clean up the shadow space and the space for the spilled arguments
                                              // (32 for shadow space + 8 for spilled arg from r9 + 8 for each arg past 4)

    std::uint8_t ret[1];                      // return to whatever called the callback

    std::uint8_t int_3[9];                    // just a bunch of interrupts to stop in case something goes wrong
#pragma pack(pop)
    Bytecode(void* instance, void* function) :

    /* Used https://defuse.ca/online-x86-assembler to translate into machine code */

      mov_rax_rcx{ 0x48, 0x89, 0xC8 },                             // mov rax, rcx

      mov_r10_rsi{ 0x49, 0x89, 0xF2 },                             // mov r10, rsi
      mov_r11_rdi{ 0x49, 0x89, 0xFB },                             // mov r11, rdi

      mov_rsi_rsp{ 0x48, 0x89, 0xE6 },                             // mov rsi, rsp
      add_rsi_40{ 0x48, 0x83, 0xC6, 0x28 },                        // add rsi, 40
      mov_rdi_rsp{ 0x48, 0x89, 0xE7 },                             // mov rdi, rsp
      add_rdi{ 0x48, 0x81, 0xC7 },                                 // add_rdi, {-n * 8}
      rsp_copy_offset{(-(n * 8))},
      mov_rcx_N{ 0x48, 0xC7, 0xC1 },                               // mov rcx, {n}
      N{(n)},
      rep_movsq{ 0xF3, 0x48, 0xA5 },                               // rep movsq

      mov_rsi_r10{ 0x4C, 0x89, 0xD6 },                             // mov rsi, r10
      mov_rdi_r11{ 0x4C, 0x89, 0xDF },                             // mov rdi, r11

      mov_qword_ptr_rsp_r9{ 0x4C, 0x89, 0x8C, 0x24 },              // mov qword ptr[rsp - {8 * (n + 1)}], r9
      spill_offset{(-(8 * (n + 1)))},

      sub_rsp{ 0x48, 0x81, 0xEC },                                 // sub rsp, {32 + 8 + (n * 8)}
      rsp_grow_offset{(32 + 8 + (n * 8)) },

      mov_r9_r8{ 0x4D, 0x89, 0xC1 },                               // mov r9, r8
      mov_r8_rdx{ 0x49, 0x89, 0xD0 },                              // mov r8, rdx
      mov_rdx_rax{ 0x48, 0x89, 0xC2 },                             // mov rdx, rax

      mov_rcx{ 0x48, 0xB9 },                                       // mov rcx, this_ptr
      this_ptr{ instance },

      mov_rax{ 0x48, 0xB8 },                                       // mov rax, function
      function{ function },

      call_rax{ 0xFF, 0xD0 },                                      // call rax
      add_rsp_48{ 0x48, 0x83, 0xC4, 0x30 },                        // add rsp, 48
      ret{ 0xC3 },                                                 // ret
      
      int_3{ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC }// int3 int3 int3 ...
    {}
  };

  // Really don't want bytecode to be mutated by a derived class, hence the getter
  Bytecode<ArgCount>* getBytecode() const
  {
    return bytecode_;
  }

  void flushInstructionCache() const
  {
#ifdef _WIN64
    // Flush instruction cache. May be required on some architectures which
    // don't feature strong cache coherency guarantees, though not on neither
    // x86, x64 nor AMD64.
    FlushInstructionCache(GetCurrentProcess(), bytecode_, sizeof(Bytecode<ArgCount>));
#endif
  }

private:
  Bytecode<ArgCount>* bytecode_;
};

// This incomplete template class is here to provide type deduction
template<typename, typename>
class Thunk;

// This is a specialization of the incomplete class,
// which formats the template arguments as so to extract information from a function pointer type
template<class T, typename Ret, typename... Args>
class Thunk<T, Ret(*)(Args...)> : ThunkBase<sizeof...(Args)>
{
public:
  constexpr static std::size_t ArgC = sizeof...(Args);
  // Type of the callback function, as passed by template argument
  using callback_type = Ret(*)(Args...);
  // Type of the corresponding method in T which has the same signature
  using method_type = Ret(T::*)(Args...);

  // Take a `this` pointer and a method pointer to bind
  Thunk(T* instance, method_type method)
    : ThunkBase{ instance, brute_cast<void*>(method) }
  {
    // ThunkBase constructor modifies executable memory, on some processors this requires an icache flush
    ThunkBase<ArgC>::flushInstructionCache();
  }

  // Returns a callback which can be called like a non-member function
  callback_type getCallback() const
  {
    // bytecode points to executable memory like a function pointer,
    // so we can simply pretend it is a function pointer.
    return reinterpret_cast<callback_type>(ThunkBase<ArgC>::getBytecode());
  }
};

}
