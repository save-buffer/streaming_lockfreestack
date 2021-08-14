#include <cstdint>
#include <immintrin.h>

class LockFreeStack
{
public:
    LockFreeStack() { m_head.head = nullptr; m_head.count = 0; }
    void Push(uint32_t entry);
    uint32_t Pop();
    uint64_t GetOperationCount();
private:
    struct StackEntry
    {
        uint32_t value;
        StackEntry* next;
    };

    union StackHead
    {
        struct
        {
            StackEntry *head;
            uint64_t count;
        };
        __m128i blob;
    };

    bool CAS(StackHead *p, const StackHead &exp_val, const StackHead &new_val);

    __attribute__((aligned(16))) StackHead m_head;
};

inline __attribute__((always_inline))
bool LockFreeStack::CAS(StackHead *p, const StackHead &exp_val, const StackHead &new_val)
{
    __m128i curr = _mm_stream_load_si128(&p->blob);
    __m128i neq = _mm_xor_si128(curr, exp_val.blob);
    if(_mm_test_all_zeros(neq, neq))
    {
        _mm_stream_si128(&p->blob , new_val.blob);
        return true;
    }
    return false;
}

void LockFreeStack::Push(uint32_t entry)
{
    StackEntry *new_entry = new StackEntry;
    new_entry->value = entry;

    StackHead old_head;
    StackHead new_head;
    new_head.head = new_entry;

    do
    {
        old_head.blob = _mm_stream_load_si128(&m_head.blob);
        new_head.count = old_head.count + 1;
        new_entry->next = old_head.head;
    } while(!CAS(&m_head, old_head, new_head));
}

uint32_t LockFreeStack::Pop()
{
    StackEntry *result = nullptr;
    StackHead old_head;
    StackHead new_head;

    do
    {
        old_head.blob = _mm_stream_load_si128(&m_head.blob);
        result = old_head.head;
        if(!result)
            return static_cast<uint32_t>(~0);
        
        new_head.count = old_head.count + 1;
        new_head.head = result->next;
    } while(!CAS(&m_head, old_head, new_head));

    // Intentionally leak memory
    return result->value;
}

uint64_t LockFreeStack::GetOperationCount()
{
    StackHead head;
    head.blob =  _mm_stream_load_si128(&m_head.blob);
    return head.count;
}
