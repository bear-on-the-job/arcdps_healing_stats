#include "arcdps_structs.h"
#include "Exports.h"
#include "GUI.h"
#include "Log.h"
#include "PlayerStats.h"
#include "Utilities.h"

#include "imgui.h"

#include <atomic>

#include <assert.h>
#include <d3d9helper.h>
#include <stdint.h>
#include <stdio.h>
#include <Windows.h>

/* proto/globals */
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_imgui(uint32_t pNotCharselOrLoading);
uintptr_t mod_options_end();
uintptr_t mod_combat(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision);
uintptr_t mod_combat_local(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision);
uintptr_t mod_wnd(HWND pWindowHandle, UINT pMessage, WPARAM pAdditionalW, LPARAM pAdditionalL);

uintptr_t ProcessLocalEvent(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision);
void ProcessPeerEvent(cbtevent* pEvent, uint16_t pPeerInstanceId);

static MallocSignature ARCDPS_MALLOC = nullptr;
static FreeSignature ARCDPS_FREE = nullptr;
static arcdps_exports ARC_EXPORTS;
static const char* ARCDPS_VERSION;

std::mutex HEAL_TABLE_OPTIONS_MUTEX;
static HealTableOptions HEAL_TABLE_OPTIONS;

static void* MallocWrapper(size_t pSize, void* pUserData)
{
	return ARCDPS_MALLOC(pSize);
}

static void FreeWrapper(void* pPointer, void* pUserData)
{
	ARCDPS_FREE(pPointer);
}

/* export -- arcdps looks for this exported function and calls the address it returns on client load */
extern "C" __declspec(dllexport) ModInitSignature get_init_addr(const char* pArcdpsVersionString, void* pImguiContext, IDirect3DDevice9* pUnused, HMODULE pArcModule , MallocSignature pArcdpsMalloc, FreeSignature pArcdpsFree)
{
	GlobalObjects::ARC_E3 = reinterpret_cast<E3Signature>(GetProcAddress(pArcModule, "e3"));
	assert(GlobalObjects::ARC_E3 != nullptr);
	GlobalObjects::ARC_E7 = reinterpret_cast<E7Signature>(GetProcAddress(pArcModule, "e7"));
	assert(GlobalObjects::ARC_E7 != nullptr);

	ARCDPS_VERSION = pArcdpsVersionString;
	SetContext(pImguiContext);

	ARCDPS_MALLOC = pArcdpsMalloc;
	ARCDPS_FREE = pArcdpsFree;
	ImGui::SetAllocatorFunctions(MallocWrapper, FreeWrapper);

	GlobalObjects::EVENT_SEQUENCER = std::make_unique<EventSequencer>(ProcessLocalEvent);
	GlobalObjects::EVENT_PROCESSOR = std::make_unique<EventProcessor>();
	GlobalObjects::EVTC_RPC_CLIENT = std::make_unique<evtc_rpc_client>("", ProcessPeerEvent);
	return mod_init;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client exit */
extern "C" __declspec(dllexport) ModReleaseSignature get_release_addr()
{
	ARCDPS_VERSION = nullptr;
	return mod_release;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init()
{
#ifdef DEBUG
	AllocConsole();
	SetConsoleOutputCP(CP_UTF8);

	/* big buffer */
	char buff[4096];
	char* p = &buff[0];
	p += _snprintf(p, 400, "==== mod_init ====\n");
	p += _snprintf(p, 400, "arcdps: %s\n", ARCDPS_VERSION);

	/* print */
	DWORD written = 0;
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(hnd, &buff[0], (DWORD)(p - &buff[0]), &written, 0);
#endif // DEBUG

	{
		std::lock_guard lock(HEAL_TABLE_OPTIONS_MUTEX);
		ReadIni(HEAL_TABLE_OPTIONS);
	}

	memset(&ARC_EXPORTS, 0, sizeof(arcdps_exports));
	ARC_EXPORTS.sig = 0x9c9b3c99;
	ARC_EXPORTS.imguivers = IMGUI_VERSION_NUM;
	ARC_EXPORTS.size = sizeof(arcdps_exports);
	ARC_EXPORTS.out_name = "healing_stats";
	ARC_EXPORTS.out_build = "1.3rc2";
	ARC_EXPORTS.combat = mod_combat;
	ARC_EXPORTS.imgui = mod_imgui;
	ARC_EXPORTS.options_end = mod_options_end;
	ARC_EXPORTS.combat_local = mod_combat_local;
	ARC_EXPORTS.wnd_nofilter = mod_wnd;
	return &ARC_EXPORTS;
}

/* release mod -- return ignored */
uintptr_t mod_release()
{
	{
		std::lock_guard lock(HEAL_TABLE_OPTIONS_MUTEX);
		WriteIni(HEAL_TABLE_OPTIONS);
	}

#ifdef DEBUG
	FreeConsole();
#endif

	return 0;
}

uintptr_t mod_imgui(uint32_t pNotCharSelectionOrLoading)
{
	if (pNotCharSelectionOrLoading == 0)
	{
		return 0;
	}

	{
		std::lock_guard lock(HEAL_TABLE_OPTIONS_MUTEX);
		Display_GUI(HEAL_TABLE_OPTIONS);
	}

	return 0;
}

uintptr_t mod_options_end()
{
	{
		std::lock_guard lock(HEAL_TABLE_OPTIONS_MUTEX);
		Display_ArcDpsOptions(HEAL_TABLE_OPTIONS);
	}

	return 0;
}

static std::atomic<uint32_t> SELF_INSTANCE_ID = UINT32_MAX;
/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision)
{
	GlobalObjects::EVENT_PROCESSOR->AreaCombat(pEvent, pSourceAgent, pDestinationAgent, pSkillname, pId, pRevision);
	return 0;
}

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat_local(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision)
{
	GlobalObjects::EVENT_SEQUENCER->ProcessEvent(pEvent, pSourceAgent, pDestinationAgent, pSkillname, pId, pRevision);
	return 0;
}

uintptr_t ProcessLocalEvent(cbtevent* pEvent, ag* pSourceAgent, ag* pDestinationAgent, const char* pSkillname, uint64_t pId, uint64_t pRevision)
{
	GlobalObjects::EVENT_PROCESSOR->LocalCombat(pEvent, pSourceAgent, pDestinationAgent, pSkillname, pId, pRevision);
	// Send to client
	return 0;
}

void ProcessPeerEvent(cbtevent* pEvent, uint16_t pPeerInstanceId)
{
	GlobalObjects::EVENT_PROCESSOR->PeerCombat(pEvent, pPeerInstanceId);
}

#pragma pack(push, 1)
struct ArcModifiers
{
	uint16_t _1;
	uint16_t _2;
	uint16_t Multi;
};
#pragma pack(pop)

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND pWindowHandle, UINT pMessage, WPARAM pAdditionalW, LPARAM pAdditionalL)
{
	ImGui_ProcessKeyEvent(pWindowHandle, pMessage, pAdditionalW, pAdditionalL);

	const ImGuiIO& io = ImGui::GetIO();

	if (pMessage == WM_KEYDOWN || pMessage == WM_SYSKEYDOWN)
	{
		int virtualKey = static_cast<int>(pAdditionalW);

		uint64_t e7_rawResult = GlobalObjects::ARC_E7();
		ArcModifiers modifiers;
		memcpy(&modifiers, &e7_rawResult, sizeof(modifiers));

		if ((modifiers._1 == 0 || io.KeysDown[modifiers._1] == true) &&
			(modifiers._2 == 0 || io.KeysDown[modifiers._2] == true))
		{
			std::lock_guard lock(HEAL_TABLE_OPTIONS_MUTEX);

			bool triggeredKey = false;
			for (uint32_t i = 0; i < HEAL_WINDOW_COUNT; i++)
			{
				if (HEAL_TABLE_OPTIONS.Windows[i].Hotkey > 0 &&
					HEAL_TABLE_OPTIONS.Windows[i].Hotkey < sizeof(io.KeysDown) &&
					virtualKey == HEAL_TABLE_OPTIONS.Windows[i].Hotkey)
				{
					assert(io.KeysDown[HEAL_TABLE_OPTIONS.Windows[i].Hotkey] == true);

					HEAL_TABLE_OPTIONS.Windows[i].Shown = !HEAL_TABLE_OPTIONS.Windows[i].Shown;
					triggeredKey = true;

					LOG("Key %i '%s' toggled window %u - new heal window state is %s", HEAL_TABLE_OPTIONS.Windows[i].Hotkey, VirtualKeyToString(HEAL_TABLE_OPTIONS.Windows[i].Hotkey).c_str(), i, BOOL_STR(HEAL_TABLE_OPTIONS.Windows[i].Shown));
				}
			}

			if (triggeredKey == true)
			{
				return 0; // Don't process message by arcdps or game
			}
		}
	}

	return pMessage;
}