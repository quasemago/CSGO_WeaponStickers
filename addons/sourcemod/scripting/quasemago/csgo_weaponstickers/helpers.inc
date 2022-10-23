stock bool AddMenuItemFormat(Menu menu, const char[] info, int style = ITEMDRAW_DEFAULT, const char[] format, any ...)
{
	char buffer[254];
	VFormat(buffer, sizeof(buffer), format, 5);
	return menu.AddItem(info, buffer, style);
}

stock bool ClientHasFlag(int client, const char[] flags)
{
	if (strlen(flags) == 0)
	{
		return true;
	}

	int iFlags = GetUserFlagBits(client);
	if (iFlags & ADMFLAG_ROOT)
	{
		return true;
	}

	AdminFlag aFlags[24];
	FlagBitsToArray(ReadFlagString(flags), aFlags, sizeof(aFlags));

	for (int i = 0; i < sizeof(aFlags); i++)
	{
		if (iFlags & FlagToBit(aFlags[i]))
		{
			return true;
		}
	}
	return false;
}

stock bool ClientWeaponHasStickers(int client, int defIndex)
{
	int index = eItems_GetWeaponNumByDefIndex(defIndex);
	if (index < 0)
	{
		return false;
	}

	for (int i = 0; i < MAX_STICKERS_SLOT; i++)
	{
		if (g_PlayerWeapon[client][index].m_sticker[i] != 0)
		{
			return true;
		}
	}
	return false;
}

stock int GetRandomSticker()
{
	int index = Math_GetRandomInt(0, g_stickerCount - 1);
	return g_Sticker[index].m_defIndex;
}

stock bool IsValidDefIndex(int defIndex)
{
	static int blackList[] =
	{
		20, 31, 37, 41, 42, 49, 57, 59, 68, 69, 70, 72, 75, 76, 78, 81, 82, 83, 84, 85
	};

	// Avoid invalid def index, grenades or knifes.
	if (defIndex <= 0 || (defIndex >= 43 && defIndex <= 48) || eItems_IsDefIndexKnife(defIndex))
	{
		return false;
	}

	// Check defIndex blacklist.
	int size = sizeof(blackList);
	for (int i = 0; i < size; i++)
	{
		if (defIndex == blackList[i])
		{
			return false;
		}
	}
	return true;
}

// Snipped from smlib.
#define SIZE_OF_INT		2147483647		// without 0
stock int Math_GetRandomInt(int min, int max)
{
	int random = GetURandomInt();
	if (random == 0)
	{
		random++;
	}
	return RoundToCeil(float(random) / (float(SIZE_OF_INT) / float(max - min + 1))) + min - 1;
}
