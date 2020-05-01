/*
 * ============================================================================
 *
 *  [CS:GO] Weapon Stickers.
 *  Copyright (C) 2020 - Bruno "quasemago" Ronning <brunoronningfn@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, per version 3 of the License, or
 *  any later version.	
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
*/

/**
 * Includes.
 */
#include <sourcemod>
#include <sdktools>
#include <multicolors>
#include <PTaH>

#undef REQUIRE_EXTENSIONS
#include <sourcescramble>
#define REQUIRE_EXTENSIONS

#pragma semicolon 1
#pragma newdecls required

/**
 * Sub.
 */
bool g_isLateLoad = false;

#include "quasemago/csgo_weaponstickers/globals.inc"
#include "quasemago/csgo_weaponstickers/helpers.inc"
#include "quasemago/csgo_weaponstickers/commands.inc"
#include "quasemago/csgo_weaponstickers/menus.inc"
#include "quasemago/csgo_weaponstickers/database.inc"
#include "quasemago/csgo_weaponstickers/api.inc"

/**
 * Plugin Init.
 */
public Plugin myinfo = 
{
	name = "[CS:GO] Weapon Stickers",
	author = "quasemago",
	description = "Stickers for Weapons",
	version = PLUGIN_VERSION,
	url = "https://github.com/quasemago"
};

public void OnPluginStart()
{
	if (GetEngineVersion() != Engine_CSGO)
	{
		SetFailState("Only CS:GO support!");
		return;
	}

	// Translations.
	LoadTranslations("common.phrases");
	LoadTranslations("csgo_weaponstickers.phrases");

	// ConVars.
	CreateConVar("sm_weaponstickers_version", PLUGIN_VERSION, "Versão do Plugin", FCVAR_NOTIFY|FCVAR_SPONLY|FCVAR_DONTRECORD);
	g_cvarEnabled = CreateConVar("sm_weaponstickers_enabled", "1", "Ativa ou desativa o plugin Weapon Stickers.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_cvarUpdateViewModel = CreateConVar("sm_weaponstickers_updateviewmodel", "1", "Especifica se o view model vai ser atualizado ao alterar os stickers (P.S: o jogador irá sentir um pequeno rollback).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_cvarReuseTime = CreateConVar("sm_weaponstickers_reusetime", "5", "Especifica quantos segundos vai ser necessario esperar para atualizar novamente o inventário.", FCVAR_NOTIFY, true, 0.1);

	AutoExecConfig(true, "csgo_weaponstickers");
	CSetPrefix("{red}[Weapon Stickers]{default}");

	// Forward event to modules.
	LoadSDK();
	LoadConfigs();
	LoadCommands();
	LoadDatabase();

	// Hooks.
	PTaH(PTaH_GiveNamedItemPre, Hook, OnGiveNamedItemPre);
	PTaH(PTaH_GiveNamedItemPost, Hook, OnGiveNamedItemPost);
	
	// Late Load.
	if (g_isLateLoad)
	{
		for (int i = 1; i <= MaxClients; i++)
		{
			if (!IsClientInGame(i) || IsFakeClient(i))
			{
				continue;
			}

			OnClientPostAdminCheck(i);
		}
	}
}

public APLRes AskPluginLoad2(Handle plugin, bool late, char[] error, int errMax)
{
	g_isLateLoad = late;

	/* API */
	LoadAPI();

	/* External Natives */
	MarkNativeAsOptional("MemoryBlock.MemoryBlock");
	MarkNativeAsOptional("MemoryBlock.Address.get");

	/* Library */
	RegPluginLibrary("csgo_weaponstickers");
	return APLRes_Success;
}

/**
 * Forwards.
 */
public void OnClientPostAdminCheck(int client)
{
	if (IsFakeClient(client) || !g_cvarEnabled.BoolValue)
	{
		return;
	}

	LoadClientData(client);
}

public void OnClientDisconnect(int client)
{
	g_playerReuseTime[client] = 0;
	g_isStickerRefresh[client] = false;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		for (int j = 0; j < MAX_STICKERS_SLOT; j++)
		{
			g_PlayerWeapon[client][i].m_stickerIndex[j] = 0;
		}
	}

	// Forward event to modules.
	MenusClientDisconnect(client);
}

/**
 * Events & Hooks.
 */
public Action OnGiveNamedItemPre(int client, char classname[64], CEconItemView &item, bool &ignoredCEconItemView, bool &isOriginNULL, float origin[3])
{
	if (!g_cvarEnabled.BoolValue)
	{
		return Plugin_Continue;
	}

	if (IsClientInGame(client) && !IsFakeClient(client))
	{
		if (IsKnifeClassname(classname))
		{
			return Plugin_Continue;
		}

		int index = GetWeaponIndexByClassname(classname);
		if (index != -1 && ClientWeaponHasStickers(client, index))
		{
			ignoredCEconItemView = true;
			return Plugin_Changed;
		}
	}
	return Plugin_Continue;
}

public void OnGiveNamedItemPost(int client, const char[] classname, const CEconItemView item, int entity, bool isOriginNULL, const float origin[3])
{
	if (!g_cvarEnabled.BoolValue)
	{
		return;
	}

	if (IsClientInGame(client) && !IsFakeClient(client) && IsValidEntity(entity))
	{
		int index = GetWeaponIndexByClassname(classname);
		if (index != -1)
		{
			if (ClientWeaponHasStickers(client, index) && IsValidWeaponToChange(-1, g_Weapons[index].m_defIndex, _, true))
			{
				// Init custom weapon.
				static int IDHigh = 16384;
				SetEntProp(entity, Prop_Send, "m_iItemIDLow", -1);
				SetEntProp(entity, Prop_Send, "m_iItemIDHigh", IDHigh++);
				SetEntProp(entity, Prop_Send, "m_iAccountID", GetSteamAccountID(client, true));
				SetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity", client);
				SetEntPropEnt(entity, Prop_Send, "m_hPrevOwner", -1);

				// Change stickers.
				Address pWeapon = GetEntityAddress(entity);
				if (pWeapon == Address_Null)
				{
					CPrintToChat(client, "%t", "Unknown Error");
					return;
				}

				Address pEconItemView = pWeapon + view_as<Address>(g_EconItemOffset);

				bool updated = false;
				for (int i = 0; i < MAX_STICKERS_SLOT; i++)
				{
					if (g_PlayerWeapon[client][index].m_stickerIndex[i] != 0)
					{
						// Sticker updated.
						updated = true;

						// TODO: add Scale and Rotation.
						SetAttributeValue(client, pEconItemView, g_PlayerWeapon[client][index].m_stickerIndex[i], "sticker slot %i id", i);
						SetAttributeValue(client, pEconItemView, view_as<int>(0.0), "sticker slot %i wear", i);
					}
				}

				// Update viewmodel if enabled.
				if (g_isStickerRefresh[client] && updated)
				{
					g_isStickerRefresh[client] = false;

					if (g_cvarUpdateViewModel.BoolValue)
					{
						PTaH_ForceFullUpdate(client);
					}
				}
			}
		}
	}
}

/**
 * Functions.
 */
void LoadSDK()
{	
	Handle gameConf = LoadGameConfigFile("csgo_weaponstickers.games");

	if (gameConf == null)
	{
		SetFailState("Game config was not loaded right.");
		return;
	}

	// Get Server Platform.
	g_ServerPlatform = view_as<ServerPlatform>(GameConfGetOffset(gameConf, "ServerPlatform"));
	if (g_ServerPlatform == OS_Mac || g_ServerPlatform == OS_Unknown)
	{
		SetFailState("Only Linux/Windows support!");
		return;
	}

	// Setup CEconItem::GetItemDefinition.
	StartPrepSDKCall(SDKCall_Raw);
	PrepSDKCall_SetFromConf(gameConf, SDKConf_Virtual, "CEconItem_GetItemDefinition");
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);

	if (!(g_SDKGetItemDefinition = EndPrepSDKCall()))
	{
		SetFailState("Method \"CEconItem::GetItemDefinition\" was not loaded right.");
		return;
	}

	// Setup CEconItemDefinition::GetNumSupportedStickerSlots.
	StartPrepSDKCall(SDKCall_Raw);
	PrepSDKCall_SetFromConf(gameConf, SDKConf_Virtual, "CEconItemDefinition_GetNumSupportedStickerSlots");
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);

	if (!(g_SDKGetNumSupportedStickerSlots = EndPrepSDKCall()))
	{
		SetFailState("Method \"CEconItemDefinition::GetNumSupportedStickerSlots\" was not loaded right.");
		return;
	}

	// Setup ItemSystem.
	StartPrepSDKCall(SDKCall_Static);
	PrepSDKCall_SetFromConf(gameConf, SDKConf_Signature, "ItemSystem");
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);

	Handle SDKItemSystem;
	if (!(SDKItemSystem = EndPrepSDKCall()))
	{
		SetFailState("Method \"ItemSystem\" was not loaded right.");
		return;
	}

	g_pItemSystem = SDKCall(SDKItemSystem);
	if (g_pItemSystem == Address_Null)
	{
		SetFailState("Failed to get \"ItemSystem\" pointer address.");
		return;
	}

	delete SDKItemSystem;
	g_pItemSchema = g_pItemSystem + view_as<Address>(4);

	// Setup CAttributeList::AddAttribute.
	StartPrepSDKCall(SDKCall_Raw);
	PrepSDKCall_SetFromConf(gameConf, SDKConf_Signature, "CAttributeList_AddAttribute");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);

	if (g_ServerPlatform == OS_Windows)
	{
		PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
		PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	}

	if (!(g_SDKAddAttribute = EndPrepSDKCall()))
	{
		SetFailState("Method \"CAttributeList::AddAttribute\" was not loaded right.");
		return;
	}

	// Linux only.
	if (g_ServerPlatform == OS_Linux)
	{
		// Setup CEconItemSystem::GenerateAttribute.
		StartPrepSDKCall(SDKCall_Raw);
		PrepSDKCall_SetFromConf(gameConf, SDKConf_Signature, "CEconItemSystem_GenerateAttribute");
		PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
		PrepSDKCall_AddParameter(SDKType_Float, SDKPass_Plain);
		PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);

		if (!(g_SDKGenerateAttribute = EndPrepSDKCall()))
		{
			SetFailState("Method \"CEconItemSystem::GenerateAttribute\" was not loaded right.");
			return;
		}
	}

	// Setup CEconItemSchema::GetAttributeDefinitionByName.
	StartPrepSDKCall(SDKCall_Raw);
	PrepSDKCall_SetFromConf(gameConf, SDKConf_Signature, "CEconItemSchema_GetAttributeDefinitionByName");
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);

	if (!(g_SDKGetAttributeDefinitionByName = EndPrepSDKCall()))
	{
		SetFailState("Method \"CEconItemSchema::GetAttributeDefinitionByName\" was not loaded right.");
		return;
	}

	// Get Offsets.
	FindGameConfOffset(gameConf, g_EconItemView_AttributeListOffset, "CEconItemView::m_AttributeList");
	FindGameConfOffset(gameConf, g_EconItemAttributeDefinition_iAttributeDefinitionIndexOffset, "CEconItemAttributeDefinition::m_iAttributeDefinitionIndex");
	FindGameConfOffset(gameConf, g_Attributes_iAttributeDefinitionIndexOffset, "m_Attributes::m_iAttributeDefinitionIndex");
	FindGameConfOffset(gameConf, g_Attributes_iRawValue32Offset, "m_Attributes::m_iRawValue32");
	FindGameConfOffset(gameConf, g_Attributes_iRawInitialValue32Offset, "m_Attributes::m_iRawInitialValue32");
	FindGameConfOffset(gameConf, g_Attributes_nRefundableCurrencyOffset, "m_Attributes::m_nRefundableCurrency");
	FindGameConfOffset(gameConf, g_Attributes_bSetBonusOffset, "m_Attributes::m_bSetBonus");
	FindGameConfOffset(gameConf, g_AttributeList_ReadOffset, "CAttributeList::read");
	FindGameConfOffset(gameConf, g_AttributeList_CountOffset, "CAttributeList::count");

	delete gameConf;

	// Find netprops Offsets.
	g_EconItemOffset = FindSendPropOffset("CBaseCombatWeapon", "m_Item");
}

void LoadConfigs()
{
	static char config[PLATFORM_MAX_PATH];

	// Parse Weapons.
	BuildPath(Path_SM, config, sizeof(config), CONFIG_PATH_WEAPONS);
	KeyValues kvWeapons = new KeyValues("Weapons");

	if (!kvWeapons.ImportFromFile(config))
	{
		ThrowError("Can't find or read the file %s...", config);
		delete kvWeapons;
		return;
	}

	g_WeaponDefIndex = new StringMap();
	g_WeaponClassName = new StringMap();

	kvWeapons.Rewind();
	int weapons = 0; // weapons counter.

	if (kvWeapons.GotoFirstSubKey())
	{
		do
		{
			static char nameKey[MAX_LENGTH_DISPLAY];
			static char indexKey[MAX_LENGTH_INDEX];
			static char classKey[MAX_LENGTH_CLASSNAME];

			kvWeapons.GetSectionName(classKey, sizeof(classKey));
			kvWeapons.GetString("index", indexKey, sizeof(indexKey));
			kvWeapons.GetString("name", nameKey, sizeof(nameKey));

			g_Weapons[weapons].m_defIndex = StringToInt(indexKey);
			strcopy(g_Weapons[weapons].m_displayName, MAX_LENGTH_DISPLAY, nameKey);
			strcopy(g_Weapons[weapons].m_className, MAX_LENGTH_CLASSNAME, classKey);
			g_Weapons[weapons].m_slot = kvWeapons.GetNum("slot", -1);

			g_WeaponDefIndex.SetValue(indexKey, weapons);
			g_WeaponClassName.SetValue(classKey, weapons);
			weapons++;
		}
		while (kvWeapons.GotoNextKey());
	}

	delete kvWeapons;
	LogMessage("Loaded %d weapons.", weapons);

	// Parse Stickers.
	BuildPath(Path_SM, config, sizeof(config), CONFIG_PATH_STICKERS);
	KeyValues kvStickers = new KeyValues("Stickers");

	if (!kvStickers.ImportFromFile(config))
	{
		ThrowError("Can't find or read the file %s...", config);
		delete kvStickers;
		return;
	}

	kvStickers.Rewind();
	int sets = 0; // stickers sets counter.
	int stickers = 0;

	if (kvStickers.GotoFirstSubKey())
	{
		int k = 0; // stickers counter.
		do
		{
			static char nameKey[MAX_LENGTH_DISPLAY];
			kvStickers.GetSectionName(nameKey, sizeof(nameKey));

			g_StickerSets[sets].m_defIndex = kvStickers.GetNum("id");
			strcopy(g_StickerSets[sets].m_displayName, MAX_LENGTH_DISPLAY, nameKey);

			if (kvStickers.GotoFirstSubKey())
			{
				do
				{
					kvStickers.GetSectionName(nameKey, sizeof(nameKey));
					g_Sticker[sets][k].m_defIndex = kvStickers.GetNum("index");
					strcopy(g_Sticker[sets][k].m_displayName, MAX_LENGTH_DISPLAY, nameKey);
					k++;
					stickers++;
				} while (kvStickers.GotoNextKey());
			}

			kvStickers.GoBack();
			g_stickerCount[sets] = k;

			sets++; // increment sticker sets counter.
			k = 0; // reset stickers counter.
		}
		while (kvStickers.GotoNextKey());
	}

	delete kvStickers;

	g_stickerSetsCount = sets;
	LogMessage("Loaded %d stickers sets [%i stickers].", sets, stickers);
}

void RefreshClientWeapon(int client, int index)
{
	// Invalid index or knife.
	if (index < 0 || g_Weapons[index].m_slot == 2)
	{
		return;
	}

	int size = GetEntPropArraySize(client, Prop_Send, "m_hMyWeapons");
	for (int i = 0; i < size; i++)
	{
		int weapon = GetEntPropEnt(client, Prop_Send, "m_hMyWeapons", i);
		if (IsValidWeapon(weapon))
		{
			int temp = GetWeaponIndexByEntity(weapon);
			if (temp == index)
			{
				int clip = GetWeaponClipAmmo(weapon);
				int reserve = GetWeaponReserveAmmo(weapon);

				RemovePlayerItem(client, weapon);
				RemoveEntity(weapon);

				// Give new weapon.
				weapon = GivePlayerItem(client, g_Weapons[index].m_className);
				if (clip != -1 || reserve != -1)
				{
					DataPack pack = new DataPack();
					pack.WriteCell(EntIndexToEntRef(weapon));
					pack.WriteCell(clip);
					pack.WriteCell(reserve);
					RequestFrame(Frame_ResetAmmo, pack);
				}
				break;
			}
		}
	}
}

/**
 * Callbacks.
 */
public void Frame_ResetAmmo(DataPack data)
{
	data.Reset();
	int weapon = EntRefToEntIndex(data.ReadCell());
	int clip = data.ReadCell();
	int reserve = data.ReadCell();
	delete data;

	if (weapon == INVALID_ENT_REFERENCE || !IsValidEntity(weapon))
	{
		return;
	}

	SetWeaponClipAmmo(weapon, clip);
	SetWeaponReserveAmmo(weapon, reserve);
}