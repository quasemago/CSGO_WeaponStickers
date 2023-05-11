void LoadDatabase()
{
	if (!g_cvarEnabled.BoolValue)
	{
		return;
	}

	if (g_Database != null)
	{
		LogMessage("(WeaponStickers) Database is already connected! (Handle: %d) Databse Type:%s", g_Database,g_bMySQL ? "MySQL":"SQLite");
		return;
	}

	if(SQL_CheckConfig(MySQL_DATABASE))
	{
		Database.Connect(SQLCallback_Connect, MySQL_DATABASE);
	}else Database.Connect(SQLCallback_Connect, "storage-local");
}

void LoadClientData(int client)
{
	if (g_Database == null)
	{
		return;
	}

	if (!ClientHasFlag(client, g_requiredFlag))
	{
		return;
	}

	char authId[MAX_LENGTH_AUTHID];
	if (!GetClientAuthId(client, AuthId_Steam3, authId, sizeof(authId)))
	{
		LogError("[LoadClientData] Auth failed for client index %d", client);
		return;
	}

	char query[512];

	FormatEx(query, sizeof(query), "SELECT * FROM %s WHERE steamid = \"%s\";", MySQL_TABLE, authId);
	g_Database.Query(SQLCallback_LoadClientData, query, GetClientUserId(client));
}

void UpdateClientData(int client, int index, int slot)
{
	if (!client || !IsClientInGame(client))
	{
		return;
	}

	char authId[MAX_LENGTH_AUTHID];
	if (!GetClientAuthId(client, AuthId_Steam3, authId, sizeof(authId)))
	{
		LogError("[UpdateClientData] Auth failed for client index %d", client);
		return;
	}

	int defIndex = eItems_GetWeaponDefIndexByWeaponNum(index);
	if (!IsValidDefIndex(defIndex))
	{
		LogError("[UpdateClientData] Weapon failed for client index %d", client);
		return;
	}

	// Update reuse time.
	g_playerReuseTime[client] = GetTime() + g_cvarReuseTime.IntValue;

	// Update MySQL.
	char query[2048];

	if(g_bMySQL)
	{
		FormatEx(query, sizeof(query), "INSERT INTO %s (`steamid`, `weaponindex`, `slot%i`,`last_seen`) VALUES (\"%s\", '%i', '%i','%d') ON DUPLICATE KEY UPDATE slot%i = '%i',last_seen='%d';", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_sticker[slot],GetTime(), slot, g_PlayerWeapon[client][index].m_sticker[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData, query);
	}
	else 
	{
		FormatEx(query, sizeof(query), "UPDATE %s SET `slot%i`='%i',`last_seen`='%d' WHERE `steamid`=\"%s\" AND `weaponindex`='%i';", MySQL_TABLE, slot, g_PlayerWeapon[client][index].m_sticker[slot],GetTime(),authId,defIndex);
		g_Database.Query(SQLCallback_UpdateClientData, query);

		FormatEx(query, sizeof(query), "INSERT OR IGNORE INTO %s (`steamid`, `weaponindex`, `slot%i`,`last_seen`) VALUES (\"%s\", '%i', '%i','%d');", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_sticker[slot], slot, g_PlayerWeapon[client][index].m_sticker[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData, query);
	}
	
}

void UpdateRotationData(int client, int index, int slot)
{
	if (!client || !IsClientInGame(client))
	{
		return;
	}

	char authId[MAX_LENGTH_AUTHID];
	if (!GetClientAuthId(client, AuthId_Steam3, authId, sizeof(authId)))
	{
		LogError("[UpdateClientData] Auth failed for client index %d", client);
		return;
	}

	int defIndex = eItems_GetWeaponDefIndexByWeaponNum(index);
	if (!IsValidDefIndex(defIndex))
	{
		LogError("[UpdateClientData] Weapon failed for client index %d", client);
		return;
	}

	// Update reuse time.
	g_playerReuseTime[client] = GetTime() + g_cvarReuseTime.IntValue;

	// Update MySQL.
	char rotation[2048];
	if(g_bMySQL)
	{
		FormatEx(rotation, sizeof(rotation), "INSERT INTO %s (`steamid`, `weaponindex`, `rotation%i`,`last_seen`) VALUES (\"%s\", '%i', '%f','%d') ON DUPLICATE KEY UPDATE rotation%i = '%f',last_seen='%d';", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_rotation[slot],GetTime(),slot, g_PlayerWeapon[client][index].m_rotation[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData,rotation);
	}
	else 
	{
		FormatEx(rotation, sizeof(rotation), "UPDATE %s SET `rotation%i`='%f',`last_seen`='%d' WHERE `steamid`=\"%s\" AND `weaponindex`='%d';", MySQL_TABLE, slot, g_PlayerWeapon[client][index].m_rotation[slot],GetTime(),authId,defIndex);
		g_Database.Query(SQLCallback_UpdateClientData, rotation);

		FormatEx(rotation, sizeof(rotation), "INSERT OR IGNORE INTO %s (`steamid`, `weaponindex`, `rotation%i`,`last_seen`) VALUES (\"%s\", '%i', '%f','%d');", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_rotation[slot], slot, g_PlayerWeapon[client][index].m_rotation[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData, rotation);
	}
	
}

void UpdateWearData(int client, int index, int slot)
{
	if (!client || !IsClientInGame(client))
	{
		return;
	}

	char authId[MAX_LENGTH_AUTHID];
	if (!GetClientAuthId(client, AuthId_Steam3, authId, sizeof(authId)))
	{
		LogError("[UpdateClientData] Auth failed for client index %d", client);
		return;
	}

	int defIndex = eItems_GetWeaponDefIndexByWeaponNum(index);
	if (!IsValidDefIndex(defIndex))
	{
		LogError("[UpdateClientData] Weapon failed for client index %d", client);
		return;
	}

	// Update reuse time.
	g_playerReuseTime[client] = GetTime() + g_cvarReuseTime.IntValue;

	// Update MySQL.
	char wear[2048];
	if(g_bMySQL)
	{
		FormatEx(wear, sizeof(wear), "INSERT INTO %s (`steamid`, `weaponindex`, `wear%i`,`last_seen`) VALUES (\"%s\", '%i', '%f','%d') ON DUPLICATE KEY UPDATE wear%i = '%f',last_seen='%d';", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_wear[slot],GetTime(), slot, g_PlayerWeapon[client][index].m_wear[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData,wear);
	}
	else
	{
		FormatEx(wear, sizeof(wear), "UPDATE %s SET `wear%i`='%f',`last_seen`='%d' WHERE `steamid`=\"%s\" AND `weaponindex`='%d';", MySQL_TABLE, slot, g_PlayerWeapon[client][index].m_wear[slot],GetTime(),authId,defIndex);
		g_Database.Query(SQLCallback_UpdateClientData, wear);

		FormatEx(wear, sizeof(wear), "INSERT OR IGNORE INTO %s (`steamid`, `weaponindex`, `wear%i`,`last_seen`) VALUES (\"%s\", '%i', '%f','%d');", MySQL_TABLE, slot, authId, defIndex, g_PlayerWeapon[client][index].m_wear[slot], slot, g_PlayerWeapon[client][index].m_wear[slot],GetTime());
		g_Database.Query(SQLCallback_UpdateClientData, wear);
	}	
}

void DeleteInactivePlayerData()
{
	if(g_Database == null)
		return;

	if(g_cvarInactive_days.IntValue <= 0)
		return;
	char query[512];

	FormatEx(query, sizeof(query), "DELETE FROM `%s` WHERE `last_seen` <= '%d'",MySQL_TABLE,GetTime()-(g_cvarInactive_days.IntValue * 86400));
	g_Database.Query(SQLCallback_DeleteInactivePlayerData, query);
}

public void SQLCallback_DeleteInactivePlayerData(Database db, DBResultSet results, const char[] error, any data)
{
	if (db == null || strlen(error) > 0)
	{
		LogError("(SQLCallback_DeleteInactivePlayerData) Fail at Query: %s", error);
	}
	LogMessage("Inactive players' data has been deleted");
}

/**
 * Callbacks.
 */
public Action Timer_RetryMySQL(Handle timer)
{
	LogMessage("(WeaponStickers) Reconnecting...");

	LoadDatabase();
	return Plugin_Stop;
}

public void SQLCallback_Connect(Database db, const char[] error, any data)
{
	if (db == null || strlen(error) > 0)
	{
		LogError("(WeaponStickers) Connection to database failed: %s", error);
		CreateTimer(5.0, Timer_RetryMySQL);
	}
	else
	{
		DBDriver driver = db.Driver;

		char buffer[32];
		driver.GetIdentifier(buffer, sizeof(buffer));

		if (!StrEqual(buffer, "mysql", false))
		{
			g_Database = db;
			SQLiteDB_Init();
			g_bMySQL = false;
			//SetFailState("(WeaponStickers) Connection to database failed: Only MySQL support!");
			LogMessage("(WeaponStickers) We have a connection! Databse Type:%s",g_bMySQL ? "MySQL":"SQLite");
			DeleteInactivePlayerData();
			return;
		}

		delete driver;
		g_Database = db;
		g_bMySQL = true;

		// Create tables if not exists.
		char query[1280];
		FormatEx(query, sizeof(query),
		"CREATE TABLE IF NOT EXISTS `%s` ( \
			`id` INT NOT NULL AUTO_INCREMENT, \
			`steamid` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL, \
			`weaponindex` int(11) NOT NULL DEFAULT '0', \
			`slot0` int(11) NOT NULL DEFAULT '0', \
			`slot1` int(11) NOT NULL DEFAULT '0', \
			`slot2` int(11) NOT NULL DEFAULT '0', \
			`slot3` int(11) NOT NULL DEFAULT '0', \
			`slot4` int(11) NOT NULL DEFAULT '0', \
			`slot5` int(11) NOT NULL DEFAULT '0', \
			`wear0` float4 NOT NULL DEFAULT '0.000000', \
			`wear1` float4 NOT NULL DEFAULT '0.000000', \
			`wear2` float4 NOT NULL DEFAULT '0.000000', \
			`wear3` float4 NOT NULL DEFAULT '0.000000', \
			`wear4` float4 NOT NULL DEFAULT '0.000000', \
			`wear5` float4 NOT NULL DEFAULT '0.000000', \
			`rotation0` float4 NOT NULL DEFAULT '0.000', \
			`rotation1` float4 NOT NULL DEFAULT '0.000', \
			`rotation2` float4 NOT NULL DEFAULT '0.000', \
			`rotation3` float4 NOT NULL DEFAULT '0.000', \
			`rotation4` float4 NOT NULL DEFAULT '0.000', \
			`rotation5` float4 NOT NULL DEFAULT '0.000', \
			`last_seen` int(11) NOT NULL DEFAULT '0', \
			PRIMARY KEY (`id`), \
			UNIQUE KEY (`steamid`, `weaponindex`) \
			) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;", MySQL_TABLE);
		g_Database.Query(SQLCallback_CreateTable, query);

		FormatEx(query, sizeof(query),"ALTER TABLE `%s` ADD COLUMN last_seen int(11);",MySQL_TABLE);
		g_Database.Query(SQLCallback_NoError, query);
		
		LogMessage("(WeaponStickers) We have a connection! Databse Type:%s",g_bMySQL ? "MySQL":"SQLite");

		DeleteInactivePlayerData();
	}
}



void SQLiteDB_Init()
{
	char query[1280];
	FormatEx(query, sizeof(query),
	"CREATE TABLE IF NOT EXISTS `%s` ( \
		`id` INTEGER PRIMARY KEY AUTOINCREMENT, \
		`steamid` varchar(64) NOT NULL,  \
		`weaponindex` int(11) NOT NULL DEFAULT '0', \
		`slot0` int(11) NOT NULL DEFAULT '0', \
		`slot1` int(11) NOT NULL DEFAULT '0', \
		`slot2` int(11) NOT NULL DEFAULT '0', \
		`slot3` int(11) NOT NULL DEFAULT '0', \
		`slot4` int(11) NOT NULL DEFAULT '0', \
		`slot5` int(11) NOT NULL DEFAULT '0', \
		`wear0` float4 NOT NULL DEFAULT '0.000000', \
		`wear1` float4 NOT NULL DEFAULT '0.000000', \
		`wear2` float4 NOT NULL DEFAULT '0.000000', \
		`wear3` float4 NOT NULL DEFAULT '0.000000', \
		`wear4` float4 NOT NULL DEFAULT '0.000000', \
		`wear5` float4 NOT NULL DEFAULT '0.000000', \
		`rotation0` float4 NOT NULL DEFAULT '0.000', \
		`rotation1` float4 NOT NULL DEFAULT '0.000', \
		`rotation2` float4 NOT NULL DEFAULT '0.000', \
		`rotation3` float4 NOT NULL DEFAULT '0.000', \
		`rotation4` float4 NOT NULL DEFAULT '0.000', \
		`rotation5` float4 NOT NULL DEFAULT '0.000', \
		`last_seen` int(11) NOT NULL DEFAULT '0', \
		UNIQUE(`steamid`, `weaponindex`) \
		)", MySQL_TABLE);
	g_Database.Query(SQLCallback_CreateTable, query);

	FormatEx(query, sizeof(query),"ALTER TABLE `%s` ADD COLUMN last_seen int(11)",MySQL_TABLE);
	g_Database.Query(SQLCallback_NoError, query);
}

public void SQLCallback_CreateTable(Database db, DBResultSet results, const char[] error, any data)
{
	if (db == null || strlen(error) > 0)
	{
		LogError("(SQLCallback_CreateTable) Fail at Query: %s", error);
	}
}

public void SQLCallback_LoadClientData(Database db, DBResultSet results, const char[] error, any data)
{
	if (db == null || strlen(error) > 0)
	{
		LogError("(SQLCallback_LoadClientData) Fail at Query: %s", error);
	}
	else
	{
		if (results.HasResults)
		{
			int client = GetClientOfUserId(data);

			if (client && IsClientInGame(client))
			{
				while (results.FetchRow())
				{
					// Get weapon defIndex and check if is valid to stickers.
					int defIndex = results.FetchInt(2);
					if (IsValidDefIndex(defIndex))
					{
						// Get weapon index.
						int weaponIndex = eItems_GetWeaponNumByDefIndex(defIndex);
						if (weaponIndex != -1)
						{
							g_PlayerWeapon[client][weaponIndex].m_sticker[0] = results.FetchInt(3);
							g_PlayerWeapon[client][weaponIndex].m_sticker[1] = results.FetchInt(4);
							g_PlayerWeapon[client][weaponIndex].m_sticker[2] = results.FetchInt(5);
							g_PlayerWeapon[client][weaponIndex].m_sticker[3] = results.FetchInt(6);
							g_PlayerWeapon[client][weaponIndex].m_sticker[4] = results.FetchInt(7);
							g_PlayerWeapon[client][weaponIndex].m_sticker[5] = results.FetchInt(8);
							
							g_PlayerWeapon[client][weaponIndex].m_wear[0] = results.FetchFloat(9);
							g_PlayerWeapon[client][weaponIndex].m_wear[1] = results.FetchFloat(10);
							g_PlayerWeapon[client][weaponIndex].m_wear[2] = results.FetchFloat(11);
							g_PlayerWeapon[client][weaponIndex].m_wear[3] = results.FetchFloat(12);
							g_PlayerWeapon[client][weaponIndex].m_wear[4] = results.FetchFloat(13);
							g_PlayerWeapon[client][weaponIndex].m_wear[5] = results.FetchFloat(14);
							
							g_PlayerWeapon[client][weaponIndex].m_rotation[0] = results.FetchFloat(15);
							g_PlayerWeapon[client][weaponIndex].m_rotation[1] = results.FetchFloat(16);
							g_PlayerWeapon[client][weaponIndex].m_rotation[2] = results.FetchFloat(17);
							g_PlayerWeapon[client][weaponIndex].m_rotation[3] = results.FetchFloat(18);
							g_PlayerWeapon[client][weaponIndex].m_rotation[4] = results.FetchFloat(19);
							g_PlayerWeapon[client][weaponIndex].m_rotation[5] = results.FetchFloat(20);
						}
					}
				}
			}
		}
	}
}

public void SQLCallback_UpdateClientData(Database db, DBResultSet results, const char[] error, any data)
{
	if (db == null || strlen(error) > 0)
	{
		LogError("(SQLCallback_UpdateClientData) Fail at Query: %s", error);
	}
}

public void SQLCallback_NoError(Database db, DBResultSet results, const char[] error, any data)
{

}
