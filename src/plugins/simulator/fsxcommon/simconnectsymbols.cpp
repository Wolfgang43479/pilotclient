/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simconnectsymbols.h"
#include "blackmisc/logmessage.h"
#include <QLibrary>
#include <Windows.h>
#include <SimConnect.h>
#include <array>

using namespace BlackMisc;

#ifdef Q_OS_WIN64

bool loadAndResolveSimConnect(bool manifestProbing)
{
    Q_UNUSED(manifestProbing);
    return true;
}

#else

typedef HRESULT(__stdcall *PfnSimConnect_Open)(HANDLE *, LPCSTR, HWND, DWORD, HANDLE, DWORD);
typedef HRESULT(__stdcall *PfnSimConnect_Close)(HANDLE);
typedef HRESULT(__stdcall *PfnSimConnect_AddToDataDefinition)(HANDLE, SIMCONNECT_DATA_DEFINITION_ID, const char *, const char *, SIMCONNECT_DATATYPE, float, DWORD);
typedef HRESULT(__stdcall *PfnSimConnect_Text)(HANDLE, SIMCONNECT_TEXT_TYPE, float, SIMCONNECT_CLIENT_EVENT_ID, DWORD, void *);
typedef HRESULT(__stdcall *PfnSimConnect_CallDispatch)(HANDLE, DispatchProc, void *);
typedef HRESULT(__stdcall *PfnSimConnect_WeatherSetModeCustom)(HANDLE);
typedef HRESULT(__stdcall *PfnSimConnect_WeatherSetModeGlobal)(HANDLE);
typedef HRESULT(__stdcall *PfnSimConnect_WeatherSetObservation)(HANDLE, DWORD, const char *);
typedef HRESULT(__stdcall *PfnSimConnect_TransmitClientEvent)(HANDLE, SIMCONNECT_OBJECT_ID, SIMCONNECT_CLIENT_EVENT_ID, DWORD, SIMCONNECT_NOTIFICATION_GROUP_ID, SIMCONNECT_EVENT_FLAG);
typedef HRESULT(__stdcall *PfnSimConnect_SetClientData)(HANDLE, SIMCONNECT_CLIENT_DATA_ID, SIMCONNECT_CLIENT_DATA_DEFINITION_ID, SIMCONNECT_CLIENT_DATA_SET_FLAG, DWORD, DWORD, void *);
typedef HRESULT(__stdcall *PfnSimConnect_RequestDataOnSimObject)(HANDLE, SIMCONNECT_DATA_REQUEST_ID, SIMCONNECT_DATA_DEFINITION_ID, SIMCONNECT_OBJECT_ID, SIMCONNECT_PERIOD, SIMCONNECT_DATA_REQUEST_FLAG, DWORD, DWORD, DWORD);
typedef HRESULT(__stdcall *PfnSimConnect_RequestClientData)(HANDLE, SIMCONNECT_CLIENT_DATA_ID, SIMCONNECT_DATA_REQUEST_ID, SIMCONNECT_CLIENT_DATA_DEFINITION_ID, SIMCONNECT_CLIENT_DATA_PERIOD, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG, DWORD, DWORD, DWORD);
typedef HRESULT(__stdcall *PfnSimConnect_SubscribeToSystemEvent)(HANDLE, SIMCONNECT_CLIENT_EVENT_ID, const char *);
typedef HRESULT(__stdcall *PfnSimConnect_MapClientEventToSimEvent)(HANDLE, SIMCONNECT_CLIENT_EVENT_ID, const char *);
typedef HRESULT(__stdcall *PfnSimConnect_SubscribeToFacilities)(HANDLE, SIMCONNECT_FACILITY_LIST_TYPE, SIMCONNECT_DATA_REQUEST_ID);
typedef HRESULT(__stdcall *PfnSimConnect_GetLastSentPacketID)(HANDLE, DWORD *);
typedef HRESULT(__stdcall *PfnSimConnect_AIRemoveObject)(HANDLE, SIMCONNECT_OBJECT_ID, SIMCONNECT_DATA_REQUEST_ID);
typedef HRESULT(__stdcall *PfnSimConnect_SetDataOnSimObject)(HANDLE, SIMCONNECT_DATA_DEFINITION_ID, SIMCONNECT_OBJECT_ID, SIMCONNECT_DATA_SET_FLAG, DWORD, DWORD, void *);
typedef HRESULT(__stdcall *PfnSimConnect_AIReleaseControl)(HANDLE, SIMCONNECT_OBJECT_ID, SIMCONNECT_DATA_REQUEST_ID);
typedef HRESULT(__stdcall *PfnSimConnect_AICreateNonATCAircraft)(HANDLE, const char *, const char *, SIMCONNECT_DATA_INITPOSITION, SIMCONNECT_DATA_REQUEST_ID);
typedef HRESULT(__stdcall *PfnSimConnect_AICreateSimulatedObject)(HANDLE, const char *, SIMCONNECT_DATA_INITPOSITION, SIMCONNECT_DATA_REQUEST_ID);
typedef HRESULT(__stdcall *PfnSimConnect_MapClientDataNameToID)(HANDLE, const char *, SIMCONNECT_CLIENT_DATA_ID);
typedef HRESULT(__stdcall *PfnSimConnect_CreateClientData)(HANDLE, SIMCONNECT_CLIENT_DATA_ID, DWORD, SIMCONNECT_CREATE_CLIENT_DATA_FLAG);
typedef HRESULT(__stdcall *PfnSimConnect_AddToClientDataDefinition)(HANDLE, SIMCONNECT_CLIENT_DATA_DEFINITION_ID, DWORD, DWORD, float, DWORD);

//! The SimCOnnect Symbols
//! \private @{
struct SimConnectSymbols
{
    PfnSimConnect_Open SimConnect_Open = nullptr;
    PfnSimConnect_Close SimConnect_Close = nullptr;
    PfnSimConnect_AddToDataDefinition SimConnect_AddToDataDefinition = nullptr;
    PfnSimConnect_Text SimConnect_Text = nullptr;
    PfnSimConnect_CallDispatch SimConnect_CallDispatch = nullptr;
    PfnSimConnect_WeatherSetModeCustom SimConnect_WeatherSetModeCustom = nullptr;
    PfnSimConnect_WeatherSetModeGlobal SimConnect_WeatherSetModeGlobal = nullptr;
    PfnSimConnect_WeatherSetObservation SimConnect_WeatherSetObservation = nullptr;
    PfnSimConnect_TransmitClientEvent SimConnect_TransmitClientEvent = nullptr;
    PfnSimConnect_SetClientData SimConnect_SetClientData = nullptr;
    PfnSimConnect_RequestDataOnSimObject SimConnect_RequestDataOnSimObject = nullptr;
    PfnSimConnect_RequestClientData SimConnect_RequestClientData = nullptr;
    PfnSimConnect_SubscribeToSystemEvent SimConnect_SubscribeToSystemEvent = nullptr;
    PfnSimConnect_MapClientEventToSimEvent SimConnect_MapClientEventToSimEvent = nullptr;
    PfnSimConnect_SubscribeToFacilities SimConnect_SubscribeToFacilities = nullptr;
    PfnSimConnect_GetLastSentPacketID SimConnect_GetLastSentPacketID = nullptr;
    PfnSimConnect_AIRemoveObject SimConnect_AIRemoveObject = nullptr;
    PfnSimConnect_SetDataOnSimObject SimConnect_SetDataOnSimObject = nullptr;
    PfnSimConnect_AIReleaseControl SimConnect_AIReleaseControl = nullptr;
    PfnSimConnect_AICreateNonATCAircraft SimConnect_AICreateNonATCAircraft = nullptr;
    PfnSimConnect_AICreateSimulatedObject SimConnect_AICreateSimulatedObject = nullptr;
    PfnSimConnect_MapClientDataNameToID SimConnect_MapClientDataNameToID = nullptr;
    PfnSimConnect_CreateClientData SimConnect_CreateClientData = nullptr;
    PfnSimConnect_AddToClientDataDefinition SimConnect_AddToClientDataDefinition = nullptr;
};
//! @ }

SimConnectSymbols gSymbols;

QString getLastErrorMsg()
{
    LPWSTR bufPtr = nullptr;
    DWORD err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result = (bufPtr) ? QString::fromUtf16((const ushort *)bufPtr).trimmed() : QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}

bool loadAndResolveSimConnect(bool manifestProbing)
{
    QLibrary simConnectDll;
    simConnectDll.setFileName("SimConnect.dll");
    simConnectDll.setLoadHints(QLibrary::PreventUnloadHint);

    if (manifestProbing)
    {
        HMODULE hInst = nullptr;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)loadAndResolveSimConnect, &hInst);
        wchar_t szModuleName[MAX_PATH];
        GetModuleFileName(hInst, szModuleName, MAX_PATH);

        std::array<WORD, 3> resourceNumbers = {{ 101U, 102U, 103U }};

        for (const auto resourceNumber :  resourceNumbers)
        {
            ACTCTX actCtx;
            memset(&actCtx, 0, sizeof(ACTCTX));
            actCtx.cbSize = sizeof(actCtx);
            actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
            actCtx.lpSource = szModuleName;
            actCtx.lpResourceName = MAKEINTRESOURCE(resourceNumber);

            HANDLE hActCtx;
            hActCtx = CreateActCtx(&actCtx);
            if (hActCtx != INVALID_HANDLE_VALUE)
            {
                ULONG_PTR lpCookie = 0;
                if (ActivateActCtx(hActCtx, &lpCookie))
                {
                    simConnectDll.load();
                    DeactivateActCtx(0, lpCookie);
                    if (simConnectDll.isLoaded()) { break; }
                }
            }
            ReleaseActCtx(hActCtx);
        }
    }

    // Try once without activation context
    if (!simConnectDll.isLoaded())
    {
        simConnectDll.load();
    }

    if (simConnectDll.isLoaded())
    {
        gSymbols.SimConnect_Open = reinterpret_cast<PfnSimConnect_Open>(simConnectDll.resolve("SimConnect_Open"));
        gSymbols.SimConnect_Close = (PfnSimConnect_Close) simConnectDll.resolve("SimConnect_Close");

        gSymbols.SimConnect_AddToDataDefinition = reinterpret_cast<PfnSimConnect_AddToDataDefinition>(simConnectDll.resolve("SimConnect_AddToDataDefinition"));
        gSymbols.SimConnect_Text = reinterpret_cast<PfnSimConnect_Text>(simConnectDll.resolve("SimConnect_Text"));
        gSymbols.SimConnect_CallDispatch = reinterpret_cast<PfnSimConnect_CallDispatch>(simConnectDll.resolve("SimConnect_CallDispatch"));
        gSymbols.SimConnect_WeatherSetModeCustom = reinterpret_cast<PfnSimConnect_WeatherSetModeCustom>(simConnectDll.resolve("SimConnect_WeatherSetModeCustom"));
        gSymbols.SimConnect_WeatherSetModeGlobal = reinterpret_cast<PfnSimConnect_WeatherSetModeGlobal>(simConnectDll.resolve("SimConnect_WeatherSetModeGlobal"));
        gSymbols.SimConnect_WeatherSetObservation = reinterpret_cast<PfnSimConnect_WeatherSetObservation>(simConnectDll.resolve("SimConnect_WeatherSetObservation"));
        gSymbols.SimConnect_TransmitClientEvent = reinterpret_cast<PfnSimConnect_TransmitClientEvent>(simConnectDll.resolve("SimConnect_TransmitClientEvent"));
        gSymbols.SimConnect_SetClientData = reinterpret_cast<PfnSimConnect_SetClientData>(simConnectDll.resolve("SimConnect_SetClientData"));
        gSymbols.SimConnect_RequestDataOnSimObject = reinterpret_cast<PfnSimConnect_RequestDataOnSimObject>(simConnectDll.resolve("SimConnect_RequestDataOnSimObject"));
        gSymbols.SimConnect_RequestClientData = reinterpret_cast<PfnSimConnect_RequestClientData>(simConnectDll.resolve("SimConnect_RequestClientData"));
        gSymbols.SimConnect_SubscribeToSystemEvent = reinterpret_cast<PfnSimConnect_SubscribeToSystemEvent>(simConnectDll.resolve("SimConnect_SubscribeToSystemEvent"));
        gSymbols.SimConnect_MapClientEventToSimEvent = reinterpret_cast<PfnSimConnect_MapClientEventToSimEvent>(simConnectDll.resolve("SimConnect_MapClientEventToSimEvent"));
        gSymbols.SimConnect_SubscribeToFacilities = reinterpret_cast<PfnSimConnect_SubscribeToFacilities>(simConnectDll.resolve("SimConnect_SubscribeToFacilities"));
        gSymbols.SimConnect_GetLastSentPacketID = reinterpret_cast<PfnSimConnect_GetLastSentPacketID>(simConnectDll.resolve("SimConnect_GetLastSentPacketID"));
        gSymbols.SimConnect_AIRemoveObject = reinterpret_cast<PfnSimConnect_AIRemoveObject>(simConnectDll.resolve("SimConnect_AIRemoveObject"));
        gSymbols.SimConnect_SetDataOnSimObject = reinterpret_cast<PfnSimConnect_SetDataOnSimObject>(simConnectDll.resolve("SimConnect_SetDataOnSimObject"));
        gSymbols.SimConnect_AIReleaseControl = reinterpret_cast<PfnSimConnect_AIReleaseControl>(simConnectDll.resolve("SimConnect_AIReleaseControl"));
        gSymbols.SimConnect_AICreateNonATCAircraft = reinterpret_cast<PfnSimConnect_AICreateNonATCAircraft>(simConnectDll.resolve("SimConnect_AICreateNonATCAircraft"));
        gSymbols.SimConnect_AICreateSimulatedObject = reinterpret_cast<PfnSimConnect_AICreateSimulatedObject>(simConnectDll.resolve("SimConnect_AICreateSimulatedObject"));
        gSymbols.SimConnect_MapClientDataNameToID = reinterpret_cast<PfnSimConnect_MapClientDataNameToID>(simConnectDll.resolve("SimConnect_MapClientDataNameToID"));
        gSymbols.SimConnect_CreateClientData = reinterpret_cast<PfnSimConnect_CreateClientData>(simConnectDll.resolve("SimConnect_CreateClientData"));
        gSymbols.SimConnect_AddToClientDataDefinition = reinterpret_cast<PfnSimConnect_AddToClientDataDefinition>(simConnectDll.resolve("SimConnect_AddToClientDataDefinition"));
    }

    return true;
}

SIMCONNECTAPI SimConnect_Open(HANDLE *phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex)
{
    return gSymbols.SimConnect_Open(phSimConnect, szName, hWnd, UserEventWin32, hEventHandle, ConfigIndex);
}

SIMCONNECTAPI SimConnect_Close(HANDLE hSimConnect)
{
    return gSymbols.SimConnect_Close(hSimConnect);
}

SIMCONNECTAPI SimConnect_AddToDataDefinition(HANDLE hSimConnect, SIMCONNECT_DATA_DEFINITION_ID DefineID, const char *DatumName, const char *UnitsName, SIMCONNECT_DATATYPE DatumType, float fEpsilon, DWORD DatumID)
{
    return gSymbols.SimConnect_AddToDataDefinition(hSimConnect, DefineID, DatumName, UnitsName, DatumType, fEpsilon, DatumID);
}

SIMCONNECTAPI SimConnect_Text(HANDLE hSimConnect, SIMCONNECT_TEXT_TYPE type, float fTimeSeconds, SIMCONNECT_CLIENT_EVENT_ID EventID, DWORD cbUnitSize, void *pDataSet)
{
    return gSymbols.SimConnect_Text(hSimConnect, type, fTimeSeconds, EventID, cbUnitSize, pDataSet);
}

SIMCONNECTAPI SimConnect_CallDispatch(HANDLE hSimConnect, DispatchProc pfcnDispatch, void *pContext)
{
    return gSymbols.SimConnect_CallDispatch(hSimConnect, pfcnDispatch, pContext);
}

SIMCONNECTAPI SimConnect_WeatherSetModeCustom(HANDLE hSimConnect)
{
    return gSymbols.SimConnect_WeatherSetModeCustom(hSimConnect);
}

SIMCONNECTAPI SimConnect_WeatherSetModeGlobal(HANDLE hSimConnect)
{
    return gSymbols.SimConnect_WeatherSetModeGlobal(hSimConnect);
}

SIMCONNECTAPI SimConnect_WeatherSetObservation(HANDLE hSimConnect, DWORD Seconds, const char *szMETAR)
{
    return gSymbols.SimConnect_WeatherSetObservation(hSimConnect, Seconds, szMETAR);
}

SIMCONNECTAPI SimConnect_TransmitClientEvent(HANDLE hSimConnect, SIMCONNECT_OBJECT_ID ObjectID, SIMCONNECT_CLIENT_EVENT_ID EventID, DWORD dwData, SIMCONNECT_NOTIFICATION_GROUP_ID GroupID, SIMCONNECT_EVENT_FLAG Flags)
{
    return gSymbols.SimConnect_TransmitClientEvent(hSimConnect, ObjectID, EventID, dwData, GroupID, Flags);
}

SIMCONNECTAPI SimConnect_SetClientData(HANDLE hSimConnect, SIMCONNECT_CLIENT_DATA_ID ClientDataID, SIMCONNECT_CLIENT_DATA_DEFINITION_ID DefineID, SIMCONNECT_CLIENT_DATA_SET_FLAG Flags, DWORD dwReserved, DWORD cbUnitSize, void *pDataSet)
{
    return gSymbols.SimConnect_SetClientData(hSimConnect, ClientDataID, DefineID, Flags, dwReserved, cbUnitSize, pDataSet);
}

SIMCONNECTAPI SimConnect_RequestDataOnSimObject(HANDLE hSimConnect, SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_DATA_DEFINITION_ID DefineID, SIMCONNECT_OBJECT_ID ObjectID, SIMCONNECT_PERIOD Period, SIMCONNECT_DATA_REQUEST_FLAG Flags, DWORD origin, DWORD interval, DWORD limit)
{
    return gSymbols.SimConnect_RequestDataOnSimObject(hSimConnect, RequestID, DefineID, ObjectID, Period, Flags, origin, interval, limit);
}

SIMCONNECTAPI SimConnect_RequestClientData(HANDLE hSimConnect, SIMCONNECT_CLIENT_DATA_ID ClientDataID, SIMCONNECT_DATA_REQUEST_ID RequestID, SIMCONNECT_CLIENT_DATA_DEFINITION_ID DefineID, SIMCONNECT_CLIENT_DATA_PERIOD Period, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG Flags, DWORD origin, DWORD interval, DWORD limit)
{
    return gSymbols.SimConnect_RequestClientData(hSimConnect, ClientDataID, RequestID, DefineID, Period, Flags, origin, interval, limit);
}

SIMCONNECTAPI SimConnect_SubscribeToSystemEvent(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char *SystemEventName)
{
    return gSymbols.SimConnect_SubscribeToSystemEvent(hSimConnect, EventID, SystemEventName);
}

SIMCONNECTAPI SimConnect_MapClientEventToSimEvent(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char *EventName)
{
    return gSymbols.SimConnect_MapClientEventToSimEvent(hSimConnect, EventID, EventName);
}

SIMCONNECTAPI SimConnect_SubscribeToFacilities(HANDLE hSimConnect, SIMCONNECT_FACILITY_LIST_TYPE type, SIMCONNECT_DATA_REQUEST_ID RequestID)
{
    return gSymbols.SimConnect_SubscribeToFacilities(hSimConnect, type, RequestID);
}

SIMCONNECTAPI SimConnect_GetLastSentPacketID(HANDLE hSimConnect, DWORD *pdwError)
{
    return gSymbols.SimConnect_GetLastSentPacketID(hSimConnect, pdwError);
}

SIMCONNECTAPI SimConnect_AIRemoveObject(HANDLE hSimConnect, SIMCONNECT_OBJECT_ID ObjectID, SIMCONNECT_DATA_REQUEST_ID RequestID)
{
    return gSymbols.SimConnect_AIRemoveObject(hSimConnect, ObjectID, RequestID);
}

SIMCONNECTAPI SimConnect_SetDataOnSimObject(HANDLE hSimConnect, SIMCONNECT_DATA_DEFINITION_ID DefineID, SIMCONNECT_OBJECT_ID ObjectID, SIMCONNECT_DATA_SET_FLAG Flags, DWORD ArrayCount, DWORD cbUnitSize, void *pDataSet)
{
    return gSymbols.SimConnect_SetDataOnSimObject(hSimConnect, DefineID, ObjectID, Flags, ArrayCount, cbUnitSize, pDataSet);
}

SIMCONNECTAPI SimConnect_AIReleaseControl(HANDLE hSimConnect, SIMCONNECT_OBJECT_ID ObjectID, SIMCONNECT_DATA_REQUEST_ID RequestID)
{
    return gSymbols.SimConnect_AIReleaseControl(hSimConnect, ObjectID, RequestID);
}

SIMCONNECTAPI SimConnect_AICreateNonATCAircraft(HANDLE hSimConnect, const char *szContainerTitle, const char *szTailNumber, SIMCONNECT_DATA_INITPOSITION InitPos, SIMCONNECT_DATA_REQUEST_ID RequestID)
{
    return gSymbols.SimConnect_AICreateNonATCAircraft(hSimConnect, szContainerTitle, szTailNumber, InitPos, RequestID);
}

SIMCONNECTAPI SimConnect_AICreateSimulatedObject(HANDLE hSimConnect, const char *szContainerTitle, SIMCONNECT_DATA_INITPOSITION InitPos, SIMCONNECT_DATA_REQUEST_ID RequestID)
{
    return gSymbols.SimConnect_AICreateSimulatedObject(hSimConnect, szContainerTitle, InitPos, RequestID);
}

SIMCONNECTAPI SimConnect_MapClientDataNameToID(HANDLE hSimConnect, const char *szClientDataName, SIMCONNECT_CLIENT_DATA_ID ClientDataID)
{
    return gSymbols.SimConnect_MapClientDataNameToID(hSimConnect, szClientDataName, ClientDataID);
}

SIMCONNECTAPI SimConnect_CreateClientData(HANDLE hSimConnect, SIMCONNECT_CLIENT_DATA_ID ClientDataID, DWORD dwSize, SIMCONNECT_CREATE_CLIENT_DATA_FLAG Flags)
{
    return gSymbols.SimConnect_CreateClientData(hSimConnect, ClientDataID, dwSize, Flags);
}

SIMCONNECTAPI SimConnect_AddToClientDataDefinition(HANDLE hSimConnect, SIMCONNECT_CLIENT_DATA_DEFINITION_ID DefineID, DWORD dwOffset, DWORD dwSizeOrType, float fEpsilon, DWORD DatumID)
{
    return gSymbols.SimConnect_AddToClientDataDefinition(hSimConnect, DefineID, dwOffset, dwSizeOrType, fEpsilon, DatumID);
}

#endif

