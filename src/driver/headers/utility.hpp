#pragma once

PLIST_ENTRY PsLoadedModuleList = nullptr;

namespace Utility
{
	PDRIVER_OBJECT GetDriverObject(PCWSTR object_path)
	{

		UNICODE_STRING object_path_unicide;
		RtlInitUnicodeString(&object_path_unicide, object_path);

		PDRIVER_OBJECT object;
		if (NT_SUCCESS(ObReferenceObjectByName(&object_path_unicide, OBJ_CASE_INSENSITIVE, NULL, NULL, *IoDriverObjectType, KernelMode, NULL, reinterpret_cast<PVOID*>(&object))))
		{
			return object;
		}

		return {};
	}

  PVOID GetKernelFunction(PCWSTR FuncName)
  {
    UNICODE_STRING func = { 0 };
    RtlInitUnicodeString(&func, FuncName);

    return MmGetSystemRoutineAddress(&func);
  }

  PKLDR_DATA_TABLE_ENTRY GetKernelModuleByName(PCWSTR ModuleName)
  {
    PKLDR_DATA_TABLE_ENTRY ModuleLdrEntry = nullptr;

    UNICODE_STRING szModule = { 0 };
    RtlInitUnicodeString(&szModule, ModuleName);

    if (!PsLoadedModuleList)
      return ModuleLdrEntry;

    auto LdrEntry = reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(PsLoadedModuleList->Flink);

    while (reinterpret_cast<PLIST_ENTRY>(LdrEntry) != PsLoadedModuleList)
    {

      if (!RtlCompareUnicodeString(&LdrEntry->BaseDllName, &szModule, TRUE))
      {
        ModuleLdrEntry = LdrEntry;
        break;
      }

      LdrEntry = reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(LdrEntry->InLoadOrderLinks.Flink);
    }

    return ModuleLdrEntry;
  }

}
