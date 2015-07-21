//====================================================
//
//  Callback File System
//
//  Copyright (c) 2006-2009, Softpanorama++ LLC
//  Exclusive rights granted to EldoS Corporation
//
//====================================================

#ifndef _CALLBACKFS_USER_LIB_
#define _CALLBACKFS_USER_LIB_

#pragma pack (8) 

#ifndef __BORLANDC__
#pragma comment(lib, "Advapi32")
#pragma comment(lib, "Version")
#endif
#define MAX_SHORT_NAME_LENGTH 12
//
// Maximum length of the file system in WCHAR
//
#define MAX_FILE_SYSTEM_NAME_LENGTH    10

static LPCWSTR errCbFsStorageNotActive          = L"Storage is Not Active";
static LPCWSTR errCbFsStorageIsActive           = L"Storage is Active";
static LPCWSTR errCbFsInvalidMountingPointIndex = L"Invalid Index Value for Mounting Point";
static LPCWSTR errCbFsInvalidSectorSize         = L"Invalid sector size";
static LPCWSTR errCbFsUnknown                   = L"Unknown error";

static LPCWSTR errCbFsInvalidKey        = L"Not valid license key";
static LPCWSTR errCbFsExpired           = L"Your license key is expired";
static LPCWSTR errCbFsProductID         = L"License key is used with not proper product";
static LPCWSTR errCbFsInvalidData       = L"License key missing some information";
static LPCWSTR errCbFsNotInitialized    = L"Initialization error. Invoke CallbackFileSystem::Initialize() method first";

class CallbackFileSystem;
class CbFsDirectoryEnumerationInfo;
class CbFsNamedStreamsEnumerationInfo;
class CbFsFileInfo;
class CbFsHandleInfo;
class CbFsOpenedFilesSnapshot;

//extern static CbFsHandleContext::CbFsHandleContext* CcbContext;

typedef void (*CbFsMountEvent)(CallbackFileSystem* Sender);

typedef void (*CbFsUnmountEvent)(CallbackFileSystem* Sender);

typedef void (*CbFsGetVolumeSizeEvent)(
    CallbackFileSystem* Sender,
    __int64* TotalAllocationUnits,
    __int64*AvailableAllocationUnits
    );

typedef void (*CbFsGetVolumeLabelEvent)(CallbackFileSystem* Sender, LPWSTR VolumeLabel);

typedef void (*CbFsSetVolumeLabelEvent)(CallbackFileSystem* Sender, LPCWSTR VolumeLabel);

typedef void (*CbFsGetVolumeIdEvent)(CallbackFileSystem* Sender, PDWORD VolumeID);

typedef void (*CbFsCreateFileEvent)(
    CallbackFileSystem* Sender,
    LPCWSTR FileName,
    ACCESS_MASK DesiredAccess,
    DWORD FileAttributes,
    DWORD ShareMode,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo
    );

typedef void (*CbFsOpenFileEvent)(
    CallbackFileSystem* Sender,
    LPCWSTR FileName,
    ACCESS_MASK DesiredAccess,
    DWORD FileAttributes,
    DWORD ShareMode,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo
    );

typedef void (*CbFsCleanupFileEvent)(CallbackFileSystem* Sender, CbFsFileInfo* FileInfo, CbFsHandleInfo* HandleInfo);

typedef void (*CbFsCloseFileEvent)(CallbackFileSystem* Sender, CbFsFileInfo* FileInfo, CbFsHandleInfo* HandleInfo);

typedef void (*CbFsGetFileInfoEvent)(
    CallbackFileSystem* Sender,
    LPCWSTR FileName,
    LPBOOL FileExists,
    PFILETIME CreationTime,
    PFILETIME LastAccessTime,
    PFILETIME LastWriteTime,
    __int64* EndOfFile,
    __int64* AllocationSize,
    __int64* FileId,
    PDWORD FileAttributes,
    LPWSTR ShortFileName OPTIONAL,
    PWORD ShortFileNameLength OPTIONAL,
    LPWSTR RealFileName OPTIONAL,
    LPWORD RealFileNameLength OPTIONAL
    );

typedef void (*CbFsEnumerateDirectoryEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* DirectoryInfo,
    CbFsHandleInfo* HandleInfo,
    CbFsDirectoryEnumerationInfo* DirectoryEnumerationInfo,
    LPCWSTR Mask,
    INT Index,
    BOOL Restart,
    LPBOOL FileFound,
    LPWSTR FileName,
    PDWORD FileNameLength,
      LPWSTR ShortFileName,
    PUCHAR ShortFileNameLength,
    PFILETIME CreationTime,
    PFILETIME LastAccessTime,
    PFILETIME LastWriteTime,
    __int64* EndOfFile,
    __int64* AllocationSize,
    __int64* FileId,
    PDWORD FileAttributes
    );

typedef void (*CbFsCloseDirectoryEnumerationEvent)(CallbackFileSystem* Sender, CbFsFileInfo* DirectoryInfo, CbFsDirectoryEnumerationInfo* EnumerationInfo);

typedef void (*CbFsCloseNamedStreamsEnumerationEvent)(CallbackFileSystem* Sender, CbFsFileInfo* FileInfo, CbFsNamedStreamsEnumerationInfo* EnumerationInfo);

typedef void (*CbFsSetAllocationSizeEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    __int64 AllocationSize
    );

typedef void (*CbFsSetEndOfFileEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    __int64 EndOfFile
    );

typedef void (*CbFsSetFileAttributesEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo,
    PFILETIME CreationTime,
    PFILETIME LastAccessTime,
    PFILETIME LastWriteTime,
    DWORD FileAttributes
    );

typedef void (*CbFsCanFileBeDeletedEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo,
    LPBOOL CanBeDeleted
    );

typedef void (*CbFsDeleteFileEvent)(CallbackFileSystem* Sender, CbFsFileInfo* FileInfo);

typedef void (*CbFsRenameOrMoveFileEvent)(CallbackFileSystem* Sender, CbFsFileInfo* FileInfo, LPCWSTR NewFileName);

typedef void (*CbFsReadFileEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    __int64 Position,
    PVOID Buffer, 
    DWORD BytesToRead,
    PDWORD BytesRead
    );

typedef void (*CbFsWriteFileEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    __int64 Position,
    PVOID Buffer, 
    DWORD BytesToWrite,
    PDWORD BytesWritten
    );

typedef void (*CbFsIsDirectoryEmptyEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* DirectoryInfo,
    LPCWSTR FileName,
    LPBOOL IsEmpty
    );

typedef void (*CbFsEnumerateNamedStreamsEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo,
    CbFsNamedStreamsEnumerationInfo* NamedStreamsEnumerationInfo,
    PWSTR StreamName,
    PDWORD StreamNameLength,
    __int64* StreamSize,
    __int64* StreamAllocationSize,
    LPBOOL NamedStreamFound
    );

typedef void (*CbFsSetFileSecurityEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD Length
    );

typedef void (*CbFsGetFileSecurityEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    CbFsHandleInfo* HandleInfo,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD Length,
    PDWORD LengthNeeded
    );

typedef void (*CbFsGetFileNameByFileIdEvent)(
    CallbackFileSystem* Sender,
    __int64 FileId,
    PWSTR FileName,
    PDWORD FileNameLength
    );

typedef void (*CbFsFlushFileEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo
  );

typedef void (*CbFsStorageEjectedEvent)(
    CallbackFileSystem* Sender
    );

typedef void (*CbFsSetValidDataLengthEvent)(
    CallbackFileSystem* Sender,
    CbFsFileInfo* FileInfo,
    __int64 ValidDataLength
    );

//ECBFSError

class ECBFSError
{
public:
    ECBFSError(LPCWSTR Message);
    ECBFSError(DWORD ErrorCode);
    ECBFSError(LPCWSTR Message, DWORD ErrorCode);
    ECBFSError(const ECBFSError&);
    ~ECBFSError();
    
    LPCWSTR Message();
    DWORD ErrorCode();

private: 
    LPCWSTR mMessage;
    DWORD mErrorCode;
    WCHAR mhLocal[1024];
    void ECBFSFormatMessage(void);
};

//CallbackFieSystem
#define CBFS_MODULE_DRIVER                 0x00000002
#define CBFS_MODULE_NET_REDIRECTOR_DLL     0x00010000
#define CBFS_MODULE_MOUNT_NOTIFIER_DLL     0x00020000

#define CBFS_SYMLINK_SIMPLE                        0x00010000
#define CBFS_SYMLINK_MOUNT_MANAGER                 0x00020000
#define CBFS_SYMLINK_NETWORK                       0x00040000

#define CBFS_SYMLINK_LOCAL                         0x10000000

#define CBFS_SYMLINK_NETWORK_ALLOW_MAP_AS_DRIVE    0x00000001
#define CBFS_SYMLINK_NETWORK_HIDDEN_SHARE          0x00000002
#define CBFS_SYMLINK_NETWORK_READ_NETWORK_ACCESS   0x00000004
#define CBFS_SYMLINK_NETWORK_WRITE_NETWORK_ACCESS  0x00000008

class CallbackFileSystem
{
public:
    static GUID EmptyGuid;

    enum CbFsStorageCharacteristics
    {
        scFloppyDiskette        = 0x00000001,
        scReadOnlyDevice        = 0x00000002,
        scWriteOnceMedia        = 0x00000008,
        scRemovableMedia        = 0x00000010,
        scAutoCreateDriveLetter = 0x00002000,
        scShowInEjectionTray    = 0x00004000,
        scAllowEjection         = 0x00008000

    };
    
    enum CbFsStorageType
    {
        stDisk,
        stCDROM,
        stVirtualDisk,
        stDiskPnP
    };

    enum CbFsNotifyFileAction
    {
        fanAdded            = 0x00000001,
        fanRemoved          = 0x00000002,
        fanModified         = 0x00000003,
        fanMetaDataModified = 0x00000004
    };

    enum CbFsDesiredAccess
    {
      paRead       = 0x00000001,
      paWrite      = 0x00000002,
      paReadWrite  = 0x00000003
    };

    enum CbFsNetworkSymLinkFlags
    {
        nsmAllowMapAsDrive      = 0x0001,
        nsmHiddenShare          = 0x0002,
        nsmReadNetworkAccess    = 0x0004,
        nsmWriteNetworkAccess   = 0x0008
    };
public:    
    
    CallbackFileSystem();
    ~CallbackFileSystem();
    void CheckActive(void);
    void CheckInactive(void);

    static void Initialize(LPCSTR ProductName);

    void SetRegistrationKey(LPCSTR Key);

    static void Install(
        LPCWSTR CabFileName,
        LPCSTR ProductName,
        LPCWSTR PathToInstall,
        BOOL SupportPnP,
        DWORD ModulesToInstall,
        LPDWORD RebootRequired
);
    static void Uninstall(
        LPCWSTR CabFileName,
        LPCSTR ProductName,
        LPCWSTR InstalledPath OPTIONAL,
        LPDWORD RebootRequired
);

        static void GetModuleStatus(
        LPCSTR ProductName,
        DWORD Module,
        LPBOOL Installed,
        LPINT FileVersionHigh,
        LPINT FileVersionLow,
        LPSERVICE_STATUS ServiceStatus
        );

    static void InstallIcon(LPCWSTR IconPath, LPCWSTR IconId, LPBOOL RebootNeeded);
    static void UninstallIcon(LPCWSTR IconId, LPBOOL RebootNeeded);

    void SetIcon(LPCWSTR IconId);
    void ResetIcon();

    BOOL IconInstalled(LPCWSTR IconId);

    void CreateStorage(void);
 
    void DeleteStorage(BOOL ForceUnmount = FALSE);
    
    void MountMedia(INT Timeout);
    
    void UnmountMedia(BOOL ForceUnmount = FALSE);
    static BOOL UnmountMedia(LPCWSTR VolumePath, BOOL ForceUnmount = FALSE);

    static BOOL IsCBFSVolume(LPCWSTR VolumePath);

    void AddMountingPoint(LPCWSTR MountingPoint);
    void AddMountingPoint(LPCWSTR MountingPoint, DWORD Flags, PLUID AuthenticationId);    

    INT GetMountingPointCount(void);
    void GetMountingPoint(INT Index, LPWSTR* MountingPoint, DWORD* Flags, PLUID AuthenticationId);

    void DeleteMountingPoint(INT Index);
    void DeleteMountingPoint(LPCWSTR MountingPoint, DWORD Flags, PLUID AuthenticationId);

    static bool ShutdownSystem(DWORD Timeout, BOOL ForceAppsClosed, BOOL RebootAfterShutdown);

    BOOL GetOriginatorProcessName(LPWSTR ProcessName, LPDWORD ProcessNameLength);

    BOOL GetOriginatorProcessId(LPDWORD ProcessId);

    HANDLE GetOriginatorToken(void);

    BOOL ReleaseUnusedFiles(void);

    BOOL ResetTimeout(DWORD Timeout);

    BOOL NotifyDirectoryChange(PCWSTR FileName, CbFsNotifyFileAction Action, BOOL Wait);

    void SetFileSystemName(LPCWSTR FileSystemName);

    // Process restriction support

    void AddGrantedProcess(LPCWSTR ProcessName, DWORD ProcessId, BOOL IncludeChildren, CbFsDesiredAccess DesiredAccess);

    void AddDeniedProcess(LPCWSTR ProcessName, DWORD ProcessId, BOOL IncludeChildren, CbFsDesiredAccess DesiredAccess);

    void DeleteGrantedProcess(LPCWSTR ProcessName, DWORD ProcessId);

    void DeleteDeniedProcess(LPCWSTR ProcessName, DWORD ProcessId);

    INT GetAccessGrantedProcessCount(void);

    INT GetAccessDeniedProcessCount(void);

    void GetAccessGrantedProcess(DWORD Index, LPWSTR* ProcessName, PDWORD ProcessId, LPBOOL IncludeChildren, CbFsDesiredAccess *DesiredAccess);

    void GetAccessDeniedProcess(DWORD Index, LPWSTR* ProcessName, PDWORD ProcessId, LPBOOL IncludeChildren, CbFsDesiredAccess *DesiredAccess);

    // Opened files enumeration support routines

    CbFsOpenedFilesSnapshot* GetOpenedFilesSnapshot(void);

    // compare file name with mask support routine

    static BOOL IsMatchMask(LPCWSTR Name, LPCWSTR Mask, BOOL CaseSensitive);
   
    //properties
    CbFsStorageEjectedEvent                 GetOnStorageEjected(void);
    void                                    SetOnStorageEjected(CbFsStorageEjectedEvent Value);

    CbFsMountEvent                          GetOnMount(void);
    void                                    SetOnMount(CbFsMountEvent Value);

    CbFsUnmountEvent                        GetOnUnmount(void);
    void                                    SetOnUnmount(CbFsUnmountEvent Value);

    CbFsGetVolumeSizeEvent                  GetOnGetVolumeSize(void);
    void                                    SetOnGetVolumeSize(CbFsGetVolumeSizeEvent Value);

    CbFsGetVolumeLabelEvent                 GetOnGetVolumeLabel(void);
    void                                    SetOnGetVolumeLabel(CbFsGetVolumeLabelEvent Value);

    CbFsSetVolumeLabelEvent                 GetOnSetVolumeLabel(void);
    void                                    SetOnSetVolumeLabel(CbFsSetVolumeLabelEvent Value);

    CbFsGetVolumeIdEvent                    GetOnGetVolumeId(void);
    void                                    SetOnGetVolumeId(CbFsGetVolumeIdEvent Value);

    CbFsCreateFileEvent                     GetOnCreateFile(void);
    void                                    SetOnCreateFile(CbFsCreateFileEvent Value);

    CbFsOpenFileEvent                       GetOnOpenFile(void);
    void                                    SetOnOpenFile(CbFsOpenFileEvent Value);

    CbFsCleanupFileEvent                    GetOnCleanupFile(void);
    void                                    SetOnCleanupFile(CbFsCleanupFileEvent Value);

    CbFsCloseFileEvent                      GetOnCloseFile(void);
    void                                    SetOnCloseFile(CbFsCloseFileEvent Value);

    CbFsGetFileInfoEvent                    GetOnGetFileInfo(void);
    void                                    SetOnGetFileInfo(CbFsGetFileInfoEvent Value);

    CbFsEnumerateDirectoryEvent             GetOnEnumerateDirectory(void);
    void                                    SetOnEnumerateDirectory(CbFsEnumerateDirectoryEvent Value);

    CbFsCloseDirectoryEnumerationEvent      GetOnCloseDirectoryEnumeration(void);
    void                                    SetOnCloseDirectoryEnumeration(CbFsCloseDirectoryEnumerationEvent Value);

    CbFsCloseNamedStreamsEnumerationEvent   GetOnCloseNamedStreamsEnumeration(void);
    void                                    SetOnCloseNamedStreamsEnumeration(CbFsCloseNamedStreamsEnumerationEvent Value);

    CbFsSetAllocationSizeEvent              GetOnSetAllocationSize(void);
    void                                    SetOnSetAllocationSize(CbFsSetAllocationSizeEvent Value);

    CbFsSetEndOfFileEvent                   GetOnSetEndOfFile(void);
    void                                    SetOnSetEndOfFile(CbFsSetEndOfFileEvent Value);

    CbFsSetFileAttributesEvent              GetOnSetFileAttributes(void);
    void                                    SetOnSetFileAttributes(CbFsSetFileAttributesEvent Value);

    CbFsCanFileBeDeletedEvent               GetOnCanFileBeDeleted(void);
    void                                    SetOnCanFileBeDeleted(CbFsCanFileBeDeletedEvent Value);

    CbFsDeleteFileEvent                     GetOnDeleteFile(void);
    void                                    SetOnDeleteFile(CbFsDeleteFileEvent Value);

    CbFsRenameOrMoveFileEvent               GetOnRenameOrMoveFile(void);
    void                                    SetOnRenameOrMoveFile(CbFsRenameOrMoveFileEvent Value);

    CbFsReadFileEvent                       GetOnReadFile(void);
    void                                    SetOnReadFile(CbFsReadFileEvent Value);

    CbFsWriteFileEvent                      GetOnWriteFile(void);
    void                                    SetOnWriteFile(CbFsWriteFileEvent Value);

    CbFsIsDirectoryEmptyEvent               GetOnIsDirectoryEmpty(void);
    void                                    SetOnIsDirectoryEmpty(CbFsIsDirectoryEmptyEvent Value);

    CbFsEnumerateNamedStreamsEvent          GetOnEnumerateNamedStreams(void);
    void                                    SetOnEnumerateNamedStreams(CbFsEnumerateNamedStreamsEvent Value);

    CbFsSetFileSecurityEvent                GetOnSetFileSecurity(void);
    void                                    SetOnSetFileSecurity(CbFsSetFileSecurityEvent Value);

    CbFsGetFileSecurityEvent                GetOnGetFileSecurity(void);
    void                                    SetOnGetFileSecurity(CbFsGetFileSecurityEvent Value);                            

    CbFsGetFileNameByFileIdEvent            GetOnGetFileNameByFileId(void);
    void                                    SetOnGetFileNameByFileId(CbFsGetFileNameByFileIdEvent Value);

    CbFsFlushFileEvent                      GetOnFlushFile(void);
    void                                    SetOnFlushFile(CbFsFlushFileEvent Value);

    CbFsSetValidDataLengthEvent             GetOnSetValidDataLength(void);
    void                                    SetOnSetValidDataLength(CbFsSetValidDataLengthEvent Value);

    CbFsStorageCharacteristics              GetStorageCharacteristics(void);    
    void                                    SetStorageCharacteristics(CbFsStorageCharacteristics Value);
    
    CbFsStorageType                         GetStorageType(void);
    void                                    SetStorageType(CbFsStorageType Value);

    void                                    SetMinWorkerThreadCount(DWORD Value);
    void                                    SetMaxWorkerThreadCount(DWORD Value);

    BOOL                                    GetProcessRestrictionsEnabled(void);
    void                                    SetProcessRestrictionsEnabled(BOOL RestrictionEnable);

    void SetVCB(PVOID Value);

    BOOL Active(void);
    BOOL StoragePresent(void);

     void SetSerializeCallbacks(BOOL Value);
     BOOL GetSerializeCallbacks(void);

    void SetCallAllOpenCloseCallbacks(BOOL Value);
    BOOL GetCallAllOpenCloseCallbacks(void);

    void SetMaxFileNameLength(DWORD Value);
    DWORD GetMaxFileNameLength(void);

    void SetMaxFilePathLength(DWORD Value);
    DWORD GetMaxFilePathLength(void);

    void SetSectorSize(WORD Value);
    WORD GetSectorSize(void);

    void SetClusterSize(WORD Value);
    WORD GetClusterSize(void);

    void SetMaxReadWriteBlockSize(DWORD Value);
    DWORD GetMaxReadWriteBlockSize(void);

    void SetShortFileNameSupport(BOOL Value);
    BOOL GetShortFileNameSupport(void);

    BOOL GetUseFileCreationFlags(void);
    void SetUseFileCreationFlags(BOOL Value);

    void* GetTag(void);
    void SetTag(void* Value);
    
    void SetParalleledProcessingAllowed(BOOL Value);
    BOOL GetParalleledProcessingAllowed(void);

    void SetCaseSensitiveFileNames(BOOL Value);
    BOOL GetCaseSensitiveFileNames(void);

    void SetMetaDataCacheEnabled(BOOL Enable);
    BOOL GetMetaDataCacheEnabled(void);

    void SetFileCacheEnabled(BOOL Enable);
    BOOL GetFileCacheEnabled(void);
    
    void SetNonexistentFilesCacheEnabled(BOOL Enable);
    BOOL GetNonexistentFilesCacheEnabled(void);

    void SetStorageGuid(GUID* Guid);
    void SetStorageGuid(LPCWSTR Guid);

    void SetMaxFileSize(__int64 Value);
    __int64 GetMaxFileSize(void);

    //File data cache policy properties
    void SetCacheSize(__int64 Value);
    // Clear data from the cache when a file is closed. Not set by default.
    void SetCachePolicyPurgeOnClose(BOOL Enable);
    BOOL GetCachePolicyPurgeOnClose(void);
    //Write is done synchronously both to the cache and to the OnWrite callback.
    // Not set by default.
    void SetCachePolicyWriteThrough(BOOL Enable);
    BOOL GetCachePolicyWriteThrough(void);

private:
    
    GUID                            mStorageGuid;

    DWORD                           mStorageID;

    CbFsStorageType                 mStorageType;

    CbFsStorageCharacteristics      mStorageCharacteristics;

    LPSTR                           mLicenseKey;

    BOOL                            mSerializeCallbacks;
    
    BOOL                            mShortFileNameSupport;

    DWORD                           mMinWorkerThreadCount;

    DWORD                           mMaxWorkerThreadCount;

    PVOID                           mVcb;

    BOOL                            mCallAllOpenCloseCallbacks;

    BOOL                            mProcessRestrictions;

    DWORD                           mMaxFileNameLength;
    
    DWORD                           mMaxFilePathLength;

    WORD                            mSectorSize;

    WORD                            mClusterSize;

    DWORD                            mMaxReadWriteBlockSize;
        
    void*                            mTag;

    HANDLE                          mStorageDestroyedEvent;

    BOOL                            mUseFileCreationFlags;

    BOOL                            mAllowParallelProcessing;

    BOOL                            mCaseSensitiveFileNames;

    BOOL                            mMetaDataCacheEnabled;

    BOOL                            mFileCacheEnabled;

    BOOL                            mNonexistentFilesCacheEnabled;

    __int64                         mMaxFileSize;

    static BOOL                        mInitialized;

    WCHAR                          mFileSystemName[MAX_FILE_SYSTEM_NAME_LENGTH];
    
    UCHAR                          mFileSystemNameLength; 

    CbFsMountEvent                          mOnMount;
    CbFsUnmountEvent                        mOnUnmount;
    CbFsGetVolumeSizeEvent                  mOnGetVolumeSize;
    CbFsGetVolumeLabelEvent                 mOnGetVolumeLabel;
    CbFsSetVolumeLabelEvent                 mOnSetVolumeLabel;
    CbFsGetVolumeIdEvent                    mOnGetVolumeId;
    CbFsCreateFileEvent                     mOnCreateFile;
    CbFsOpenFileEvent                       mOnOpenFile;
    CbFsCleanupFileEvent                    mOnCleanupFile;
    CbFsCloseFileEvent                      mOnCloseFile;
    CbFsGetFileInfoEvent                    mOnGetFileInfo;
    CbFsEnumerateDirectoryEvent             mOnEnumerateDirectory;
    CbFsSetAllocationSizeEvent              mOnSetAllocationSize;
    CbFsSetEndOfFileEvent                   mOnSetEndOfFile;
    CbFsSetFileAttributesEvent              mOnSetFileAttributes;
    CbFsCanFileBeDeletedEvent               mOnCanFileBeDeleted;
    CbFsDeleteFileEvent                     mOnDeleteFile;
    CbFsRenameOrMoveFileEvent               mOnRenameOrMoveFile;
    CbFsReadFileEvent                       mOnReadFile;
    CbFsWriteFileEvent                      mOnWriteFile;
    CbFsCloseDirectoryEnumerationEvent      mOnCloseDirectoryEnumeration;
    CbFsCloseNamedStreamsEnumerationEvent   mOnCloseNamedStreamsEnumeration;
    CbFsIsDirectoryEmptyEvent               mOnIsDirectoryEmpty;
    CbFsEnumerateNamedStreamsEvent          mOnEnumerateNamedStreams;
    CbFsSetFileSecurityEvent                mOnSetFileSecurity;
    CbFsGetFileSecurityEvent                mOnGetFileSecurity;
    CbFsGetFileNameByFileIdEvent            mOnGetFileNameByFileId;
    CbFsFlushFileEvent                      mOnFlushFile;
    CbFsStorageEjectedEvent                 mOnStorageDestroyed;
    CbFsSetValidDataLengthEvent             mOnSetValidDataLength;

    static BOOL InternalGetMountingPoint(DWORD StorageID, INT Index, LPWSTR* OutBuf);
    static INT InternalGetMountingPointCount(DWORD StorageID);

    BOOL GetActive(void);
    BOOL GetStoragePresent(void);
};


class CbFsOpenedFilesSnapshot
{
public:
    //virtual DWORD get_FileCount(VOID) = 0;
    virtual DWORD get_HandleCount(VOID) = 0;
    virtual DWORD get_OpenCount(VOID) = 0;
    virtual BOOL GetNextOpenedFile(VOID) = 0;
    // next properties are valid only after successful call
    // of GetNextOpenedFile method
    virtual const LPWSTR& get_FileName(VOID) = 0;
    virtual const LPWSTR& get_ProcessName(VOID) = 0;
    virtual DWORD get_ProcessId(VOID) = 0;
    virtual BOOL get_HandleClosed(VOID) = 0;
    virtual ~CbFsOpenedFilesSnapshot(){};
};


class CbFsDirectoryEnumerationInfo
{
public:
    virtual VOID  set_UserContext(PVOID) = 0;
    virtual PVOID get_UserContext(VOID) = 0;
    virtual ~CbFsDirectoryEnumerationInfo(){};
};


class CbFsNamedStreamsEnumerationInfo
{
public:
    virtual VOID  set_UserContext(PVOID) = 0;
    virtual PVOID get_UserContext(VOID) = 0;
    virtual ~CbFsNamedStreamsEnumerationInfo(){};
};


class CbFsFileInfo
{
public:
    virtual CallbackFileSystem* get_Volume(VOID) = 0;
    virtual DWORD get_FileName(LPWSTR OutBuf) = 0;
    virtual LPCWSTR get_FileNameBuffer(VOID) = 0;
    virtual void set_FileName(LPCWSTR) = 0;
    virtual PVOID get_UserContext(VOID) = 0;
    virtual void set_UserContext(PVOID) = 0;
    virtual BOOL get_DeleteFile(VOID) = 0;
    virtual ~CbFsFileInfo(){};
};


class CbFsHandleInfo
{
public:
    virtual VOID  set_UserContext(PVOID) = 0;
    virtual PVOID get_UserContext(VOID) = 0;
    virtual ~CbFsHandleInfo(){};
};

#endif
