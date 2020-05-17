/**
 * Defines.
 */
#define PLUGIN_VERSION			"1.6.0"

#define MySQL_DATABASE			"csgo_weaponstickers"
#define MySQL_TABLE				"csgo_weaponstickers"

#define MAX_WEAPONS				128
#define MAX_STICKERS_SLOT		6
#define MAX_STICKERS_SETS		24

#define MAX_LENGTH_AUTHID		64
#define MAX_LENGTH_CLASSNAME 	32
#define MAX_LENGTH_DISPLAY		128
#define MAX_LENGTH_INDEX		16

/**
 * Variables.
 */
enum ServerPlatform
{
	OS_Unknown = 0,
	OS_Windows,
	OS_Linux,
	OS_Mac
}

ServerPlatform g_ServerPlatform;

ConVar g_cvarEnabled = null;
ConVar g_cvarUpdateViewModel = null;
ConVar g_cvarReuseTime = null;

int g_stickerCount;
int g_stickerSetsCount;

ArrayList g_StickerSet[MAX_STICKERS_SETS] = {null, ...};
StringMap g_Stickers = null;

/* Clients */
Database g_Database = null;

enum struct PlayerWeapon
{
	int m_stickerDefIndex[MAX_STICKERS_SLOT];

	// TODO:
	// Add sticker wear,rotation,scale.
}

PlayerWeapon g_PlayerWeapon[MAXPLAYERS + 1][MAX_WEAPONS];
bool g_isStickerRefresh[MAXPLAYERS + 1] = {false, ...};
int g_playerReuseTime[MAXPLAYERS + 1] = {0, ...};

/* SDK */
Address g_pItemSystem = Address_Null;
Address g_pItemSchema = Address_Null;

// SDKCalls
Handle g_SDKGetItemDefinition = null;
Handle g_SDKGetNumSupportedStickerSlots = null;
Handle g_SDKAddAttribute = null;
Handle g_SDKGenerateAttribute = null;
Handle g_SDKGetAttributeDefinitionByName = null;

// Offsets
int g_EconItemView_AttributeListOffset = -1;
int g_EconItemAttributeDefinition_iAttributeDefinitionIndexOffset = -1;
int g_Attributes_iAttributeDefinitionIndexOffset = -1;
int g_Attributes_iRawValue32Offset = -1;
int g_Attributes_iRawInitialValue32Offset = -1;
int g_Attributes_nRefundableCurrencyOffset = -1;
int g_Attributes_bSetBonusOffset = -1;
int g_AttributeList_ReadOffset = -1;
int g_AttributeList_CountOffset = -1;
int g_EconItemOffset = -1;