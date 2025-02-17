class BehaviorSettings
{
	string BehaviorType;
	int Chance;		
};

class SpawnSettingsBase
{
	//! Type of manager to use
	string ManagerType;
	
	//! Chance a group/unit will spawn outside (instead of inside)
	float SpawnOutsideChance;
	
	//! Time in seconds for checking when to spawn AI
	int SpawnTimerInSeconds;
	
	//! The number of chunks (from player center) AI can spawn (Think minecraft view distance)
	int SpawnDistanceInChunks;
	
	//! Grid Size
	int SpawnGridSize;
	
	//! Radius in chunks around player that is considered off limits for AI to spawn
	int AntiSpawnDistanceInChunks;
	
	//! Anti Spawn Grid Size
	int AntiSpawnGridSize;
	
	//! Time in seconds. Interval GC is checked
	float GarbageCollectionTimerInSeconds;
	
	//! Maximum number of units that can spawn per group (Random value betwee 1 and this)
	int GroupSize;
	
	//! Waypoint to use when groups spawn
	string DefendWaypointPrefab;
	
	//! Waypoint to use when groups are set to attack
	string AttackWaypointPrefab;
	
	//! Waypoint to use when creating patrol waypoints
	string PatrolWaypointPrefab;
	
	//! Waypoint to use when creating patrols
	string CycleWaypointPrefab;
	
	int WanderIntervalInSeconds;
	
	//! Settings per faction
	ref array<ref FactionSpawnSettings> FactionSettings;		
	
	void SetData(string type,
				 int spawnTimer,
				 int gridSize,
				 int gridRadius,
				 int antiGridRadius,
				 int antiGridSize,
				 float gcTimer,
				 int groupSize)
	{
		ManagerType = type;
		SpawnTimerInSeconds = spawnTimer;
		SpawnDistanceInChunks = gridRadius;
		SpawnGridSize = gridSize;
		AntiSpawnDistanceInChunks = antiGridRadius;
		AntiSpawnGridSize = antiGridSize;
		GarbageCollectionTimerInSeconds = gcTimer;
		WanderIntervalInSeconds = 60;
		FactionSettings = {};
		GroupSize = groupSize;
	}
	
	void AddFaction(FactionSpawnSettings settings)
	{
		FactionSettings.Insert(settings);
	}
	
	static const string FILENAME = "$profile:spawnSettings.json";
	
	static bool SaveToFile(SpawnSettingsBase settings)
	{
		if(!TW_Util.SaveJsonFile(FILENAME, settings, true))
		{
			PrintFormat("TrainWreck: SpawnSettingsBase -> Failed to save to file '%1'", FILENAME, LogLevel.ERROR);
			return false;
		}
		
		return true;
	}
	
	private static FactionSpawnSettings GetDefaultFactionSettings(SCR_Faction faction)
	{
		ref FactionSpawnSettings factionSettings = new FactionSpawnSettings();
				
		factionSettings.FactionName = faction.GetFactionKey();
		
		int maxAmount = 30;
		bool isEnabled = false;
		
		if(faction.GetFactionKey() == "USSR")
		{
			maxAmount = 2;
			isEnabled = true;	
		}
		else if(faction.GetFactionKey() == "CIV")
		{
			maxAmount = 10;
			isEnabled = true;
		}
		else if(faction.GetFactionKey() == "FIA")
		{
			maxAmount = 60;
			isEnabled = true;
		}
		
		factionSettings.MaxAmount = maxAmount;
		factionSettings.IsEnabled = isEnabled;
		factionSettings.AIWanderChance = 0.25;
		factionSettings.ChanceToSpawn = 10;
		factionSettings.Characters = {};
		factionSettings.Groups = {};
		factionSettings.Vehicles = {};
		
		factionSettings.Behaviors = new map<string, float>();
		
		factionSettings.Behaviors.Insert(SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.Patrol), 8);
		factionSettings.Behaviors.Insert(SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.DefendLocal), 90);
		factionSettings.Behaviors.Insert(SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.Attack), 0);
		factionSettings.Behaviors.Insert(SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.DefendArea), 2);
		
		SCR_EntityCatalog characterCatlaog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.CHARACTER);
		SCR_EntityCatalog vehicleCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		SCR_EntityCatalog groupCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		
		ref array<SCR_EntityCatalogEntry> entries = {};
		if(characterCatlaog)
		{
			int amount = characterCatlaog.GetEntityList(entries);
			InsertPrefabChanceFor(amount, entries, factionSettings.Characters);
		}
		
		if(vehicleCatalog)
		{
			int amount = vehicleCatalog.GetEntityList(entries);
			InsertVehiclePrefabChanceFor(amount, entries, factionSettings.Vehicles);
		}
		
		if(groupCatalog)
		{
			int amount = groupCatalog.GetEntityList(entries);
			InsertPrefabChanceFor(amount, entries, factionSettings.Groups);
		}
		
		return factionSettings;
	}
	
	private static void InsertPrefabChanceFor(int count, notnull array<SCR_EntityCatalogEntry> entries, notnull inout array<ref PrefabItemChance> items)
	{
		if(count <= 0) return;
		
		int defaultChance = 0;
		foreach(SCR_EntityCatalogEntry entry : entries)
		{
			ref PrefabItemChance item = new PrefabItemChance();
			item.PrefabName = entry.GetPrefab();
			if(item.PrefabName.Contains("Random"))
				item.Chance = 10;
			else
				item.Chance = defaultChance;
			items.Insert(item);
		}
	}
	
	private static void InsertVehiclePrefabChanceFor(int count, notnull array<SCR_EntityCatalogEntry> entries, notnull inout array<ref VehicleItemChance> items)
	{
		if(count <= 0)
			return;
		
		foreach(SCR_EntityCatalogEntry entry : entries)
		{
			ref VehicleItemChance item = new VehicleItemChance();
			item.PrefabName = entry.GetPrefab();
			item.Chance = 0;
			
			item.VehicleType = SCR_Enum.GetEnumName(TW_VehicleType, TW_VehicleType.Small);
			items.Insert(item);
		}
	}
	
	private static void MergeFactionsInGame(notnull out array<ref FactionSpawnSettings> factions)
	{
		FactionManager manager = GetGame().GetFactionManager();
		
		if(!manager)
			return;
		
		ref set<string> factionKeys = new set<string>();
		foreach(FactionSpawnSettings fac : factions)
			if(!factionKeys.Contains(fac.FactionName))
				factionKeys.Insert(fac.FactionName);
		
		ref array<Faction> entries = {};
		manager.GetFactionsList(entries);
		
		foreach(Faction entry : entries)
		{
			SCR_Faction faction = SCR_Faction.Cast(entry);
			
			if(!faction)
				continue;
			
			if(factionKeys.Contains(faction.GetFactionKey()))
				continue;
			
			factions.Insert(GetDefaultFactionSettings(faction));
		}
	}
	
	private static SpawnSettingsBase GetDefault()
	{
		PrintFormat("TrainWreck: SpawnSettingsBase -> Loading Default Settings", LogLevel.WARNING);
		ref SpawnSettingsBase settings = new SpawnSettingsBase();
		
		settings.SetData("default", 2, 250, 3, 2, 150, 5.0, 3);
		
		settings.CycleWaypointPrefab = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
		settings.PatrolWaypointPrefab = "{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et";
		settings.DefendWaypointPrefab = "{C0224D9332272F79}Prefabs/AI/Waypoints/TW_AIWaypoint_DefendSmall.et";
		settings.AttackWaypointPrefab = "{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et";
		
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if(factionManager)
		{
			ref array<Faction> entries = {};
			factionManager.GetFactionsList(entries);
			
			foreach(Faction entry : entries)
			{
				SCR_Faction faction = SCR_Faction.Cast(entry);
				
				if(!faction)
					continue;
				
				settings.AddFaction(GetDefaultFactionSettings(faction));
			}
		}
		
		return settings;
	}
	
	static SpawnSettingsBase LoadFromFile()
	{
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		
		if(!loadContext.LoadFromFile(FILENAME))
		{
			PrintFormat("TrainWreck: SpawnSettingsBase -> Failed to load settings from: %1", FILENAME, LogLevel.ERROR);
			ref SpawnSettingsBase settings = GetDefault();
			SpawnSettingsBase.SaveToFile(settings);
			return settings;
		}
		
		PrintFormat("TrainWreck: SpawnSettingsBase -> Successfully loaded spawnSettings.json");
		
		ref SpawnSettingsBase settings = new SpawnSettingsBase();
		SCR_JsonLoadContext context = TW_Util.LoadJsonFile(FILENAME, true);
		
		if(!context.ReadValue("", settings))
		{
			PrintFormat("TrainWreck: SpawnSettingsBase -> Loading settings failed", LogLevel.ERROR);
			return null;
		}
		
		return settings;
	}
};

class PrefabItemChance
{
	//! Path of prefab
	string PrefabName;
	
	//! Value between 0 and 100
	int Chance;
}

class VehicleItemChance : PrefabItemChance
{
	string VehicleType;
}

class FactionSpawnSettings
{	
	//! Key of faction in faction manager these values pertain to
	string FactionName;
	
	//! Value between 0 and 100
	int ChanceToSpawn;
	
	//! Maximum amount units that can belong to this faction
	int MaxAmount;
	
	//! Is this faction enabled
	bool IsEnabled;
	
	//! Chance the AI from this faction will randomly be told to wander
	float AIWanderChance;
	
	ref array<ref PrefabItemChance> Characters;
	ref array<ref VehicleItemChance> Vehicles;
	ref array<ref PrefabItemChance> Groups;
	
	ref map<string, float> Behaviors;
};