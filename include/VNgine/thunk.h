#pragma once

#include <VNgine/helper.h>
#include <mutex>

#include <GLFW/glfw3.h>

namespace VNgine
{

void g_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

class ThunkBase : non_copyable<ThunkBase>
{
protected:
  ThunkBase(void* instance, void* function);
  ~ThunkBase();

  struct Bytecode
  {
#pragma pack(push, 1)
    std::uint8_t mov_rax[2];                  // store the pointer to member function in RAX
    void* function;                           // it may actually be a pointer to vtable but it will still work

    std::uint8_t push_qword_ptr_rsp40[4];     // copy the first spilled argument to the stack
    std::uint8_t push_r9[2];                  // spill the argument from R9 onto the stack

    std::uint8_t sub_rsp_32[4];               // create the shadow space for the member callback

    std::uint8_t mov_r9_r8[3];                // shift the arguments from [RCX, RDX, R8] to [RDX, R8, R9]
    std::uint8_t mov_r8_rdx[3];               // to make room for the `this` ptr in RCX
    std::uint8_t mov_rdx_rcx[3];              

    std::uint8_t mov_rcx[2];                  // store the `this` ptr in RCX
    void* this_ptr;

    std::uint8_t call_rax[2];                 // execute the member function and allow it to return back here
    std::uint8_t add_rsp_48[4];               // clean up the shadow space and the space for the spilled arguments
    std::uint8_t ret[1];
    //std::uint8_t rex_jmp_rax[3];              // jump to the address in RAX (member function) and start executing

    std::uint8_t int_3[9];                    // just a bunch of interrupts to stop in case something goes wrong
#pragma pack(pop)
    Bytecode(void* instance, void* function) :

    /* Used https://defuse.ca/online-x86-assembler to translate into machine code */

      mov_rax{ 0x48, 0xB8 },                                       // mov rax, function
      function{ function },

      push_qword_ptr_rsp40{ 0xFF, 0x74, 0x24, 0x28 },              // push qword ptr[rsp + 40]
      push_r9{ 0x41, 0x51 },                                       // push r9

      sub_rsp_32{ 0x48, 0x83, 0xEC, 0x20 },                        // sub rsp, 32

      mov_r9_r8{ 0x4D, 0x89, 0xC1 },                               // mov r9, r8
      mov_r8_rdx{ 0x49, 0x89, 0xD0 },                              // mov r8, rdx
      mov_rdx_rcx{ 0x48, 0x89, 0xCA },                             // mov rdx, rcx

      mov_rcx{ 0x48, 0xB9 },                                       // mov rcx, this_ptr
      this_ptr{ instance },

      call_rax{ 0xFF, 0xD0 },                                      // call rax
      add_rsp_48{ 0x48, 0x83, 0xC4, 0x30 },                        // add rsp, 48
      ret{ 0xC3 },                                                 // ret
      //rex_jmp_rax{ 0x48, 0xFF, 0xE0 },                             // rex jmp rax
      
      int_3{ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC }// int3 int3 int3 ...
    {}
  };

  // Really don't want bytecode to be mutated by a derived class, hence the getter
  Bytecode* getBytecode() const;
  void flushInstructionCache() const;

private:
  Bytecode* bytecode_;
  static void* volatile bytecode_heap;
  static std::mutex heap_mutex;

  static void __cdecl cleanupHeap();
};

// This incomplete template class is here to provide type deduction
template<typename, typename>
class Thunk;

// This is a specialization of the incomplete class,
// which formats the template arguments as so to extract information from a function pointer type
template<class T, typename Ret, typename... Args>
class Thunk<T, Ret(*)(Args...)> : ThunkBase
{
public:
  // Type of the callback function, as passed by template argument
  using callback_type = Ret(*)(Args...);
  // Type of the corresponding method in T which has the same signature
  using method_type = Ret(T::*)(Args...);

  // Take a `this` pointer and a method pointer to bind
  Thunk(T* instance, method_type method)
    :  ThunkBase(instance, brute_cast<void*>(method))
  {
    // ThunkBase constructor modifies executable memory, on some processors this requires an icache flush
    flushInstructionCache();
  }

  // Returns a callback which can be called like a non-member function
  callback_type getCallback() const
  {
    // bytecode points to executable memory like a function pointer,
    // so we can simply pretend it is a function pointer.
    return reinterpret_cast<callback_type>(getBytecode());
  }
};

}
