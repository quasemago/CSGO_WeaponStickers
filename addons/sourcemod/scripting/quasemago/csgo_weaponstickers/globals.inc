/**
 * Defines.
 */
#define PLUGIN_VERSION			"1.3.5"

#define MySQL_DATABASE			"csgo_weaponstickers"
#define MySQL_TABLE			"csgo_weaponstickers"

#define MAX_WEAPONS			128
#define MAX_STICKERS			8000
#define MAX_STICKERS_SETS		200
#define MAX_STICKERS_SLOT		6

#define MAX_LENGTH_AUTHID		64
#define MAX_LENGTH_CLASSNAME 	32
#define MAX_LENGTH_DISPLAY		1024
#define MAX_LENGTH_INDEX		16

#define DEFAULT_PAINT			0

/**
 * Variables.
 */
bool g_isLateLoad = false;
bool g_bMySQL = false;

char g_requiredFlag[20];

/* ConVars */
ConVar g_cvarEnabled = null;
ConVar g_cvarUpdateViewModel = null;
ConVar g_cvarReuseTime = null;
ConVar g_cvarOverrideViewItem = null;
ConVar g_cvarFlagUse = null;
ConVar g_cvarInactive_days = null;

/* Stickers */
enum struct StickerSet
{
	int m_Id;
	char m_displayName[MAX_LENGTH_DISPLAY];
	ArrayList m_Stickers;
}

enum struct Sticker
{
	int m_defIndex;
	int m_setId;
	char m_displayName[MAX_LENGTH_DISPLAY];
}

StickerSet g_StickerSet[MAX_STICKERS_SETS];
Sticker g_Sticker[MAX_STICKERS];

int g_stickerCount = 0;
int g_stickerSetsCount = 0;

/* Clients */
Database g_Database = null;

enum struct PlayerWeapon
{
	int m_sticker[MAX_STICKERS_SLOT];
	float m_wear[MAX_STICKERS_SLOT];
	float m_rotation[MAX_STICKERS_SLOT];
}

PlayerWeapon g_PlayerWeapon[MAXPLAYERS + 1][MAX_WEAPONS];
bool g_isStickerRefresh[MAXPLAYERS + 1] = {false, ...};
int g_playerReuseTime[MAXPLAYERS + 1] = {0, ...};
