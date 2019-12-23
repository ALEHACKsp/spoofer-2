#pragma once

extern "C" __declspec(dllimport) PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID Base);
extern "C" __declspec(dllimport) POBJECT_TYPE * IoDriverObjectType;
extern "C" __declspec(dllimport) NTSTATUS NTAPI ObReferenceObjectByName(PUNICODE_STRING ObjectName, ULONG Attributes, PACCESS_STATE AccessState, ACCESS_MASK DesiredAccess, POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode, PVOID ParseContext OPTIONAL, PVOID * Object);

typedef struct _KLDR_DATA_TABLE_ENTRY
{
  LIST_ENTRY		 InLoadOrderLinks;
  void* ExceptionTable;
  unsigned int	 ExceptionTableSize;
  void* GpValue;
  void* NonPagedDebugInfo;
  void* DllBase;
  void* EntryPoint;
  unsigned int	 SizeOfImage;
  UNICODE_STRING	 FullDllName;
  UNICODE_STRING	 BaseDllName;
  unsigned int	 Flags;
  unsigned __int16 LoadCount;
  unsigned __int16 u1;
  void* SectionPointer;
  unsigned int	 CheckSum;
  unsigned int	 CoverageSectionSize;
  void* CoverageSection;
  void* LoadedImports;
  void* Spare;
  unsigned int	 SizeOfImageNotRounded;
  unsigned int	 TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
