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
#include <eItems>

#pragma semicolon 1
#pragma newdecls required
#pragma tabsize 0

/**
 * Sub.
 */
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
	author = "quasemago and z1ntex",
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

	
	if(PTaH_Version() < 101030)
	{
		char sBuf[16];

		PTaH_Version(sBuf, sizeof(sBuf));
		SetFailState("PTaH extension needs to be updated. (Installed Version: %s - Required Version: 1.1.3+) [ Download from: https://ptah.zizt.ru ]", sBuf);

		return;
	}
	// Translations.
	LoadTranslations("common.phrases");
	LoadTranslations("csgo_weaponstickers.phrases");

	// ConVars.
	CreateConVar("sm_weaponstickers_version", PLUGIN_VERSION, "Plugin Version", FCVAR_NOTIFY|FCVAR_SPONLY|FCVAR_DONTRECORD);
	g_cvarEnabled = CreateConVar("sm_weaponstickers_enabled", "1", "Enable or disable Plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_cvarUpdateViewModel = CreateConVar("sm_weaponstickers_updateviewmodel", "0", "Specifies whether the view model will be updated when changing stickers (P.S: the player will experience a small rollback).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_cvarReuseTime = CreateConVar("sm_weaponstickers_reusetime", "5", "Specifies how many seconds it will be necessary to wait to update the stickers again.", FCVAR_NOTIFY, true, 0.1);
	g_cvarOverrideViewItem = CreateConVar("sm_weaponstickers_overrideview", "1", "Specifies whether the plugin will override the weapon view (p.s: Recommended if !ws plugin is used).", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	g_cvarFlagUse = CreateConVar("sm_weaponstickers_flag", "", "Specifies the required flag (e.g: 'a' for reserved slot).", FCVAR_NOTIFY);
	g_cvarInactive_days = CreateConVar("sm_weaponstickers_inactive_days", "30", "Number of days before a player (SteamID) is marked as inactive and his data is deleted. (0 or any negative value to disable deleting)", FCVAR_NOTIFY);
	
	AutoExecConfig(true, "csgo_weaponstickers");
	CSetPrefix("%t", "Prefiks");

	// Forward event to modules.
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

	/* Library */
	RegPluginLibrary("csgo_weaponstickers");
	return APLRes_Success;
}

/**
 * Forwards.
 */
public void OnConfigsExecuted()
{
	DeleteInactivePlayerData();
	g_cvarFlagUse.GetString(g_requiredFlag, sizeof(g_requiredFlag));
}

public void eItems_OnItemsSynced()
{
	g_stickerCount = eItems_GetStickersCount();
	g_stickerSetsCount = eItems_GetStickersSetsCount();

	RequestFrame(Frame_ItemsSync);
}

public void Frame_ItemsSync(any data)
{
	// Load stickers.
	for (int i = 0; i < g_stickerSetsCount; i++)
	{
		g_StickerSet[i].m_Id = eItems_GetStickerSetIdByStickerSetNum(i);
		eItems_GetStickerSetDisplayNameByStickerSetNum(i, g_StickerSet[i].m_displayName, MAX_LENGTH_DISPLAY);

		if (g_StickerSet[i].m_Stickers != null)
		{
			delete g_StickerSet[i].m_Stickers;
		}

		g_StickerSet[i].m_Stickers = new ArrayList(1);
		for (int j = 0; j < g_stickerCount; j++)
		{
			if (eItems_IsStickerInSet(i, j))
			{
				g_Sticker[j].m_setId = g_StickerSet[i].m_Id;
				g_StickerSet[i].m_Stickers.Push(j);
			}
		}
	}

	for (int i = 0; i < g_stickerCount; i++)
	{
		g_Sticker[i].m_defIndex = eItems_GetStickerDefIndexByStickerNum(i);
		eItems_GetStickerDisplayNameByStickerNum(i, g_Sticker[i].m_displayName, MAX_LENGTH_DISPLAY);
	}
}

public void OnClientPostAdminCheck(int client)
{
	if (!g_cvarEnabled.BoolValue || IsFakeClient(client))
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
			g_PlayerWeapon[client][i].m_sticker[j] = DEFAULT_PAINT;
		}
	}

	MenusClientDisconnect(client);
}

/**
 * Events & Hooks.
 */
public Action OnGiveNamedItemPre(int client, char classname[64], CEconItemView &item, bool &ignoredCEconItemView, bool &isOriginNULL, float origin[3])
{
	if (!g_cvarEnabled.BoolValue || !g_cvarOverrideViewItem.BoolValue)
	{
		return Plugin_Continue;
	}

	if (IsClientInGame(client) && !IsFakeClient(client))
	{
		int defIndex = eItems_GetWeaponDefIndexByClassName(classname);
		if (IsValidDefIndex(defIndex))
		{
			if (ClientWeaponHasStickers(client, defIndex))
			{
				ignoredCEconItemView = true;
				return Plugin_Changed;
			}
		}
	}
	return Plugin_Continue;
}

public void OnGiveNamedItemPost(int client, const char[] classname, const CEconItemView item, int entity, bool isOriginNULL, const float origin[3])
{
	if (!g_cvarEnabled.BoolValue || entity == -1)
	{
		return;
	}

	SetWeaponSticker(client, entity);
}

void SetWeaponSticker(int iClient, int iEntity)
{
	if (IsClientInGame(iClient) && !IsFakeClient(iClient) && IsValidEntity(iEntity))
	{
		CEconItemView pItemView = PTaH_GetEconItemViewFromEconEntity(iEntity);

		int iDefIndex = pItemView.GetItemDefinition().GetDefinitionIndex();

		if (IsValidDefIndex(iDefIndex) && ClientWeaponHasStickers(iClient, iDefIndex))
		{
			int iIndex = eItems_GetWeaponNumByDefIndex(iDefIndex);

			if (iIndex != -1)
			{
				// Check if item is already initialized by external ws.
				if (GetEntProp(iEntity, Prop_Send, "m_iItemIDHigh") < 16384)
				{
					static int IDHigh = 16384;
					
					SetEntProp(iEntity, Prop_Send, "m_iItemIDLow", -1);
					SetEntProp(iEntity, Prop_Send, "m_iItemIDHigh", IDHigh++);
				}

				// Change stickers.
				CAttributeList pAttributeList = pItemView.NetworkedDynamicAttributesForDemos;

				bool bUpdated = false;

				for (int i = 0; i < MAX_STICKERS_SLOT; i++)
				{
					if (g_PlayerWeapon[iClient][iIndex].m_sticker[i] != 0)
					{
						// Sticker updated.
						bUpdated = true;

						pAttributeList.SetOrAddAttributeValue(113 + i * 4, g_PlayerWeapon[iClient][iIndex].m_sticker[i]); // sticker slot %i id
						
						if(g_PlayerWeapon[iClient][iIndex].m_wear[i] != 0.0)
						{
							pAttributeList.SetOrAddAttributeValue(114 + i * 4, g_PlayerWeapon[iClient][iIndex].m_wear[i]); // sticker slot %i wear
						}
						
						if(g_PlayerWeapon[iClient][iIndex].m_rotation[i] != 0.0)
						{
							pAttributeList.SetOrAddAttributeValue(116 + i * 4, g_PlayerWeapon[iClient][iIndex].m_rotation[i]); // sticker slot %i rotation
						}
					}
				}

				// Update viewmodel if enabled.
				if (bUpdated && g_isStickerRefresh[iClient])
				{
					g_isStickerRefresh[iClient] = false;					
			
					if (g_cvarUpdateViewModel.BoolValue)
					{
						PTaH_ForceFullUpdate(iClient);
					}
				}
			}
		}
	}	
}

/**
 * Functions.
 */

void RefreshClientWeapon(int client, int index)
{
	// Validate weapon defIndex or knife.
	int defIndex = eItems_GetWeaponDefIndexByWeaponNum(index);
	if (!IsValidDefIndex(defIndex) || eItems_IsDefIndexKnife(defIndex))
	{
		return;
	}

	// Get weapon classname.
	char classname[MAX_LENGTH_CLASSNAME];
	if (!eItems_GetWeaponClassNameByWeaponNum(index, classname, sizeof(classname)))
	{
		return;
	}

	int size = GetEntPropArraySize(client, Prop_Send, "m_hMyWeapons");
	for (int i = 0; i < size; i++)
	{
		int weapon = GetEntPropEnt(client, Prop_Send, "m_hMyWeapons", i);
		if (eItems_IsValidWeapon(weapon))
		{
			int temp = eItems_GetWeaponNumByWeapon(weapon);
			if (temp == index)
			{
				eItems_RespawnWeapon(client, weapon);
				break;
			}
		}
	}
}
