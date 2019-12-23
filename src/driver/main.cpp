#include "headers/includes.hpp"
#include "headers/structs.hpp"
#include "headers/utility.hpp"

#include <algorithm>

PDRIVER_DISPATCH PartmgrOriginal;

auto GetModuleBase(PCWSTR Name) -> UINT_PTR
{
  auto module = Utility::GetKernelModuleByName(Name);

  if (!module)
  {
    DbgPrint("Failed to find %ws in LinkedList \n", Name);
    return 0;
  }

  auto base = reinterpret_cast<UINT_PTR>(module->DllBase);

  if (!module)
  {
    DbgPrint("Failed to get %ws base address \n", Name);
    return 0;
  }

  return base;
}

NTSTATUS device_control(PDEVICE_OBJECT device, PIRP irp)
{
	return PartmgrOriginal(device, irp);
}

const unsigned char jmp_buffer[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };                                            // jmp rax

NTSTATUS Spoofer()
{
   auto partmgr_base = GetModuleBase(L"partmgr.sys");

   if (!partmgr_base)
   {
     return STATUS_UNSUCCESSFUL;
   }

   auto partmgr = RtlImageNtHeader(reinterpret_cast<PVOID>( partmgr_base ));

   const auto entry = reinterpret_cast<void*>( ( partmgr_base + partmgr->OptionalHeader.AddressOfEntryPoint ) - 0x1000 );

   auto physical_entry = MmGetPhysicalAddress(entry);

   if (!physical_entry.QuadPart)
   {
	   DbgPrint("MmGetPhysicalAddress Failed\n");
	   return STATUS_UNSUCCESSFUL;
   }

   auto mapped_mem = MmMapIoSpaceEx(physical_entry, PAGE_SIZE, PAGE_READWRITE);

   if (!mapped_mem)
   {
	   DbgPrint("MmMapIoSpaceEx Failed\n");
	   return STATUS_UNSUCCESSFUL;
   }

   memcpy(mapped_mem, jmp_buffer, sizeof(jmp_buffer));

   *reinterpret_cast<void**>(&reinterpret_cast<unsigned char*>(mapped_mem)[2]) = &device_control;

   MmUnmapIoSpace(mapped_mem, PAGE_SIZE);

   auto partmgr_object = Utility::GetDriverObject(L"\\Driver\\partmgr");

   if (!partmgr_object)
   {
	   DbgPrint("partmgr_object Failed\n");
	   return STATUS_UNSUCCESSFUL;
   }

   PartmgrOriginal = partmgr_object->MajorFunction[IRP_MJ_DEVICE_CONTROL];

   partmgr_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = reinterpret_cast<PDRIVER_DISPATCH>(entry);

   DbgPrint("Success! %p \n", entry);

  return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(DRIVER_OBJECT* driver_object, UNICODE_STRING* registry_path)
{
  UNREFERENCED_PARAMETER(registry_path);
  UNREFERENCED_PARAMETER(driver_object);

  PsLoadedModuleList = reinterpret_cast<PLIST_ENTRY>(Utility::GetKernelFunction(L"PsLoadedModuleList"));

  if (!PsLoadedModuleList)
  {
    DbgPrint("Failed to get Kernel Function: PsLoadedModuleList \n");
    return STATUS_UNSUCCESSFUL;
  }

  auto status = Spoofer();

  return status;
}
