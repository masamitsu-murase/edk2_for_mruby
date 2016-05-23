#include <Python.h>

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleTextOut.h>

typedef struct {
  UINTN                        Length;
  UINTN                        BufferLength;
  CHAR8                        *Buffer;
} CON_OUT_BUFFER;

typedef struct {
  EFI_TEXT_RESET               Reset;
  EFI_TEXT_STRING              OutputString;
  EFI_TEXT_TEST_STRING         TestString;
  EFI_TEXT_QUERY_MODE          QueryMode;
  EFI_TEXT_SET_MODE            SetMode;
  EFI_TEXT_SET_ATTRIBUTE       SetAttribute;
  EFI_TEXT_CLEAR_SCREEN        ClearScreen;
  EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
  EFI_TEXT_ENABLE_CURSOR       EnableCursor;
  EFI_SIMPLE_TEXT_OUTPUT_MODE  *Mode;

  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *OriginalSimpleTextOut;
  EFI_HANDLE                   OriginalSimpleTextOutHandle;
  EFI_HANDLE                   ThisHandle;

  CON_OUT_BUFFER               *ConOutBuffer;
} CON_OUT_WRAPPER;

typedef struct {
    CON_OUT_WRAPPER ConOutWrapper;
    CON_OUT_WRAPPER StdErrWrapper;

    CON_OUT_BUFFER  ConOutBuffer;
} CON_OUT_WRAPPER_MANAGER;

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  BOOLEAN                         ExtendedVerification
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  CHAR16                          *WString
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
{
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *PassThruProtocol;
  
  PassThruProtocol = ((CON_OUT_WRAPPER *)This)->OriginalSimpleTextOut;
  
  // Pass the QueryMode call thru to the original SimpleTextOutProtocol
  return (PassThruProtocol->QueryMode(
    PassThruProtocol,
    ModeNumber,
    Columns,
    Rows));
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                         ModeNumber
  )
{
  return (EFI_UNSUPPORTED);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  UINTN                           Attribute
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  UINTN                         Column,
  IN  UINTN                         Row
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *This,
  IN  BOOLEAN                       Visible
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutOutputStringNull (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  CHAR16                          *WString
  )
{
  return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
Edk2ModuleExSimpleTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN  CHAR16                          *WString
  )
{
  UINTN i, Size;
  CON_OUT_WRAPPER *Wrapper;
  CON_OUT_BUFFER *ConOutBuffer;

  Wrapper = (CON_OUT_WRAPPER *)This;
  ConOutBuffer = Wrapper->ConOutBuffer;

  Size = StrLen(WString);
  if (ConOutBuffer->Length + Size + 1> ConOutBuffer->BufferLength) {
    UINTN NewBufferLength;

    NewBufferLength = ConOutBuffer->BufferLength * 2;
    while (ConOutBuffer->Length + Size + 1 > NewBufferLength) {
      NewBufferLength *= 2;
    }

    ConOutBuffer->Buffer = ReallocatePool(ConOutBuffer->BufferLength, NewBufferLength,
                                          ConOutBuffer->Buffer);
    if (ConOutBuffer->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    ConOutBuffer->BufferLength = NewBufferLength;
  }

  for (i=0; i<Size; i++) {
    ConOutBuffer->Buffer[ConOutBuffer->Length] = (CHAR8)WString[i];
    ConOutBuffer->Length++;
  }
  ConOutBuffer->Buffer[ConOutBuffer->Length] = '\0';

  return EFI_SUCCESS;
}

static
VOID
CalculateEfiHdrCrc (
  IN  OUT EFI_TABLE_HEADER    *Hdr
  )
{
  UINT32 Crc;

  Hdr->CRC32 = 0;

  //
  // If gBS->CalculateCrce32 () == CoreEfiNotAvailableYet () then
  //  Crc will come back as zero if we set it to zero here
  //
  Crc = 0;
  gBS->CalculateCrc32 ((UINT8 *)Hdr, Hdr->HeaderSize, &Crc);
  Hdr->CRC32 = Crc;
}

static
EFI_STATUS
InitializeConOutWrapper(CON_OUT_WRAPPER *Wrapper,
                        EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *OriginalConOut,
                        EFI_HANDLE OriginalConOutHandle,
                        int capture)
{
    EFI_STATUS Status;

    Wrapper->Reset             = Edk2ModuleExSimpleTextOutReset;
    Wrapper->TestString        = Edk2ModuleExSimpleTextOutTestString;
    Wrapper->QueryMode         = Edk2ModuleExSimpleTextOutQueryMode;
    Wrapper->SetMode           = Edk2ModuleExSimpleTextOutSetMode;
    Wrapper->SetAttribute      = Edk2ModuleExSimpleTextOutSetAttribute;
    Wrapper->ClearScreen       = Edk2ModuleExSimpleTextOutClearScreen;
    Wrapper->SetCursorPosition = Edk2ModuleExSimpleTextOutSetCursorPosition;
    Wrapper->EnableCursor      = Edk2ModuleExSimpleTextOutEnableCursor;
    switch (capture) {
      case 1:
      default:
        Wrapper->OutputString  = Edk2ModuleExSimpleTextOutOutputString;
        break;
      case 2:
        Wrapper->OutputString  = Edk2ModuleExSimpleTextOutOutputStringNull;
        break;
    }
    Wrapper->Mode              = AllocateZeroPool(sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));

    Wrapper->OriginalSimpleTextOut = OriginalConOut;
    Wrapper->OriginalSimpleTextOutHandle = OriginalConOutHandle;
    Wrapper->ThisHandle = NULL;

    Status = gBS->InstallProtocolInterface(&(Wrapper->ThisHandle),
                                           &gEfiSimpleTextOutProtocolGuid,
                                           EFI_NATIVE_INTERFACE,
                                           Wrapper);
    return Status;
}

static
EFI_STATUS
CreateConOutWrapper(CON_OUT_WRAPPER_MANAGER *Manager,
                    int capture_stdout, int capture_stderr)
{
    EFI_STATUS Status;

    Manager->ConOutBuffer.Length = 0;
    Manager->ConOutBuffer.BufferLength = 1024;
    Manager->ConOutBuffer.Buffer = AllocateZeroPool(Manager->ConOutBuffer.BufferLength);
    if (Manager->ConOutBuffer.Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    if (capture_stdout) {
        Status = InitializeConOutWrapper(&(Manager->ConOutWrapper), gST->ConOut,
                                         gST->ConsoleOutHandle, capture_stdout);
        if (EFI_ERROR(Status)) {
            return Status;
        }

        Manager->ConOutWrapper.ConOutBuffer = &(Manager->ConOutBuffer);

        gST->ConOut = (EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *)(&(Manager->ConOutWrapper));
        gST->ConsoleOutHandle = Manager->ConOutWrapper.ThisHandle;
    }

    if (capture_stderr) {
        Status = InitializeConOutWrapper(&(Manager->StdErrWrapper), gST->StdErr,
                                         gST->StandardErrorHandle, capture_stderr);
        if (EFI_ERROR(Status)) {
            return Status;
        }

        Manager->StdErrWrapper.ConOutBuffer = &(Manager->ConOutBuffer);

        gST->StdErr = (EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *)(&(Manager->StdErrWrapper));
        gST->StandardErrorHandle = Manager->StdErrWrapper.ThisHandle;
    }

    CalculateEfiHdrCrc(&gST->Hdr);

    return EFI_SUCCESS;
}

static
EFI_STATUS
RestoreConOutWrapper(CON_OUT_WRAPPER_MANAGER *Manager)
{
    if (Manager->ConOutWrapper.ThisHandle) {
        gBS->UninstallProtocolInterface(Manager->ConOutWrapper.ThisHandle,
                                        &gEfiSimpleTextOutProtocolGuid,
                                        &(Manager->ConOutWrapper));
        gST->ConOut = Manager->ConOutWrapper.OriginalSimpleTextOut;
        gST->ConsoleOutHandle = Manager->ConOutWrapper.OriginalSimpleTextOutHandle;
    }

    if (Manager->StdErrWrapper.ThisHandle) {
        gBS->UninstallProtocolInterface(Manager->StdErrWrapper.ThisHandle,
                                        &gEfiSimpleTextOutProtocolGuid,
                                        &(Manager->StdErrWrapper));
        gST->StdErr = Manager->StdErrWrapper.OriginalSimpleTextOut;
        gST->StandardErrorHandle = Manager->StdErrWrapper.OriginalSimpleTextOutHandle;
    }

    CalculateEfiHdrCrc(&gST->Hdr);

    FreePool(Manager->ConOutWrapper.Mode);
    FreePool(Manager->StdErrWrapper.Mode);
    FreePool(Manager->ConOutBuffer.Buffer);

    return EFI_SUCCESS;
}

PyObject *
edk2ex_system(char *command, int capture_stdout, int capture_stderr)
{
    EFI_STATUS Status;
    long sts;
    CON_OUT_WRAPPER_MANAGER *Manager;
    PyObject *result;

    Manager = AllocateZeroPool(sizeof(CON_OUT_WRAPPER_MANAGER));

    if (capture_stdout || capture_stderr) {
        Status = CreateConOutWrapper(Manager, capture_stdout, capture_stderr);
        if (EFI_ERROR(Status)) {
            RestoreConOutWrapper(Manager);
            FreePool(Manager);

            Py_INCREF(Py_None);
            return Py_None;
        }
    }

    Py_BEGIN_ALLOW_THREADS
    sts = system(command);
    Py_END_ALLOW_THREADS

    result = Py_BuildValue("ls", sts, Manager->ConOutBuffer.Buffer);

    RestoreConOutWrapper(Manager);
    FreePool(Manager);

    return result;
}
