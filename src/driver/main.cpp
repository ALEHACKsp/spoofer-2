#include "headers/includes.hpp"
#include "headers/structs.hpp"
#include "headers/utility.hpp"
#include "headers/disks.hpp"

#include <algorithm>
#include <string.h>

PDRIVER_DISPATCH PartmgrOriginal;

auto get_module_base(PCWSTR Name) -> UINT_PTR
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

void spoof_ioctl(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine)
{
	auto request = (PIOC_REQUEST)ExAllocatePool(NonPagedPool, sizeof(IOC_REQUEST));

	request->Buffer = irp->AssociatedIrp.SystemBuffer;
	request->BufferLength = ioc->Parameters.DeviceIoControl.OutputBufferLength;
	request->OldContext = ioc->Context;
	request->OldRoutine = ioc->CompletionRoutine;

	ioc->Control = SL_INVOKE_ON_SUCCESS;
	ioc->Context = request;
	ioc->CompletionRoutine = routine;
}

auto is_from_valid_module(const char* module_name) -> bool
{
	if (strstr("csgo", module_name) || strstr("esportal", module_name) || strstr("Gamers Cl", module_name) )
	{
		DbgPrint("Process %s called Partmgr IOCTL \n", module_name);
		return true;
	}

	return false;
}

auto partmgr_control(PDEVICE_OBJECT device, PIRP irp) -> NTSTATUS
{
	auto ioc = IoGetCurrentIrpStackLocation(irp);

	auto process = IoGetCurrentProcess();

	auto file_name = (const char*)( PsGetProcessImageFileName(process) );

	if (!is_from_valid_module(file_name))
	{
		return PartmgrOriginal(device, irp);
	}

	switch (ioc->Parameters.DeviceIoControl.IoControlCode)
	{

	case IOCTL_DISK_GET_PARTITION_INFO_EX:
		spoof_ioctl(ioc, irp, partition_info_ioctl);
		break;

	case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:
		spoof_ioctl(ioc, irp, layout_info_ioctl);
		break;

	}

	return PartmgrOriginal(device, irp);
}

const unsigned char jmp_buffer[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };                                            // jmp rax

NTSTATUS spoof_partmgr()
{
   auto partmgr_base = get_module_base(L"partmgr.sys");

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

   *reinterpret_cast<void**>(&reinterpret_cast<unsigned char*>(mapped_mem)[2]) = &partmgr_control;

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

  
  spoof_partmgr();

  return STATUS_SUCCESS;
}
