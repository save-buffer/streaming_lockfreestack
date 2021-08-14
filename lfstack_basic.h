#include <cstdint>

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
        __int128 blob;
    };

    __attribute__((aligned(16))) StackHead m_head;
};

void LockFreeStack::Push(uint32_t entry)
{
    StackEntry *new_entry = new StackEntry;
    new_entry->value = entry;

    StackHead old_head;
    StackHead new_head;
    new_head.head = new_entry;

    do
    {
        old_head = *(StackHead * volatile)&m_head;
        new_head.count = old_head.count + 1;
        new_entry->next = old_head.head;
    } while(!__sync_bool_compare_and_swap(&m_head.blob, old_head.blob, new_head.blob));
}

uint32_t LockFreeStack::Pop()
{
    StackEntry *result = nullptr;
    StackHead old_head;
    StackHead new_head;

    do
    {
        old_head = *(StackHead * volatile)&m_head;
        result = old_head.head;
        if(!result)
            return static_cast<uint32_t>(~0);
        
        new_head.count = old_head.count + 1;
        new_head.head = result->next;
    } while(!__sync_bool_compare_and_swap(&m_head.blob, old_head.blob, new_head.blob));

    // Intentionally leak memory
    return result->value;
}

uint64_t LockFreeStack::GetOperationCount()
{
    StackHead head = *(StackHead * volatile)&m_head;
    return head.count;
}
