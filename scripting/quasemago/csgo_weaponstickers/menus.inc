static int g_menuSite[MAXPLAYERS + 1] = {0, ...};
static int g_tempSlot[MAXPLAYERS + 1] = {-1, ...};
static int g_tempMaxSlots[MAXPLAYERS + 1] = {0, ...};
static int g_tempIndex[MAXPLAYERS + 1] = {-1, ...};
static char g_tempSearch[MAXPLAYERS +1][MAX_LENGTH_CLASSNAME];

#define ALL_SLOTS 99

void MenusClientDisconnect(int client)
{
	g_menuSite[client] = 0;
	g_tempSlot[client] = -1;
	g_tempMaxSlots[client] = 0;
	g_tempIndex[client] = -1;
	g_tempSearch[client][0] = '\0';
}

/**
 * Menus.
 */
void ShowWeaponStickersMenu(int client, const char[] search = "")
{
	if (!IsPlayerAlive(client))
	{
		CPrintToChat(client, "%t", "Need Alive");
		return;
	}

	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (!IsValidEntity(weapon) || !IsValidWeaponToChange(client, GetWeaponDefIndex(weapon), false))
	{
		CPrintToChat(client, "%t", "Invalid Stickers Weapon");
		return;
	}

	int slots = GetWeaponSupportedStickerSlots(weapon);
	if (slots <= 0)
	{
		CPrintToChat(client, "%t", "Invalid Stickers Weapon");
		return;
	}

	int index = GetWeaponIndexByEntity(weapon);
	if (index < 0)
	{
		CPrintToChat(client, "%t", "Validate Error");
		return;
	}

	g_tempIndex[client] = index;
	g_tempMaxSlots[client] = slots;
	strcopy(g_tempSearch[client], MAX_LENGTH_CLASSNAME, search);

	Menu menu = new Menu(MenuHandler_Menu_WeaponStickers);
	menu.SetTitle("%T", "Menu Stickers Title", client);
	
	for (int i = 0; i < slots; i++)
	{
		static char slot[16];
		IntToString(i, slot, sizeof(slot));
		
		if (g_PlayerWeapon[client][index].m_stickerIndex[i] != 0)
		{
			static char displayName[MAX_LENGTH_DISPLAY];
			GetStickerNameByDefIndex(g_PlayerWeapon[client][index].m_stickerIndex[i], displayName, sizeof(displayName));
			AddMenuItemEx(menu, slot, _, "Slot %i\n  -> %s.", i, displayName);
		}
		else
		{
			AddMenuItemEx(menu, slot, _, "Slot %i\n  -> %T.", i, "None Sticker", client);
		}
	}

	menu.AddItem("x", "", ITEMDRAW_SPACER);
	AddMenuItemEx(menu, "99", _, "%T.", "All Slots", client);

	menu.ExitButton = true;
	menu.Display(client, MENU_TIME_FOREVER);
}


public int MenuHandler_Menu_WeaponStickers(Menu menu, MenuAction action, int client, int param)
{
	if (action == MenuAction_Select)
	{
		if (g_tempIndex[client] < 0)
		{
			ShowWeaponStickersMenu(client);
			return;
		}
		
		char buffer[16];
		menu.GetItem(param, buffer, sizeof(buffer));

		g_tempSlot[client] = StringToInt(buffer);

		if (strlen(g_tempSearch[client]) > 2)
		{
			ShowWeaponStickersSetMenu(client, g_tempSlot[client], -1, g_tempSearch[client]);
		}
		else
		{
			ShowWeaponStickerSlotMenu(client, g_tempSlot[client]);
		}
	}
	else if (action == MenuAction_Cancel)
	{
		ResetClientTempVars(client);
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
}

static void ShowWeaponStickerSlotMenu(int client, int slot)
{
	if (!IsPlayerAlive(client))
	{
		CPrintToChat(client, "%t", "Need Alive");
		return;
	}

	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (!IsValidEntity(weapon))
	{
		CPrintToChat(client, "%t", "Invalid Stickers Weapon");
		return;
	}

	int weaponIndex = GetWeaponIndexByEntity(weapon);
	if (weaponIndex < 0 || weaponIndex != g_tempIndex[client])
	{
		CPrintToChat(client, "%t", "Validate Error");
		return;
	}

	Menu menu = new Menu(MenuHandler_Menu_StickerSlot);
	if (slot != ALL_SLOTS && g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot] != 0)
	{
		char displayName[MAX_LENGTH_DISPLAY];
		GetStickerNameByDefIndex(g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot], displayName, sizeof(displayName));
		menu.SetTitle("%T", "Menu Stickers Slot Already Title", client, g_Weapons[weaponIndex].m_displayName, slot, displayName);
	}
	else
	{
		menu.SetTitle("%T", slot == ALL_SLOTS ? "Menu Stickers Slot Title AllSlots" : "Menu Stickers Slot Title", client, g_Weapons[weaponIndex].m_displayName, slot);
	}

	AddMenuItemEx(menu, "-1", _, "%T", "Menu Stickers Slot Remove", client);

	for (int i = 0; i < g_stickerSetsCount; i++)
	{
		static char index[16];
		IntToString(i, index, sizeof(index));
		AddMenuItemEx(menu, index, _, g_StickerSets[i].m_displayName);
	}

	menu.ExitButton = true;
	menu.ExitBackButton = true;
	menu.DisplayAt(client, g_menuSite[client], MENU_TIME_FOREVER);
}

public int MenuHandler_Menu_StickerSlot(Menu menu, MenuAction action, int client, int param)
{
	if (action == MenuAction_Select)
	{
		if (g_tempIndex[client] < 0)
		{
			ShowWeaponStickersMenu(client);
			return;
		}

		char buffer[32];
		menu.GetItem(param, buffer, sizeof(buffer));

		int index = StringToInt(buffer);
		if (index == -1)
		{
			if (g_tempSlot[client] == ALL_SLOTS)
			{
				for (int i = 0; i < g_tempMaxSlots[client]; i++)
				{
					g_PlayerWeapon[client][g_tempIndex[client]].m_stickerIndex[i] = 0;
					UpdateClientStickers(client, g_tempIndex[client], i);
				}
			}
			else
			{
				g_PlayerWeapon[client][g_tempIndex[client]].m_stickerIndex[g_tempSlot[client]] = 0;
				UpdateClientStickers(client, g_tempIndex[client], g_tempSlot[client]);
			}

			g_isStickerRefresh[client] = true;
			RefreshClientWeapon(client, g_tempIndex[client]);

			// Announce.
			if (g_tempSlot[client] == ALL_SLOTS)
			{
				CPrintToChat(client, "%t", "Remove Sticker AllSlots", g_Weapons[g_tempIndex[client]].m_displayName);
			}
			else
			{
				CPrintToChat(client, "%t", "Remove Sticker", g_Weapons[g_tempIndex[client]].m_displayName, g_tempSlot[client]);
			}

			// Reopen menu.
			g_menuSite[client] = GetMenuSelectionPosition();
			ShowWeaponStickerSlotMenu(client, g_tempSlot[client]);
		}
		else
		{
			ShowWeaponStickersSetMenu(client, g_tempSlot[client], index);
		}
	}
	else if (action == MenuAction_Cancel)
	{
		ResetClientTempVars(client);

		if (param == MenuCancel_ExitBack)
		{
			ShowWeaponStickersMenu(client);
		}
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
}

static void ShowWeaponStickersSetMenu(int client, int slot, int stickerSet, const char[] search = "")
{
	if (!IsPlayerAlive(client))
	{
		CPrintToChat(client, "%t", "Need Alive");
		return;
	}

	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (!IsValidEntity(weapon))
	{
		CPrintToChat(client, "%t", "Invalid Stickers Weapon");
		return;
	}

	int weaponIndex = GetWeaponIndexByEntity(weapon);
	if (weaponIndex < 0 || weaponIndex != g_tempIndex[client])
	{
		CPrintToChat(client, "%t", "Validate Error");
		return;
	}

	if (strlen(search) < 2 && stickerSet < 0)
	{
		CPrintToChat(client, "%t", "Validate Error");
		return;
	}

	Menu menu = new Menu(MenuHandler_Menu_StickerSet);
	int count;

	// Add stickers to menu.
	if (strlen(search) > 2)
	{
		if (slot != ALL_SLOTS && g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot] != 0)
		{
			char displayName[MAX_LENGTH_DISPLAY];
			GetStickerNameByDefIndex(g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot], displayName, sizeof(displayName));
			menu.SetTitle("%T", "Menu Stickers Set Search Already Title", client, g_Weapons[weaponIndex].m_displayName, slot, displayName, search);
		}
		else
		{
			menu.SetTitle("%T", slot == ALL_SLOTS ? "Menu Stickers Set Search Title AllSlots" : "Menu Stickers Set Search Title", client, g_Weapons[weaponIndex].m_displayName, slot, search);
		}

		for (int i = 0; i < g_stickerSetsCount; i++)
		{
			for (int j = 0; j < g_stickerCount[i]; j++)
			{
				if (StrContains(g_Sticker[i][j].m_displayName, search, false) == -1)
				{
					continue;
				}
				
				count++;
	
				static char index[16];
				Format(index, sizeof(index), "%i;%i", i, j);
				AddMenuItemEx(menu, index, _, g_Sticker[i][j].m_displayName);
			}
		}
	}
	else
	{
		if (slot != ALL_SLOTS && g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot] != 0)
		{
			char displayName[MAX_LENGTH_DISPLAY];
			GetStickerNameByDefIndex(g_PlayerWeapon[client][weaponIndex].m_stickerIndex[slot], displayName, sizeof(displayName));
			menu.SetTitle("%T", "Menu Stickers Set Already Title", client, g_Weapons[weaponIndex].m_displayName, slot, displayName, g_StickerSets[stickerSet].m_displayName);
		}
		else
		{
			menu.SetTitle("%T", slot == ALL_SLOTS ? "Menu Stickers Set Title AllSlots" : "Menu Stickers Set Title", client, g_Weapons[weaponIndex].m_displayName, slot, g_StickerSets[stickerSet].m_displayName);
		}

		for (int i = 0; i < g_stickerCount[stickerSet]; i++)
		{
			static char index[16];
			Format(index, sizeof(index), "%i;%i", stickerSet, i);
			AddMenuItemEx(menu, index, _, g_Sticker[stickerSet][i].m_displayName);
		}
	}

	if (!count && strlen(search) > 2)
	{
		AddMenuItemEx(menu, "-1", ITEMDRAW_DISABLED, "%T", "Menu Stickers Slot Search None", client);
	}

	menu.ExitButton = true;
	menu.ExitBackButton = true;
	menu.DisplayAt(client, g_menuSite[client], MENU_TIME_FOREVER);
}

public int MenuHandler_Menu_StickerSet(Menu menu, MenuAction action, int client, int param)
{
	if (action == MenuAction_Select)
	{
		if (g_tempIndex[client] < 0)
		{
			ShowWeaponStickersMenu(client);
			return;
		}

		char buffer[32];
		menu.GetItem(param, buffer, sizeof(buffer));

		char output[2][16];
		ExplodeString(buffer, ";", output, sizeof(output), sizeof(output[]));

		int stickerSet = StringToInt(output[0]);
		int stickerIndex = StringToInt(output[1]);
		if (stickerSet < 0 || stickerIndex < 0)
		{
			ShowWeaponStickersMenu(client);
			return;
		}

		// Reuse countdown.
		int time = GetTime();
		if (g_playerReuseTime[client] > time)
		{
			CPrintToChat(client, "%t", "Delay Sticker", g_playerReuseTime[client] - time);

			// Reopen menu.
			g_menuSite[client] = GetMenuSelectionPosition();
			ShowWeaponStickersSetMenu(client, g_tempSlot[client], stickerSet, g_tempSearch[client]);
			return;
		}

		if (g_tempSlot[client] == ALL_SLOTS)
		{
			for (int i = 0; i < g_tempMaxSlots[client]; i++)
			{
				g_PlayerWeapon[client][g_tempIndex[client]].m_stickerIndex[i] = g_Sticker[stickerSet][stickerIndex].m_defIndex;
				UpdateClientStickers(client, g_tempIndex[client], i);
			}
		}
		else
		{
			g_PlayerWeapon[client][g_tempIndex[client]].m_stickerIndex[g_tempSlot[client]] = g_Sticker[stickerSet][stickerIndex].m_defIndex;
			UpdateClientStickers(client, g_tempIndex[client], g_tempSlot[client]);
		}

		g_isStickerRefresh[client] = true;
		RefreshClientWeapon(client, g_tempIndex[client]);

		// Announce.
		if (g_tempSlot[client] == ALL_SLOTS)
		{
			CPrintToChat(client, "%t", "Change Sticker AllSlots", g_Sticker[stickerSet][stickerIndex].m_displayName, g_Weapons[g_tempIndex[client]].m_displayName);
		}
		else
		{
			CPrintToChat(client, "%t", "Change Sticker", g_Sticker[stickerSet][stickerIndex].m_displayName, g_Weapons[g_tempIndex[client]].m_displayName, g_tempSlot[client]);
		}

		// Reopen menu.
		g_menuSite[client] = GetMenuSelectionPosition();
		ShowWeaponStickersSetMenu(client, g_tempSlot[client], stickerSet, g_tempSearch[client]);
	}
	else if (action == MenuAction_Cancel)
	{
		if (param == MenuCancel_ExitBack)
		{
			if (strlen(g_tempSearch[client]) > 2)
			{
				ResetClientTempVars(client);
				ShowWeaponStickersMenu(client);
			}
			else
			{
				ShowWeaponStickerSlotMenu(client, g_tempSlot[client]);
			}
		}
	}
	else if (action == MenuAction_End)
	{
		delete menu;
	}
}

static void ResetClientTempVars(int client)
{
	g_menuSite[client] = 0;
	g_tempSlot[client] = -1;
	g_tempMaxSlots[client] = 0;
	g_tempIndex[client] = -1;
	g_tempSearch[client][0] = '\0';
}