#include "Mock12.h"
#include <assert.h>

const GUID Mock12Resource::guid =
{ 0xe4d1943f, 0xe48, 0x4b63, { 0x80, 0x17, 0xe9, 0xf8, 0xfc, 0x65, 0xc, 0x8c } };

const GUID Mock12CommandList::guid =
{ 0x1b12b3dc, 0x3387, 0x46a3, { 0x94, 0x57, 0x85, 0x36, 0xbe, 0x62, 0x89, 0xb4 } };

void Mock12Test()
{
    {
        Mock12Resource* a = new Mock12Resource(nullptr);
        assert(a->m_dwRef == 1);
        Mock12Resource* b = new Mock12Resource(a);
        assert(a->m_dwRef == 2);
        assert(b->m_dwRef == 1);
        b->Release();
        assert(a->m_dwRef == 1);
        a->Release();
    }
}