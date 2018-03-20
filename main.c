
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp.h"

#include <stdio.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )


// https://stackoverflow.com/questions/4308996/finding-the-address-range-of-the-data-segment
void print_PE_section_info(HANDLE hModule) // hModule is the handle to a loaded Module (.exe or .dll)
{
   // get the location of the module's IMAGE_NT_HEADERS structure
   IMAGE_NT_HEADERS *pNtHdr = ImageNtHeader(hModule);

   // section table immediately follows the IMAGE_NT_HEADERS
   IMAGE_SECTION_HEADER *pSectionHdr = (IMAGE_SECTION_HEADER *)(pNtHdr + 1);

   const char* imageBase = (const char*)hModule;
   char scnName[sizeof(pSectionHdr->Name) + 1];
   scnName[sizeof(scnName) - 1] = '\0'; // enforce nul-termination for scn names that are the whole length of pSectionHdr->Name[]

   for (int scn = 0; scn < pNtHdr->FileHeader.NumberOfSections; ++scn)
   {
      // Note: pSectionHdr->Name[] is 8 bytes long. If the scn name is 8 bytes long, ->Name[] will
      // not be nul-terminated. For this reason, copy it to a local buffer that's nul-terminated
      // to be sure we only print the real scn name, and no extra garbage beyond it.
      strncpy(scnName, (const char*)pSectionHdr->Name, sizeof(pSectionHdr->Name));

      printf("  Section %3d: %p...%p %-10s (%u bytes)\n",
         scn,
         imageBase + pSectionHdr->VirtualAddress,
         imageBase + pSectionHdr->VirtualAddress + pSectionHdr->Misc.VirtualSize - 1,
         scnName,
         pSectionHdr->Misc.VirtualSize);
      ++pSectionHdr;
   }
}

// For demo purpopses, create an extra constant data section whose name is exactly 8 bytes long (the max)
#pragma const_seg(".t_const") // begin allocating const data in a new section whose name is 8 bytes long (the max)
const char const_string1[] = "This string is allocated in a special const data segment named \".t_const\".";
#pragma const_seg() // resume allocating const data in the normal .rdata section



#pragma bss_seg(push, /*stack1,*/ ".arbeitsseite")
int in_der_arbeitsseite;

#pragma bss_seg(pop/*, stack1*/)
int bss_normal;

void daqTest(void);
void pagTest(void);

int main()
{
    daqTest();
    pagTest();

    print_PE_section_info(GetModuleHandle(NULL));
    Xcp_Init();

    //DBG_PRINT("Starting XCP task...\n");
    DBG_PRINT1("Starting XCP task...\n");
    fflush(stdout);
//#if 0
    for (;;) {
        XcpTl_Task();
    }
//#endif
    XcpTl_DeInit();

    return 0;
}

