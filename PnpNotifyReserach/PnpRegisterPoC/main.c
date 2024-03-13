#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <initguid.h>
#include <usbiodef.h>
#include <ntddstor.h>

PVOID RegValue;

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	IoUnregisterPlugPlayNotification(RegValue);
	DbgPrint("[*] poc reg unloaded\n");
}

NTSTATUS DriverCallback(PVOID NotificationStructure, PVOID Context)
{
	DbgPrint("[*] called\n");
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	GUID guid = GUID_DEVINTERFACE_DISK;

	IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange, PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES, &guid, DriverObject, DriverCallback, NULL, &RegValue);

	DbgPrint("[*] poc reg loaded\n");
	DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}