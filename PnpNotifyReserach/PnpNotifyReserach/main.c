#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <initguid.h>
#include <usbiodef.h>
#include <ntddstor.h>
#include "PnpReversal.h"

unsigned char PnpDeviceClassNotifyListSig[] = { 0x6B, 0xC2, 0x0D, 0x48, 0x8D }; // 22H2 lea rdx, PnpDeviceClassNotifyList  

PVOID PatternScan(unsigned char* StartAddress, unsigned char* Pattern, int PatternLength, int SearchLimitLength) {
	for (int i = 0; i < SearchLimitLength; i++) {
		if (StartAddress[i] == Pattern[0]) {
			if (memcmp(StartAddress + i, Pattern, PatternLength) == 0) {
				return StartAddress + i;
			}
		}
	}
	return NULL;
}


void PnpEnumerateCallbacks(LPCGUID ClassGuid, PLIST_ENTRY PnpDeviceClassListPtr)
{
	PLIST_ENTRY head = &PnpDeviceClassListPtr[PnpHashGuid(ClassGuid)];
	PLIST_ENTRY current = head;

	do
	{
		PDEVICE_CLASS_CALLBACK_ENTRY Entry = (PDEVICE_CLASS_CALLBACK_ENTRY)current;
		 if(IopCompareGuid(ClassGuid,&Entry->ClassGuid))
			DbgPrint("[*]  registered DeviceInterfaceChange callback  : 0x%p by %wZ\n", Entry->CallbackRoutine, Entry->DriverObject->DriverName);
		current = current->Flink;

	} while (current != head);


}



BOOLEAN PoC()
{
	UNICODE_STRING RegisterPnpName = RTL_CONSTANT_STRING(L"IoRegisterPlugPlayNotification");

	PVOID IoRegisterPnpNotificationAddr = MmGetSystemRoutineAddress(&RegisterPnpName);
	if (!IoRegisterPnpNotificationAddr)
	{
		DbgPrint("[*] failed to resolve IoRegisterPlugPlayNotification\n");
		return FALSE;
	}
	PVOID PatternAddress = PatternScan(IoRegisterPnpNotificationAddr, PnpDeviceClassNotifyListSig, sizeof(PnpDeviceClassNotifyListSig), 0x500);
	if (!PatternAddress || !MmIsAddressValid(PatternAddress))
	{
		DbgPrint("[*] failed to resolve pattern\n");
		return FALSE;
	}
	PVOID InstructionAddress = (ULONG_PTR)PatternAddress + 3; // this now points at lea rdx, PnpDeviceClassNotifyList 
	DbgPrint("[*] lea rdx, PnpDeviceClassNotifyList at 0x%p\n", InstructionAddress);

	DWORD Offset = *(PDWORD)((ULONG_PTR)InstructionAddress + 3);
	PVOID PnpDeviceClassList = (ULONG_PTR)InstructionAddress + Offset + 7;

	if (!PnpDeviceClassList || !MmIsAddressValid(PnpDeviceClassList))
	{
		DbgPrint("[*] failed to resolve PnpDeviceClassList\n");
		return FALSE;
	}
	DbgPrint("[*] PnpDeviceClassList is at 0x%p\n", PnpDeviceClassList);

	// interface class to enumerate callbacks for 
	GUID guid = GUID_DEVINTERFACE_DISK;
	PnpEnumerateCallbacks(&guid, (PLIST_ENTRY)PnpDeviceClassList);

	return TRUE;
}

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	DbgPrint("[*] unloading\n");

}


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint("[*] loading\n");

	PoC();

	DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
