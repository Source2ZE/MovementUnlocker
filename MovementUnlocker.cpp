#include <stdio.h>
#include "MovementUnlocker.h"
#include <cstdint>
#include <sh_memory.h>
#ifdef _WIN32
#include <Windows.h>
#endif

MovementUnlocker g_MovementUnlocker;

#ifdef _WIN32
const unsigned char *pPatchSignature = (unsigned char *)"\x76\x58\xF3\x0F\x10\x40\x44\xF3\x0F\x10\x50\x40";
const char *pPatchPattern = "x?xxxxxxxxxx";
#elif __linux__
unsigned char * pPatchSignature = (unsigned char *)"\x76\x45\xF3\x0F\x11\x9D\x4C\xFF\xFF\xFF";
const char* pPatchPattern = "x?xxxxxxxx";
#endif

// From https://git.botox.bz/CSSZombieEscape/sm-ext-PhysHooks
uintptr_t FindPattern(uintptr_t BaseAddr, const unsigned char* pData, const char* pPattern, size_t MaxSize, bool Reverse)
{
	unsigned char* pMemory;
	uintptr_t PatternLen = strlen(pPattern);

	pMemory = reinterpret_cast<unsigned char*>(BaseAddr);

	if (!Reverse)
		for (uintptr_t i = 0; i < MaxSize; i++)
		{
			uintptr_t Matches = 0;
			while (*(pMemory + i + Matches) == pData[Matches] || pPattern[Matches] != 'x')
			{
				Matches++;
				if (Matches == PatternLen)
					return (uintptr_t)(pMemory + i);
			}
		}
	else
		for (uintptr_t i = 0; i < MaxSize; i++)
		{
			uintptr_t Matches = 0;
			while (*(pMemory - i + Matches) == pData[Matches] || pPattern[Matches] != 'x')
			{
				Matches++;
				if (Matches == PatternLen)
					return (uintptr_t)(pMemory - i);
			}
		}

	return 0x00;
}

PLUGIN_EXPOSE(MovementUnlocker, g_MovementUnlocker);
bool MovementUnlocker::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	int PatchLen = strlen(pPatchPattern);
#ifdef _WIN32
	char *pBinPath = "csgo/bin/server.dll";
	auto *pBin = LoadLibrary(pBinPath);
#elif __linux__
	char * pBinPath = "csgo/bin/server.so";
	auto *pBin = dlopen(pBinPath, RTLD_NOW);
#endif

	if (!pBin)
	{
		snprintf(error, maxlen, "Could not open %s", pBinPath);
		return false;
	}

#ifdef _WIN32
	uintptr_t pPatchAddress = (uintptr_t)GetProcAddress(pBin, "CreateInterface");
#elif __linux__
	uintptr_t pPatchAddress = (uintptr_t)dlsym(pBin, "CreateInterface");
#endif

	pPatchAddress = FindPattern(pPatchAddress, pPatchSignature, pPatchPattern, ULLONG_MAX, true);

	if (!pPatchAddress)
	{
		snprintf(error, maxlen, "Could not find WalkMove patch signature!");
		return false;
	}

	SourceHook::SetMemAccess((void*)pPatchAddress, PatchLen, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);
	*(unsigned char*)(pPatchAddress) = ((unsigned char*)"\xEB")[0];
	SourceHook::SetMemAccess((void*)pPatchAddress, PatchLen, SH_MEM_READ | SH_MEM_EXEC);

	return true;
}

bool MovementUnlocker::Unload(char *error, size_t maxlen)
{
	return true;
}

void MovementUnlocker::AllPluginsLoaded()
{
}

bool MovementUnlocker::Pause(char *error, size_t maxlen)
{
	return true;
}

bool MovementUnlocker::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *MovementUnlocker::GetLicense()
{
	return "GNU General Public License v3.0";
}

const char *MovementUnlocker::GetVersion()
{
	return "1.0";
}

const char *MovementUnlocker::GetDate()
{
	return __DATE__;
}

const char *MovementUnlocker::GetLogTag()
{
	return "MOVEMENTUNLOCKER";
}

const char *MovementUnlocker::GetAuthor()
{
	return "Vauff";
}

const char *MovementUnlocker::GetDescription()
{
	return "CS2 MM:S port of Movement Unlocker, removes max speed limitation from players on the ground.";
}

const char *MovementUnlocker::GetName()
{
	return "Movement Unlocker";
}

const char *MovementUnlocker::GetURL()
{
	return "https://github.com/Source2ZE/MovementUnlocker";
}
