void LoadAPI()
{
	CreateNative("CS_SetWeaponSticker", Native_SetWeaponSticker);
}

public int Native_SetWeaponSticker(Handle plugins, int numParams)
{
	int client = GetNativeCell(1);
	if (!client || !IsClientInGame(client))
	{
		ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index [%i]", client);
		return false;
	}
	
	int weapon = GetNativeCell(2);
	if (!IsValidEntity(weapon))
	{
		return false;
	}

	Address pWeapon = GetEntityAddress(weapon);
	if (pWeapon == Address_Null)
	{
		ThrowNativeError(SP_ERROR_NATIVE, "Invalid weapon address [%i]", weapon);
		return false;
	}

	int slot = GetNativeCell(3);
	if (slot < 0 || slot > 5)
	{
		ThrowNativeError(SP_ERROR_NATIVE, "Invalid sticker slot");
		return false;
	}

	Address pEconItemView = pWeapon + view_as<Address>(g_EconItemOffset);
	SetAttributeValue(client, pEconItemView, GetNativeCell(4), "sticker slot %i id", slot);
	SetAttributeValue(client, pEconItemView, view_as<int>(GetNativeCell(5)), "sticker slot %i wear", slot);

	if (g_cvarUpdateViewModel.BoolValue)
	{
		PTaH_ForceFullUpdate(client);
	}
	return true;
}