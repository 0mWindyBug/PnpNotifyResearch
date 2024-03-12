#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

// PnpDeviceClassList is actually an array of lists / hash table to be precise 
// Callbacks registred for a particular interface class GUID are stored in an index based on a hash calculated from the GUID (hash table ish) 
// Each entry in the list is of type 'DEVICE_CLASS_CALLBACK_ENTRY' (see definition below) 

#define DEVICE_CLASS_HASH_BUCKETS 13

inline ULONG PnpHashGuid(LPCGUID Guid)
{
    return (((PULONG)Guid)[0] + ((PULONG)Guid)[1] + ((PULONG)Guid)[2]
        + ((PULONG)Guid)[3]) % DEVICE_CLASS_HASH_BUCKETS;
}
inline BOOLEAN IopCompareGuid(IN LPCGUID guid1, IN LPCGUID guid2)
{
    return guid1 == guid2 || RtlCompareMemory(guid1, guid2, sizeof(GUID)) == sizeof(GUID);
}

typedef struct _DEVICE_CLASS_CALLBACK_ENTRY {
    LIST_ENTRY ListEntry;
    IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
    ULONG SessionId;
    HANDLE SessionHandle;
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Context;
    PDRIVER_OBJECT DriverObject;
    USHORT RefCount;
    BOOLEAN Unregistered;
    KGUARDED_MUTEX Lock;
    PERESOURCE EntryLock;
    GUID ClassGuid;  
} DEVICE_CLASS_CALLBACK_ENTRY, * PDEVICE_CLASS_CALLBACK_ENTRY;


