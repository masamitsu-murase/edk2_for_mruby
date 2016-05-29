/** @file
  Provides interface to advanced shell functionality for parsing both handle and protocol database.

  Copyright (c) 2011 - 2015, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
  (C) Copyright 2013-2016 Hewlett-Packard Development Company, L.P.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _UEFI_HANDLE_PARSING_LIB_INTERNAL_H_
#define _UEFI_HANDLE_PARSING_LIB_INTERNAL_H_

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/ConsoleInDevice.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/StandardErrorDevice.h>
#include <Guid/GlobalVariable.h>
#include <Guid/Gpt.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/ShellLibHiiGuid.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellEnvironment2.h>
#include <Protocol/EfiShell.h>
#include <Protocol/EfiShellParameters.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DriverConfiguration2.h>
#include <Protocol/DriverConfiguration.h>
#include <Protocol/DriverDiagnostics2.h>
#include <Protocol/DriverDiagnostics.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ComponentName.h>
#include <Protocol/PlatformDriverOverride.h>
#include <Protocol/DevicePathUtilities.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/BusSpecificDriverOverride.h>
#include <Protocol/PlatformToDriverConfiguration.h>
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/SerialIo.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidOverride.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LoadFile2.h>
#include <Protocol/TapeIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/ScsiPassThru.h>
#include <Protocol/ScsiPassThruExt.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/IScsiInitiatorName.h>
#include <Protocol/UsbIo.h>
#include <Protocol/UsbHostController.h>
#include <Protocol/Usb2HostController.h>
#include <Protocol/DebugSupport.h>
#include <Protocol/DebugPort.h>
#include <Protocol/Decompress.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/Ebc.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/NetworkInterfaceIdentifier.h>
#include <Protocol/PxeBaseCode.h>
#include <Protocol/PxeBaseCodeCallBack.h>
#include <Protocol/Bis.h>
#include <Protocol/ManagedNetwork.h>
#include <Protocol/Arp.h>
#include <Protocol/Dhcp4.h>
#include <Protocol/Tcp4.h>
#include <Protocol/Ip4.h>
#include <Protocol/Ip4Config.h>
#include <Protocol/Ip4Config2.h>
#include <Protocol/Udp4.h>
#include <Protocol/Mtftp4.h>
#include <Protocol/AuthenticationInfo.h>
#include <Protocol/Hash.h>
#include <Protocol/HiiFont.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/DeviceIo.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/UgaIo.h>
#include <Protocol/DriverConfiguration.h>
#include <Protocol/DriverConfiguration2.h>
#include <Protocol/DevicePathUtilities.h>
//#include <Protocol/FirmwareVolume.h>
//#include <Protocol/FirmwareVolume2.h>
#include <Protocol/DriverFamilyOverride.h>
#include <Protocol/Pcd.h>
#include <Protocol/TcgService.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/Ip6.h>
#include <Protocol/Ip6Config.h>
#include <Protocol/Mtftp6.h>
#include <Protocol/Dhcp6.h>
#include <Protocol/Udp6.h>
#include <Protocol/Tcp6.h>
#include <Protocol/VlanConfig.h>
#include <Protocol/Eap.h>
#include <Protocol/EapManagement.h>
#include <Protocol/Ftp4.h>
#include <Protocol/IpSecConfig.h>
#include <Protocol/DriverHealth.h>
#include <Protocol/DeferredImageLoad.h>
#include <Protocol/UserCredential.h>
#include <Protocol/UserManager.h>
#include <Protocol/AtaPassThru.h>
#include <Protocol/FirmwareManagement.h>
#include <Protocol/IpSec.h>
#include <Protocol/Kms.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/StorageSecurityCommand.h>
#include <Protocol/UserCredential2.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/AdapterInformation.h>
#include <Protocol/EfiShellDynamicCommand.h>
#include <Protocol/DiskInfo.h>

#include <Library/HandleParsingLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include <Library/ShellLib.h>
#include <Library/SortLib.h>
#include <Library/ShellCommandLib.h>

#define   EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION_V1   1
#define   EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION_V2   2


extern EFI_GUID gEfiShellProtocolGuid;
extern EFI_GUID gEfiShellParametersProtocolGuid;
extern EFI_GUID gEfiSimpleTextInputExProtocolGuid;
extern EFI_GUID gEfiSimpleTextInProtocolGuid;
extern EFI_GUID gEfiSimpleTextOutProtocolGuid;
extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
extern EFI_GUID gEfiLoadedImageProtocolGuid;
extern EFI_GUID gEfiComponentName2ProtocolGuid;
extern EFI_GUID gEfiUnicodeCollation2ProtocolGuid;
extern EFI_GUID gEfiDevicePathProtocolGuid;
extern EFI_GUID gEfiHiiPackageListProtocolGuid;
extern EFI_GUID gEfiDevicePathUtilitiesProtocolGuid;
extern EFI_GUID gEfiDevicePathToTextProtocolGuid;
extern EFI_GUID gEfiDevicePathFromTextProtocolGuid;
extern EFI_GUID gEfiDebugPortProtocolGuid;
extern EFI_GUID gEfiHiiFontProtocolGuid;
extern EFI_GUID gEfiHiiStringProtocolGuid;
extern EFI_GUID gEfiHiiImageProtocolGuid;
extern EFI_GUID gEfiHiiDatabaseProtocolGuid;
extern EFI_GUID gEfiHiiConfigRoutingProtocolGuid;
extern EFI_GUID gEfiDriverBindingProtocolGuid;
extern EFI_GUID gEfiGraphicsOutputProtocolGuid;
extern EFI_GUID gEfiUgaDrawProtocolGuid;
extern EFI_GUID gEfiComponentNameProtocolGuid;
extern EFI_GUID gEfiDriverConfigurationProtocolGuid;
extern EFI_GUID gEfiDriverConfiguration2ProtocolGuid;
extern EFI_GUID gEfiDriverDiagnosticsProtocolGuid;
extern EFI_GUID gEfiDriverDiagnostics2ProtocolGuid;
extern EFI_GUID gEfiFormBrowser2ProtocolGuid;
extern EFI_GUID gEfiShellEnvironment2Guid;
extern EFI_GUID gEfiShellInterfaceGuid;
extern EFI_GUID gEfiBusSpecificDriverOverrideProtocolGuid;
extern EFI_GUID gEfiPlatformDriverOverrideProtocolGuid;
extern EFI_GUID gEfiPlatformToDriverConfigurationProtocolGuid;
extern EFI_GUID gEfiDriverSupportedEfiVersionProtocolGuid;
extern EFI_GUID gEfiLoadedImageDevicePathProtocolGuid;
extern EFI_GUID gEfiSimplePointerProtocolGuid;
extern EFI_GUID gEfiAbsolutePointerProtocolGuid;
extern EFI_GUID gEfiSerialIoProtocolGuid;
extern EFI_GUID gEfiEdidDiscoveredProtocolGuid;
extern EFI_GUID gEfiEdidActiveProtocolGuid;
extern EFI_GUID gEfiEdidOverrideProtocolGuid;
extern EFI_GUID gEfiLoadFileProtocolGuid;
extern EFI_GUID gEfiLoadFile2ProtocolGuid;
extern EFI_GUID gEfiTapeIoProtocolGuid;
extern EFI_GUID gEfiDiskIoProtocolGuid;
extern EFI_GUID gEfiBlockIoProtocolGuid;
extern EFI_GUID gEfiUnicodeCollationProtocolGuid;
extern EFI_GUID gEfiPciRootBridgeIoProtocolGuid;
extern EFI_GUID gEfiPciIoProtocolGuid;
extern EFI_GUID gEfiScsiPassThruProtocolGuid;
extern EFI_GUID gEfiScsiIoProtocolGuid;
extern EFI_GUID gEfiExtScsiPassThruProtocolGuid;
extern EFI_GUID gEfiIScsiInitiatorNameProtocolGuid;
extern EFI_GUID gEfiUsbIoProtocolGuid;
extern EFI_GUID gEfiUsbHcProtocolGuid;
extern EFI_GUID gEfiUsb2HcProtocolGuid;
extern EFI_GUID gEfiDebugSupportProtocolGuid;
extern EFI_GUID gEfiDecompressProtocolGuid;
extern EFI_GUID gEfiAcpiTableProtocolGuid;
extern EFI_GUID gEfiEbcProtocolGuid;
extern EFI_GUID gEfiSimpleNetworkProtocolGuid;
extern EFI_GUID gEfiNetworkInterfaceIdentifierProtocolGuid;
extern EFI_GUID gEfiNetworkInterfaceIdentifierProtocolGuid_31;
extern EFI_GUID gEfiPxeBaseCodeProtocolGuid;
extern EFI_GUID gEfiPxeBaseCodeCallbackProtocolGuid;
extern EFI_GUID gEfiBisProtocolGuid;
extern EFI_GUID gEfiManagedNetworkServiceBindingProtocolGuid;
extern EFI_GUID gEfiManagedNetworkProtocolGuid;
extern EFI_GUID gEfiArpServiceBindingProtocolGuid;
extern EFI_GUID gEfiArpProtocolGuid;
extern EFI_GUID gEfiDhcp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiDhcp4ProtocolGuid;
extern EFI_GUID gEfiTcp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiTcp4ProtocolGuid;
extern EFI_GUID gEfiIp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiIp4ProtocolGuid;
extern EFI_GUID gEfiIp4ConfigProtocolGuid;
extern EFI_GUID gEfiIp4Config2ProtocolGuid;
extern EFI_GUID gEfiUdp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiUdp4ProtocolGuid;
extern EFI_GUID gEfiMtftp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiMtftp4ProtocolGuid;
extern EFI_GUID gEfiAuthenticationInfoProtocolGuid;
extern EFI_GUID gEfiHashServiceBindingProtocolGuid;
extern EFI_GUID gEfiHashProtocolGuid;
extern EFI_GUID gEfiHiiConfigAccessProtocolGuid;
extern EFI_GUID gEfiDeviceIoProtocolGuid;
extern EFI_GUID gEfiUgaIoProtocolGuid;
extern EFI_GUID gEfiIp6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiIp6ProtocolGuid;
extern EFI_GUID gEfiIp6ConfigProtocolGuid;
extern EFI_GUID gEfiMtftp6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiMtftp6ProtocolGuid;
extern EFI_GUID gEfiDhcp6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiDhcp6ProtocolGuid;
extern EFI_GUID gEfiUdp6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiUdp6ProtocolGuid;
extern EFI_GUID gEfiTcp6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiTcp6ProtocolGuid;
extern EFI_GUID gEfiVlanConfigProtocolGuid;
extern EFI_GUID gEfiEapProtocolGuid;
extern EFI_GUID gEfiEapManagementProtocolGuid;
extern EFI_GUID gEfiFtp4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiFtp4ProtocolGuid;
extern EFI_GUID gEfiIpSecConfigProtocolGuid;
extern EFI_GUID gEfiDriverHealthProtocolGuid;
extern EFI_GUID gEfiDeferredImageLoadProtocolGuid;
extern EFI_GUID gEfiUserCredentialProtocolGuid;
extern EFI_GUID gEfiUserManagerProtocolGuid;
extern EFI_GUID gEfiAtaPassThruProtocolGuid;
extern EFI_GUID gEfiFirmwareManagementProtocolGuid;
extern EFI_GUID gEfiIpSecProtocolGuid;
extern EFI_GUID gEfiIpSec2ProtocolGuid;
extern EFI_GUID gEfiKmsProtocolGuid;
extern EFI_GUID gEfiBlockIo2ProtocolGuid;
extern EFI_GUID gEfiStorageSecurityCommandProtocolGuid;
extern EFI_GUID gEfiUserCredential2ProtocolGuid;
extern EFI_GUID gPcdProtocolGuid;
extern EFI_GUID gEfiTcgProtocolGuid;
extern EFI_GUID gEfiDriverFamilyOverrideProtocolGuid;
extern EFI_GUID gEfiIdeControllerInitProtocolGuid;
extern EFI_GUID gEfiDiskIo2ProtocolGuid;
extern EFI_GUID gEfiAdapterInformationProtocolGuid;
extern EFI_GUID gEfiShellDynamicCommandProtocolGuid;
extern EFI_GUID gEfiDiskInfoProtocolGuid;
extern EFI_GUID gGetPcdInfoProtocolGuid;
extern EFI_GUID gEfiBdsArchProtocolGuid;
extern EFI_GUID gEfiCpuArchProtocolGuid;
extern EFI_GUID gEfiMetronomeArchProtocolGuid;
extern EFI_GUID gEfiMonotonicCounterArchProtocolGuid;
extern EFI_GUID gEfiRealTimeClockArchProtocolGuid;
extern EFI_GUID gEfiResetArchProtocolGuid;
extern EFI_GUID gEfiRuntimeArchProtocolGuid;
extern EFI_GUID gEfiSecurityArchProtocolGuid;
extern EFI_GUID gEfiTimerArchProtocolGuid;
extern EFI_GUID gEfiVariableWriteArchProtocolGuid;
extern EFI_GUID gEfiVariableArchProtocolGuid;
extern EFI_GUID gEfiSecurityPolicyProtocolGuid;
extern EFI_GUID gEfiWatchdogTimerArchProtocolGuid;
extern EFI_GUID gEfiStatusCodeRuntimeProtocolGuid;
extern EFI_GUID gEfiSmbusHcProtocolGuid;
extern EFI_GUID gEfiFirmwareVolume2ProtocolGuid;
extern EFI_GUID gEfiFirmwareVolumeBlockProtocolGuid;
extern EFI_GUID gEfiCapsuleArchProtocolGuid;
extern EFI_GUID gEfiMpServiceProtocolGuid;
extern EFI_GUID gEfiPciHostBridgeResourceAllocationProtocolGuid;
extern EFI_GUID gEfiPciPlatformProtocolGuid;
extern EFI_GUID gEfiPciOverrideProtocolGuid;
extern EFI_GUID gEfiPciEnumerationCompleteProtocolGuid;
extern EFI_GUID gEfiIncompatiblePciDeviceSupportProtocolGuid;
extern EFI_GUID gEfiPciHotPlugInitProtocolGuid;
extern EFI_GUID gEfiPciHotPlugRequestProtocolGuid;
extern EFI_GUID gEfiSmbiosProtocolGuid;
extern EFI_GUID gEfiS3SaveStateProtocolGuid;
extern EFI_GUID gEfiS3SmmSaveStateProtocolGuid;
extern EFI_GUID gEfiRscHandlerProtocolGuid;
extern EFI_GUID gEfiSmmRscHandlerProtocolGuid;
extern EFI_GUID gEfiAcpiSdtProtocolGuid;
extern EFI_GUID gEfiSioProtocolGuid;
extern EFI_GUID gEfiSmmCpuIo2ProtocolGuid;
extern EFI_GUID gEfiSmmBase2ProtocolGuid;
extern EFI_GUID gEfiSmmAccess2ProtocolGuid;
extern EFI_GUID gEfiSmmControl2ProtocolGuid;
extern EFI_GUID gEfiSmmConfigurationProtocolGuid;
extern EFI_GUID gEfiSmmReadyToLockProtocolGuid;
extern EFI_GUID gEfiDxeSmmReadyToLockProtocolGuid;
extern EFI_GUID gEfiSmmCommunicationProtocolGuid;
extern EFI_GUID gEfiSmmStatusCodeProtocolGuid;
extern EFI_GUID gEfiSmmCpuProtocolGuid;
extern EFI_GUID gEfiSmmPciRootBridgeIoProtocolGuid;
extern EFI_GUID gEfiSmmSwDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmSxDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmPeriodicTimerDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmUsbDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmGpiDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmStandbyButtonDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmPowerButtonDispatch2ProtocolGuid;
extern EFI_GUID gEfiSmmIoTrapDispatch2ProtocolGuid;
extern EFI_GUID gEfiPcdProtocolGuid;
extern EFI_GUID gEfiFirmwareVolumeBlock2ProtocolGuid;
extern EFI_GUID gEfiCpuIo2ProtocolGuid;
extern EFI_GUID gEfiLegacyRegion2ProtocolGuid;
extern EFI_GUID gEfiSalMcaInitPmiProtocolGuid;
extern EFI_GUID gEfiExtendedSalBootServiceProtocolGuid;
extern EFI_GUID gEfiExtendedSalBaseIoServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalStallServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalRtcServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalVariableServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalMtcServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalResetServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalStatusCodeServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalFvBlockServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalMpServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalPalServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalBaseServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalMcaServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalPciServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalCacheServicesProtocolGuid;
extern EFI_GUID gEfiExtendedSalMcaLogServicesProtocolGuid;
extern EFI_GUID gEfiSecurity2ArchProtocolGuid;
extern EFI_GUID gEfiSmmEndOfDxeProtocolGuid;
extern EFI_GUID gEfiIsaHcProtocolGuid;
extern EFI_GUID gEfiIsaHcServiceBindingProtocolGuid;
extern EFI_GUID gEfiSioControlProtocolGuid;
extern EFI_GUID gEfiGetPcdInfoProtocolGuid;
extern EFI_GUID gEfiI2cMasterProtocolGuid;
extern EFI_GUID gEfiI2cIoProtocolGuid;
extern EFI_GUID gEfiI2cEnumerateProtocolGuid;
extern EFI_GUID gEfiI2cHostProtocolGuid;
extern EFI_GUID gEfiI2cBusConfigurationManagementProtocolGuid;
extern EFI_GUID gEfiTrEEProtocolGuid;
extern EFI_GUID gEfiTcg2ProtocolGuid;
extern EFI_GUID gEfiTimestampProtocolGuid;
extern EFI_GUID gEfiRngProtocolGuid;
extern EFI_GUID gEfiNvmExpressPassThruProtocolGuid;
extern EFI_GUID gEfiHash2ServiceBindingProtocolGuid;
extern EFI_GUID gEfiHash2ProtocolGuid;
extern EFI_GUID gEfiBlockIoCryptoProtocolGuid;
extern EFI_GUID gEfiSmartCardReaderProtocolGuid;
extern EFI_GUID gEfiSmartCardEdgeProtocolGuid;
extern EFI_GUID gEfiUsbFunctionIoProtocolGuid;
extern EFI_GUID gEfiBluetoothHcProtocolGuid;
extern EFI_GUID gEfiBluetoothIoServiceBindingProtocolGuid;
extern EFI_GUID gEfiBluetoothIoProtocolGuid;
extern EFI_GUID gEfiBluetoothConfigProtocolGuid;
extern EFI_GUID gEfiRegularExpressionProtocolGuid;
extern EFI_GUID gEfiBootManagerPolicyProtocolGuid;
extern EFI_GUID gEfiConfigKeywordHandlerProtocolGuid;
extern EFI_GUID gEfiWiFiProtocolGuid;
extern EFI_GUID gEfiEapManagement2ProtocolGuid;
extern EFI_GUID gEfiEapConfigurationProtocolGuid;
extern EFI_GUID gEfiPkcs7VerifyProtocolGuid;
extern EFI_GUID gEfiDns4ServiceBindingProtocolGuid;
extern EFI_GUID gEfiDns4ProtocolGuid;
extern EFI_GUID gEfiDns6ServiceBindingProtocolGuid;
extern EFI_GUID gEfiDns6ProtocolGuid;
extern EFI_GUID gEfiHttpServiceBindingProtocolGuid;
extern EFI_GUID gEfiHttpProtocolGuid;
extern EFI_GUID gEfiHttpUtilitiesProtocolGuid;
extern EFI_GUID gEfiRestProtocolGuid;


///
/// EFI_FIRMWARE_IMAGE_DESCRIPTOR in UEFI spec < 2.4a
///
typedef struct {
  /// 
  /// A unique number identifying the firmware image within the device.  The number is 
  /// between 1 and DescriptorCount.
  /// 
  UINT8                            ImageIndex;
  ///
  /// A unique number identifying the firmware image type.  
  /// 
  EFI_GUID                         ImageTypeId;
  /// 
  /// A unique number identifying the firmware image. 
  /// 
  UINT64                           ImageId;
  /// 
  /// A pointer to a null-terminated string representing the firmware image name.  
  /// 
  CHAR16                           *ImageIdName;
  /// 
  /// Identifies the version of the device firmware. The format is vendor specific and new 
  /// version must have a greater value than an old version.
  /// 
  UINT32                           Version;
  /// 
  /// A pointer to a null-terminated string representing the firmware image version name.
  /// 
  CHAR16                           *VersionName;
  /// 
  /// Size of the image in bytes.  If size=0, then only ImageIndex and ImageTypeId are valid.
  /// 
  UINTN                            Size;
  /// 
  /// Image attributes that are supported by this device.  See 'Image Attribute Definitions' 
  /// for possible returned values of this parameter.  A value of 1 indicates the attribute is 
  /// supported and the current setting value is indicated in AttributesSetting.  A 
  /// value of 0 indicates the attribute is not supported and the current setting value in 
  /// AttributesSetting is meaningless.
  /// 
  UINT64                           AttributesSupported;
  /// 
  /// Image attributes.  See 'Image Attribute Definitions' for possible returned values of 
  /// this parameter.
  /// 
  UINT64                           AttributesSetting;
  /// 
  /// Image compatibilities.  See 'Image Compatibility Definitions' for possible returned 
  /// values of this parameter.
  /// 
  UINT64                           Compatibilities;
} EFI_FIRMWARE_IMAGE_DESCRIPTOR_V1;


///
/// EFI_FIRMWARE_IMAGE_DESCRIPTOR in UEFI spec > 2.4a and < 2.5
///
typedef struct {
  /// 
  /// A unique number identifying the firmware image within the device.  The number is 
  /// between 1 and DescriptorCount.
  /// 
  UINT8                            ImageIndex;
  ///
  /// A unique number identifying the firmware image type.  
  /// 
  EFI_GUID                         ImageTypeId;
  /// 
  /// A unique number identifying the firmware image. 
  /// 
  UINT64                           ImageId;
  /// 
  /// A pointer to a null-terminated string representing the firmware image name.  
  /// 
  CHAR16                           *ImageIdName;
  /// 
  /// Identifies the version of the device firmware. The format is vendor specific and new 
  /// version must have a greater value than an old version.
  /// 
  UINT32                           Version;
  /// 
  /// A pointer to a null-terminated string representing the firmware image version name.
  /// 
  CHAR16                           *VersionName;
  /// 
  /// Size of the image in bytes.  If size=0, then only ImageIndex and ImageTypeId are valid.
  /// 
  UINTN                            Size;
  /// 
  /// Image attributes that are supported by this device.  See 'Image Attribute Definitions' 
  /// for possible returned values of this parameter.  A value of 1 indicates the attribute is 
  /// supported and the current setting value is indicated in AttributesSetting.  A 
  /// value of 0 indicates the attribute is not supported and the current setting value in 
  /// AttributesSetting is meaningless.
  /// 
  UINT64                           AttributesSupported;
  /// 
  /// Image attributes.  See 'Image Attribute Definitions' for possible returned values of 
  /// this parameter.
  /// 
  UINT64                           AttributesSetting;
  /// 
  /// Image compatibilities.  See 'Image Compatibility Definitions' for possible returned 
  /// values of this parameter.
  /// 
  UINT64                           Compatibilities;
  ///
  /// Describes the lowest ImageDescriptor version that the device will accept. Only
  /// present in version 2 or higher.
  UINT32                           LowestSupportedImageVersion;
} EFI_FIRMWARE_IMAGE_DESCRIPTOR_V2;

typedef struct {
  LIST_ENTRY  Link;
  EFI_HANDLE  TheHandle;
  UINTN       TheIndex;
}HANDLE_LIST;

typedef struct {
  HANDLE_LIST   List;
  UINTN         NextIndex;
} HANDLE_INDEX_LIST;

typedef
CHAR16 *
(EFIAPI *DUMP_PROTOCOL_INFO)(
  IN CONST EFI_HANDLE TheHandle,
  IN CONST BOOLEAN    Verbose
  );

typedef struct _GUID_INFO_BLOCK{
  EFI_STRING_ID                 StringId;
  EFI_GUID                      *GuidId;
  DUMP_PROTOCOL_INFO            DumpInfo;
} GUID_INFO_BLOCK;

#endif

