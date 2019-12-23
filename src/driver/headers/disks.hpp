#pragma once

auto layout_info_ioctl(PDEVICE_OBJECT device, PIRP irp, PVOID context) -> NTSTATUS
{ 
	if (context)
	{
		IOC_REQUEST request = *(PIOC_REQUEST)(context);
		ExFreePool(context);

		if (request.BufferLength >= sizeof(DRIVE_LAYOUT_INFORMATION_EX))
		{
			auto info = reinterpret_cast<PDRIVE_LAYOUT_INFORMATION_EX>(request.Buffer);

			if (PARTITION_STYLE_GPT == info->PartitionStyle)
			{
				memset(&info->Gpt.DiskId, 0, sizeof(GUID));
			}
		}

		if (request.OldRoutine && irp->StackCount > 1)
		{
			return request.OldRoutine(device, irp, request.OldContext);
		}

	}

	return STATUS_SUCCESS;
}

auto partition_info_ioctl(PDEVICE_OBJECT device, PIRP irp, PVOID context) -> NTSTATUS
{
	if (context)
	{
		IOC_REQUEST request = *(PIOC_REQUEST)(context);
		ExFreePool(context);

		if (request.BufferLength >= sizeof(PARTITION_INFORMATION_EX))
		{
			auto info = reinterpret_cast<PPARTITION_INFORMATION_EX>(request.Buffer);

			if (PARTITION_STYLE_GPT == info->PartitionStyle)
			{
				memset(&info->Gpt.PartitionId, 0, sizeof(GUID));
			}
		}

		if (request.OldRoutine && irp->StackCount > 1)
		{
			return request.OldRoutine(device, irp, request.OldContext);
		}
	}

	return STATUS_SUCCESS;
}