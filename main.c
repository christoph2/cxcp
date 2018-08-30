
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "xcp_ow.h"

#include <stdio.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )

#define _CRT_SECURE_NO_WARNINGS


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
      strncpy_s(scnName, IMAGE_SIZEOF_SHORT_NAME, (const char*)pSectionHdr->Name, sizeof(pSectionHdr->Name));

      printf("  Section %3d: %p...%p %-15s (%u bytes)\n",
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

void pagTest(void);

static Xcp_HwMapFileType a2lFile;

static const char FNAME[] = "C:\\Users\\Public\\Documents\\Vector CANape 15\\Examples\\XCPDemo\\XCPsim.a2l";


void XcpOnCan_Init(void);

int main()
{
    pagTest();

    XcpOnCan_Init();

    XcpOw_MapFileOpen(FNAME, &a2lFile);

    //print_PE_section_info(GetModuleHandle(NULL));
    Xcp_Init();
    DBG_PRINT1("Starting XCP task...\n");
    fflush(stdout);

    for (;;) {
        XcpTl_Task();
    }

    XcpTl_DeInit();

    XcpOw_MapFileClose(&a2lFile);
    return 0;
}


bool Xcp_HookFunction_GetId(uint8_t idType)
{
    if (idType == 4) {
        Xcp_SetMta(Xcp_GetNonPagedAddress(a2lFile.view.mappingAddress));
        Xcp_Send8(8, 0xff, 0, 0, 0, LOBYTE(LOWORD(a2lFile.size)), HIBYTE(LOWORD(a2lFile.size)), LOBYTE(HIWORD(a2lFile.size)), HIBYTE(HIWORD(a2lFile.size)));
        return TRUE;
    }
    return FALSE;
}
