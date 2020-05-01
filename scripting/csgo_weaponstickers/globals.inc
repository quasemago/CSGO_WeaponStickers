/**
 * Defines.
 */
#define PLUGIN_VERSION			"1.5.0"

#define MySQL_DATABASE			"csgo_weaponstickers"
#define MySQL_TABLE				"csgo_weaponstickers"

#define MAX_WEAPONS				53
#define MAX_STICKERS			500 // per set
#define MAX_STICKERS_SETS		20
#define MAX_STICKERS_SLOT		6

#define MAX_LENGTH_AUTHID		64
#define MAX_LENGTH_CLASSNAME 	32
#define MAX_LENGTH_DISPLAY		128
#define MAX_LENGTH_INDEX		16

#define CONFIG_PATH_WEAPONS		"configs/weapon_stickers/weapons.cfg"
#define CONFIG_PATH_STICKERS	"configs/weapon_stickers/stickers.cfg"

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

/* ConVars */
ConVar g_cvarEnabled = null;
ConVar g_cvarUpdateViewModel = null;
ConVar g_cvarReuseTime = null;

/* Modules */

// Weapons
enum struct WeaponData
{
	int m_defIndex;
	char m_displayName[MAX_LENGTH_DISPLAY];
	char m_className[MAX_LENGTH_CLASSNAME];
	int m_slot;
}

WeaponData g_Weapons[MAX_WEAPONS];
StringMap g_WeaponDefIndex = null;
StringMap g_WeaponClassName = null;

// Stickers
enum struct StickerData
{
	int m_defIndex;
	char m_displayName[MAX_LENGTH_DISPLAY];
}

StickerData g_StickerSets[MAX_STICKERS_SETS];
StickerData g_Sticker[MAX_STICKERS_SETS][MAX_STICKERS];

int g_stickerSetsCount = 0;
int g_stickerCount[MAX_STICKERS_SETS] = {0, ...};

/* Clients */
Database g_Database = null;

enum struct PlayerWeapon
{
	int m_stickerIndex[MAX_STICKERS_SLOT];

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