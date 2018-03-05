#include "stdafx.h"
#include "stdio.h"
#include <windows.h>
#include "includes\injector\injector.hpp"
#include <cstdint>
#include "includes\IniReader.h"

float LeftGroupX, LeftGroupY, RightGroupX, RightGroupY;

void(__cdecl *FE_Object_SetCenter)(DWORD* FEObject, float _PositionX, float _PositionY) = (void(__cdecl*)(DWORD*, float, float))0x597A70;
void(__cdecl *FE_Object_GetCenter)(DWORD* FEObject, float *PositionX, float *PositionY) = (void(__cdecl*)(DWORD*, float*, float*))0x597900;
void*(__cdecl *FEObject_FindObject)(const char *pkg_name, unsigned int obj_hash) = (void*(__cdecl*)(const char*, unsigned int))0x5A0250;

int __stdcall cFEng_QueuePackageMessage_Hook(unsigned int MessageHash, char const *FEPackageName, DWORD* FEObject)
{
	int ResX, ResY;
	float Difference;

	ResX = *(int*)0xAAF548;
	ResY = *(int*)0xAAF54C;
	
	if (ResX != 0 && ResY != 0) Difference = (((float)ResX / (float)ResY) * 240) - 320; // Calculate position difference for current aspect ratio
	else Difference = 0; // 4:3 if we can't get any values

	injector::WriteMemory<float*>(0x5D52FB, &Difference, true);
	injector::WriteMemory<float*>(0x5D5358, &Difference, true);

	// PiP UCAP Position formula
	injector::WriteMemory<float>(0x750DF5, ((((320.0f + Difference) / 320.0f) - 1.042f) / -2.0f), true);

	DWORD* LeftGroup = (DWORD*)FEObject_FindObject(FEPackageName, 0x1603009E); // "HUD_SingleRace.fng", leftgrouphash

	if (LeftGroup) // Move left group
	{
		FE_Object_GetCenter(LeftGroup, &LeftGroupX, &LeftGroupY);
		FE_Object_SetCenter(LeftGroup, LeftGroupX - Difference, LeftGroupY);
	}

	DWORD* RightGroup = (DWORD*)FEObject_FindObject(FEPackageName, 0x5D0101F1); // "HUD_SingleRace.fng", rightgrouphash

	if (RightGroup) // Move right group
	{
		FE_Object_GetCenter(RightGroup, &RightGroupX, &RightGroupY);
		FE_Object_SetCenter(RightGroup, RightGroupX + Difference, RightGroupY);
	}

	return 1;
}

void Init()
{
	injector::MakeCALL(0x5D52D8, cFEng_QueuePackageMessage_Hook, true);
	injector::MakeCALL(0x5D5339, cFEng_QueuePackageMessage_Hook, true);
}

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID /*lpReserved*/)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}
	return TRUE;

}

