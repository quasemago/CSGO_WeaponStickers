void LoadAPI()
{
	CreateNative("CS_SetWeaponSticker", Native_SetWeaponSticker);
	CreateNative("CS_GetRandomSticker", Native_GetRandomSticker);
}

public int Native_SetWeaponSticker(Handle plugins, int numParams)
{
	int iClient = GetNativeCell(1);

	if (iClient < 1 || !IsClientInGame(iClient))
	{
		ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index [%i]", iClient);

		return false;
	}
	
	int iWeapon = GetNativeCell(2);

	if (!IsValidEntity(iWeapon))
	{
		return false;
	}

	int iSlot = GetNativeCell(3);

	if (iSlot < 0 || iSlot > 5)
	{
		ThrowNativeError(SP_ERROR_NATIVE, "Invalid sticker slot");

		return false;
	}

	int iStickerIndex = GetNativeCell(4);

	if (iStickerIndex == -1) // random
	{
		iStickerIndex = GetRandomSticker();
	}

	
	CAttributeList pAttributeList = PTaH_GetEconItemViewFromEconEntity(iWeapon).NetworkedDynamicAttributesForDemos;

	pAttributeList.SetOrAddAttributeValue(113 + iSlot * 4, iStickerIndex); // sticker slot %i id

	float fStickerWear = GetNativeCell(5);
	
	if(fStickerWear != 0.0)
	{
		pAttributeList.SetOrAddAttributeValue(114 + iSlot * 4, fStickerWear); // sticker slot %i wear
	}
	
	float fStickerRotation = GetNativeCell(6);
	
	if(fStickerRotation != 0.0)
	{
		pAttributeList.SetOrAddAttributeValue(116 + iSlot * 4, fStickerRotation); //sticker slot %i rotation
	}

	PTaH_ForceFullUpdate(iClient);


	return true;
}

public int Native_GetRandomSticker(Handle plugins, int numParams)
{
	return GetRandomSticker();
}
