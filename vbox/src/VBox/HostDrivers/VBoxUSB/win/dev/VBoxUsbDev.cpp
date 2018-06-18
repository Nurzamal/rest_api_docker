/* $Id: VBoxUsbDev.cpp 69500 2017-10-28 15:14:05Z vboxsync $ */
/** @file
 * VBoxUsbDev.cpp - USB device.
 */

/*
 * Copyright (C) 2011-2017 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 */


/*********************************************************************************************************************************
*   Header Files                                                                                                                 *
*********************************************************************************************************************************/
#include "VBoxUsbCmn.h"
#include <iprt/assert.h>
#include <VBox/log.h>


/*********************************************************************************************************************************
*   Defined Constants And Macros                                                                                                 *
*********************************************************************************************************************************/
#define VBOXUSB_MEMTAG 'bUBV'



DECLHIDDEN(PVOID) vboxUsbMemAlloc(SIZE_T cbBytes)
{
    PVOID pvMem = ExAllocatePoolWithTag(NonPagedPool, cbBytes, VBOXUSB_MEMTAG);
    Assert(pvMem);
    return pvMem;
}

DECLHIDDEN(PVOID) vboxUsbMemAllocZ(SIZE_T cbBytes)
{
    PVOID pvMem = vboxUsbMemAlloc(cbBytes);
    if (pvMem)
    {
        RtlZeroMemory(pvMem, cbBytes);
    }
    return pvMem;
}

DECLHIDDEN(VOID) vboxUsbMemFree(PVOID pvMem)
{
    ExFreePoolWithTag(pvMem, VBOXUSB_MEMTAG);
}

VBOXUSB_GLOBALS g_VBoxUsbGlobals = {0};

static NTSTATUS vboxUsbDdiAddDevice(PDRIVER_OBJECT pDriverObject,
            PDEVICE_OBJECT pPDO)
{
    PDEVICE_OBJECT pFDO = NULL;
    NTSTATUS Status = IoCreateDevice(pDriverObject,
            sizeof (VBOXUSBDEV_EXT),
            NULL, /* IN PUNICODE_STRING pDeviceName OPTIONAL */
            FILE_DEVICE_UNKNOWN, /* IN DEVICE_TYPE DeviceType */
            FILE_AUTOGENERATED_DEVICE_NAME, /* IN ULONG DeviceCharacteristics */
            FALSE, /* IN BOOLEAN fExclusive */
            &pFDO);
    Assert(Status == STATUS_SUCCESS);
    if (Status == STATUS_SUCCESS)
    {
        PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pFDO->DeviceExtension;
        /* init Device Object bits */
        pFDO->Flags |= DO_DIRECT_IO;
        if (pPDO->Flags & DO_POWER_PAGABLE)
            pFDO->Flags |= DO_POWER_PAGABLE;


        /* now init our state bits */

        pDevExt->cHandles = 0;

        pDevExt->pFDO = pFDO;
        pDevExt->pPDO = pPDO;
        pDevExt->pLowerDO = IoAttachDeviceToDeviceStack(pFDO, pPDO);
        Assert(pDevExt->pLowerDO);
        if (pDevExt->pLowerDO)
        {
            vboxUsbDdiStateInit(pDevExt);
            Status = vboxUsbRtInit(pDevExt);
            if (Status == STATUS_SUCCESS)
            {
                /* we're done! */
                pFDO->Flags &= ~DO_DEVICE_INITIALIZING;
                return STATUS_SUCCESS;
            }

            IoDetachDevice(pDevExt->pLowerDO);
        }
        else
            Status = STATUS_NO_SUCH_DEVICE;

        IoDeleteDevice(pFDO);
    }

    return Status;
}

static VOID vboxUsbDdiUnload(PDRIVER_OBJECT pDriverObject)
{
    RT_NOREF1(pDriverObject);
    LogRel(("VBoxUsb::DriverUnload. Built Date (%s) Time (%s)\n", __DATE__, __TIME__));
    VBoxDrvToolStrFree(&g_VBoxUsbGlobals.RegPath);

    vboxUsbRtGlobalsTerm();

    PRTLOGGER pLogger = RTLogRelSetDefaultInstance(NULL);
    if (pLogger)
    {
        RTLogDestroy(pLogger);
    }
    pLogger = RTLogSetDefaultInstance(NULL);
    if (pLogger)
    {
        RTLogDestroy(pLogger);
    }
}

static NTSTATUS vboxUsbDispatchCreate(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_INVALID_HANDLE;
    do
    {
        if (vboxUsbPnPStateGet(pDevExt) != ENMVBOXUSB_PNPSTATE_STARTED)
        {
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }

        PIO_STACK_LOCATION pSl = IoGetCurrentIrpStackLocation(pIrp);
        PFILE_OBJECT pFObj = pSl->FileObject;
        if (!pFObj)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        pFObj->FsContext = NULL;

        if (pFObj->FileName.Length)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Status = vboxUsbRtCreate(pDevExt, pIrp);
        if (!NT_SUCCESS(Status))
        {
            AssertFailed();
            break;
        }

        ASMAtomicIncU32(&pDevExt->cHandles);
        Status = STATUS_SUCCESS;
        break;
    } while (0);

    Status = VBoxDrvToolIoComplete(pIrp, Status, 0);
    return Status;
}

static NTSTATUS vboxUsbDispatchClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pDeviceObject->DeviceExtension;
    NTSTATUS Status = STATUS_SUCCESS;
#ifdef VBOX_STRICT
    PIO_STACK_LOCATION pSl = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT pFObj = pSl->FileObject;
    Assert(pFObj);
    Assert(!pFObj->FileName.Length);
#endif
    Status = vboxUsbRtClose(pDevExt, pIrp);
    if (NT_SUCCESS(Status))
    {
        ASMAtomicDecU32(&pDevExt->cHandles);
    }
    else
    {
        AssertFailed();
    }
    Status = VBoxDrvToolIoComplete(pIrp, Status, 0);
    return Status;
}

static NTSTATUS vboxUsbDispatchDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pDeviceObject->DeviceExtension;
    if (vboxUsbDdiStateRetainIfStarted(pDevExt))
        return vboxUsbRtDispatch(pDevExt, pIrp);
    return VBoxDrvToolIoComplete(pIrp, STATUS_INVALID_DEVICE_STATE, 0);
}

static NTSTATUS vboxUsbDispatchCleanup(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    RT_NOREF1(pDeviceObject);
    return VBoxDrvToolIoComplete(pIrp, STATUS_SUCCESS, 0);
}

static NTSTATUS vboxUsbDevAccessDeviedDispatchStub(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pDeviceObject->DeviceExtension;
    if (!vboxUsbDdiStateRetainIfNotRemoved(pDevExt))
    {
        VBoxDrvToolIoComplete(pIrp, STATUS_DELETE_PENDING, 0);
        return STATUS_DELETE_PENDING;
    }

    NTSTATUS Status = VBoxDrvToolIoComplete(pIrp, STATUS_ACCESS_DENIED, 0);

    vboxUsbDdiStateRelease(pDevExt);

    return Status;
}

static NTSTATUS vboxUsbDispatchSystemControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    PVBOXUSBDEV_EXT pDevExt = (PVBOXUSBDEV_EXT)pDeviceObject->DeviceExtension;
    if (!vboxUsbDdiStateRetainIfNotRemoved(pDevExt))
    {
        VBoxDrvToolIoComplete(pIrp, STATUS_DELETE_PENDING, 0);
        return STATUS_DELETE_PENDING;
    }

    IoSkipCurrentIrpStackLocation(pIrp);

    NTSTATUS Status = IoCallDriver(pDevExt->pLowerDO, pIrp);

    vboxUsbDdiStateRelease(pDevExt);

    return Status;
}

static NTSTATUS vboxUsbDispatchRead(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
#ifdef DEBUG_misha
    AssertFailed();
#endif
    return vboxUsbDevAccessDeviedDispatchStub(pDeviceObject, pIrp);
}

static NTSTATUS vboxUsbDispatchWrite(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
#ifdef DEBUG_misha
    AssertFailed();
#endif
    return vboxUsbDevAccessDeviedDispatchStub(pDeviceObject, pIrp);
}

RT_C_DECLS_BEGIN

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath);

RT_C_DECLS_END

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
    LogRel(("VBoxUsb::DriverEntry. Built Date (%s) Time (%s)\n", __DATE__, __TIME__));

    NTSTATUS Status = vboxUsbRtGlobalsInit();
    Assert(Status == STATUS_SUCCESS);
    if (Status == STATUS_SUCCESS)
    {
        Status = VBoxDrvToolStrCopy(&g_VBoxUsbGlobals.RegPath, pRegistryPath);
        Assert(Status == STATUS_SUCCESS);
        if (Status == STATUS_SUCCESS)
        {
            g_VBoxUsbGlobals.pDrvObj = pDriverObject;

            pDriverObject->DriverExtension->AddDevice = vboxUsbDdiAddDevice;

            pDriverObject->DriverUnload = vboxUsbDdiUnload;

            pDriverObject->MajorFunction[IRP_MJ_CREATE] = vboxUsbDispatchCreate;
            pDriverObject->MajorFunction[IRP_MJ_CLOSE] =  vboxUsbDispatchClose;
            pDriverObject->MajorFunction[IRP_MJ_READ] = vboxUsbDispatchRead;
            pDriverObject->MajorFunction[IRP_MJ_WRITE] = vboxUsbDispatchWrite;
            pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = vboxUsbDispatchDeviceControl;
            pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = vboxUsbDispatchCleanup;
            pDriverObject->MajorFunction[IRP_MJ_POWER] = vboxUsbDispatchPower;
            pDriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = vboxUsbDispatchSystemControl;
            pDriverObject->MajorFunction[IRP_MJ_PNP] = vboxUsbDispatchPnP;

            return STATUS_SUCCESS;
        }
        vboxUsbRtGlobalsTerm();
    }

    LogRel(("VBoxUsb::DriverEntry. failed with Status (0x%x)\n", Status));

    return Status;
}

#ifdef VBOX_STRICT
DECLHIDDEN(VOID) vboxUsbPnPStateGbgChange(ENMVBOXUSB_PNPSTATE enmOldState, ENMVBOXUSB_PNPSTATE enmNewState)
{
    /* *ensure the state change is valid */
    switch (enmNewState)
    {
        case ENMVBOXUSB_PNPSTATE_STARTED:
            Assert(   enmOldState == ENMVBOXUSB_PNPSTATE_START_PENDING
                   || enmOldState == ENMVBOXUSB_PNPSTATE_REMOVE_PENDING
                   || enmOldState == ENMVBOXUSB_PNPSTATE_STOPPED
                   || enmOldState == ENMVBOXUSB_PNPSTATE_STOP_PENDING);
            break;
        case ENMVBOXUSB_PNPSTATE_STOP_PENDING:
            Assert(enmOldState == ENMVBOXUSB_PNPSTATE_STARTED);
            break;
        case ENMVBOXUSB_PNPSTATE_STOPPED:
            Assert(enmOldState == ENMVBOXUSB_PNPSTATE_STOP_PENDING);
            break;
        case ENMVBOXUSB_PNPSTATE_SURPRISE_REMOVED:
            Assert(enmOldState == ENMVBOXUSB_PNPSTATE_STARTED);
            break;
        case ENMVBOXUSB_PNPSTATE_REMOVE_PENDING:
            Assert(enmOldState == ENMVBOXUSB_PNPSTATE_STARTED);
            break;
        case ENMVBOXUSB_PNPSTATE_REMOVED:
            Assert(   enmOldState == ENMVBOXUSB_PNPSTATE_REMOVE_PENDING
                   || enmOldState == ENMVBOXUSB_PNPSTATE_SURPRISE_REMOVED);
            break;
        default:
            AssertFailed();
            break;
    }

}
#endif
