stock void FindGameConfOffset(Handle gameConf, int &offset, char[] key)
{
	if ((offset = GameConfGetOffset(gameConf, key)) == -1)
	{
		SetFailState("Failed to get \"%s\" offset.", key);
	}
}

stock int FindSendPropOffset(char[] cls, char[] prop)
{
	int offset;
	if ((offset = FindSendPropInfo(cls, prop)) < 1)
	{
		SetFailState("Failed to find prop: \"%s\" on \"%s\" class.", prop, cls);
	}
	return offset;
}

stock bool AddMenuItemFormat(Menu menu, const char[] info, int style = ITEMDRAW_DEFAULT, const char[] format, any ...)
{
	char buffer[254];
	VFormat(buffer, sizeof(buffer), format, 5);
	return menu.AddItem(info, buffer, style);
}

stock bool ClientWeaponHasStickers(int client, int index)
{
	for (int i = 0; i < MAX_STICKERS_SLOT; i++)
	{
		if (g_PlayerWeapon[client][index].m_stickerDefIndex[i] != 0)
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

stock int GetWeaponSupportedStickerSlots(int weapon)
{
	Address pWeapon = GetEntityAddress(weapon);
	if (pWeapon == Address_Null)
	{
		return -1;
	}

	Address pItemDefition = SDKCall(g_SDKGetItemDefinition, pWeapon + view_as<Address>(g_econItemOffset));
	if (pItemDefition == Address_Null)
	{
		return -1;
	}

	return SDKCall(g_SDKGetNumSupportedStickerSlots, pItemDefition);
}

stock bool IsValidWeaponToChange(int client, int defIndex, bool message = false)
{
	if (defIndex <= 0)
	{
		return false;
	}
	else if (eItems_IsDefIndexKnife(defIndex))
	{
		if (message)
		{
			CPrintToChat(client, "You can't change knifes!");
		}
		return false;
	}
	else if (defIndex == 49)
	{
		if (message)
		{
			CPrintToChat(client, "You can't change c4!");
		}
		return false;
	}
	else if (defIndex >= 43 && defIndex <= 48)
	{
		if (message)
		{
			CPrintToChat(client, "You can't change grenades!");
		}
		return false;
	}
	return true;
}

stock bool IsKnifeClassname(const char[] classname)
{
	if ((StrContains(classname, "knife") > -1 && strcmp(classname, "weapon_knifegg") != 0) || StrContains(classname, "bayonet") > -1)
	{
		return true;
	}
	return false;
}

stock bool SetAttributeValue(int client, Address pEconItemView, int attrValue, const char[] format, any ...)
{
	char attr[254];
	VFormat(attr, sizeof(attr), format, 5);

	Address pAttributeDef = SDKCall(g_SDKGetAttributeDefinitionByName, g_pItemSchema, attr);
	if (pAttributeDef == Address_Null)
	{
		PrintToChat(client, "[SM] Invalid item attribute definition, contact an administrator.");
		return false;
	}

	// Get item attribute list.
	Address pAttributeList = pEconItemView + view_as<Address>(g_econItemView_AttributeListOffset);

	// Get attributes data.
	int attrDefIndex = LoadFromAddress(pAttributeDef + view_as<Address>(g_econItemAttributeDefinition_iAttributeDefinitionIndexOffset), NumberType_Int16);
	int attrCount = LoadFromAddress(pAttributeList + view_as<Address>(g_attributeList_CountOffset), NumberType_Int32);

	// Checks if the item already has the attribute, then update value.
	int k = 0;
	for (int i = 0; i < attrCount; i++)
	{
		Address pAttribute = DereferencePointer(pAttributeList + view_as<Address>(g_attributeList_ReadOffset)) + view_as<Address>(k);

		int defIndex = LoadFromAddress(pAttribute + view_as<Address>(g_attributes_iAttributeDefinitionIndexOffset), NumberType_Int16);
		if (defIndex == attrDefIndex)
		{
			// Checks if the value is different.
			int value = LoadFromAddress(pAttribute + view_as<Address>(g_attributes_iRawValue32Offset), NumberType_Int32);
			if (value != attrValue)
			{
				StoreToAddress(pAttribute + view_as<Address>(g_attributes_iRawValue32Offset), attrValue, NumberType_Int32);
				return true;
			}
			return false;
		}

		// Increment index.
		k += 24;
	}

	// Didn't find it. Add a new one.
	if (g_ServerPlatform == OS_Windows)
	{
		MemoryBlock block = new MemoryBlock(24);

		// Generate attribute block.
		Address pAttribute = block.Address;

		if (IsValidAddress(pAttribute))
		{
			// Set attribute values.
			StoreToAddress(pAttribute + view_as<Address>(g_attributes_iAttributeDefinitionIndexOffset), attrDefIndex, NumberType_Int16);
			StoreToAddress(pAttribute + view_as<Address>(g_attributes_iRawValue32Offset), attrValue, NumberType_Int32);
			StoreToAddress(pAttribute + view_as<Address>(g_attributes_iRawInitialValue32Offset), attrValue, NumberType_Int32);
			StoreToAddress(pAttribute + view_as<Address>(g_attributes_nRefundableCurrencyOffset), 0, NumberType_Int32);
			StoreToAddress(pAttribute + view_as<Address>(g_attributes_bSetBonusOffset), 0, NumberType_Int8);

			// Attach attribute in weapon.
			SDKCall(g_SDKAddAttribute, pAttributeList, DereferencePointer(pEconItemView + view_as<Address>(172)), pAttribute);

			delete block; // Needed?
			return true;
		}
	}
	else
	{
		// Generate attribute.
		Address pAttribute = SDKCall(g_SDKGenerateAttribute, g_pItemSystem, attrDefIndex, view_as<float>(attrValue));
		if (IsValidAddress(pAttribute))
		{
			// Attach attribute in weapon.
			SDKCall(g_SDKAddAttribute, pAttributeList, pAttribute);
			return true;
		}
	}
	return false;
}

stock Address DereferencePointer(Address addr)
{
	return view_as<Address>(LoadFromAddress(addr, NumberType_Int32));
}

// Snipped from TF2 Attributes.
stock bool IsValidAddress(Address pAddress)
{
	static Address Address_MinimumValid = view_as<Address>(0x10000);
	if (pAddress == Address_Null)
	{
		return false;
	}
	return unsigned_compare(view_as<int>(pAddress), view_as<int>(Address_MinimumValid)) >= 0;
}

stock int unsigned_compare(int a, int b)
{
	if (a == b)
		return 0;
	if ((a >>> 31) == (b >>> 31))
		return ((a & 0x7FFFFFFF) > (b & 0x7FFFFFFF)) ? 1 : -1;
	return ((a >>> 31) > (b >>> 31)) ? 1 : -1;
}

// Snipped from smlib
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