class BehaviorSettings
{
	string BehaviorType;
	int Chance;		
};

class SpawnSettingsBase
{
	//! Type of manager to use
	string ManagerType;
	
	//! Time in seconds for checking when to spawn AI
	int SpawnTimerInSeconds;
	
	//! The spawn radius will mimic location settings
	int SpawnGridRadius;
	
	//! Grid Size
	int SpawnGridSize;
	
	//! Radius in chunks around player that is considered off limits for AI to spawn
	int AntiSpawnGridRadius;
	
	//! Anti Spawn Grid Size
	int AntiSpawnGridSize;
	
	//! Time in seconds. Interval GC is checked
	float GarbageCollectionTimerInSeconds;
	
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
				 float gcTimer)
	{
		ManagerType = type;
		SpawnTimerInSeconds = spawnTimer;
		SpawnGridRadius = gridRadius;
		SpawnGridSize = gridSize;
		AntiSpawnGridRadius = antiGridRadius;
		AntiSpawnGridSize = antiGridSize;
		GarbageCollectionTimerInSeconds = gcTimer;
		WanderIntervalInSeconds = 60;
		FactionSettings = {};		
	}
	
	void AddFaction(FactionSpawnSettings settings)
	{
		FactionSettings.Insert(settings);
	}
	
	static const string FILENAME = "$profile:spawnSettings.json";
	
	static bool HasFile() 
	{
		SCR_JsonLoadContext load = new SCR_JsonLoadContext();
		return load.LoadFromFile(FILENAME);
	}
	
	static bool SaveToFile(SpawnSettingsBase settings)
	{
		ContainerSerializationSaveContext saveContext = new ContainerSerializationSaveContext();
		PrettyJsonSaveContainer prettyContainer = new PrettyJsonSaveContainer();
		saveContext.SetContainer(prettyContainer);
		
		saveContext.WriteValue("managerType", settings.ManagerType);
		saveContext.WriteValue("spawnTimerInSeconds", settings.SpawnTimerInSeconds);
		saveContext.WriteValue("spawnGridSize", settings.SpawnGridSize);
		saveContext.WriteValue("spawnGridRadius", settings.SpawnGridRadius);
		saveContext.WriteValue("antiSpawnGridRadius", settings.AntiSpawnGridRadius);
		saveContext.WriteValue("antiSpawnGridSize", settings.AntiSpawnGridSize);
		saveContext.WriteValue("garbageCollectionTimerInSeconds", settings.GarbageCollectionTimerInSeconds);
		saveContext.WriteValue("factions", settings.FactionSettings);
		saveContext.WriteValue("defendWaypoint", settings.DefendWaypointPrefab);
		saveContext.WriteValue("attackWaypoint", settings.AttackWaypointPrefab);
		saveContext.WriteValue("cycleWaypoint", settings.CycleWaypointPrefab);
		saveContext.WriteValue("patrolWaypoint", settings.PatrolWaypointPrefab);
		saveContext.WriteValue("wanderIntervalInSeconds", settings.WanderIntervalInSeconds);
				
		return prettyContainer.SaveToFile(FILENAME);
	}
	
	private static FactionSpawnSettings GetDefaultFactionSettings(SCR_Faction faction)
	{
		ref FactionSpawnSettings factionSettings = new FactionSpawnSettings();
				
		factionSettings.FactionName = faction.GetFactionKey();
		factionSettings.MaxAmount = 30;
		factionSettings.IsEnabled = false;
		factionSettings.AIWanderChance = 0.25;
		factionSettings.ChanceToSpawn = 0;
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
		ref SpawnSettingsBase settings = new SpawnSettingsBase();
		
		settings.SetData("default", 2, 250, 3, 2, 150, 5.0);
		
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
			PrintFormat("TrainWreck-SpawnSystem: Failed to load settings from :%1", FILENAME, LogLevel.ERROR);
			ref SpawnSettingsBase settings = GetDefault();
			SpawnSettingsBase.SaveToFile(settings);
			return settings;
		}
		
		ref SpawnSettingsBase settings = new SpawnSettingsBase();
		
		loadContext.ReadValue("managerType", settings.ManagerType);
		loadContext.ReadValue("spawnTimerInSeconds", settings.SpawnTimerInSeconds);
		loadContext.ReadValue("spawnGridSize", settings.SpawnGridSize);
		loadContext.ReadValue("spawnGridRadius", settings.SpawnGridRadius);
		loadContext.ReadValue("antiSpawnGridRadius", settings.AntiSpawnGridRadius);
		loadContext.ReadValue("garbageCollectionTimerInSeconds", settings.GarbageCollectionTimerInSeconds);
		loadContext.ReadValue("factions", settings.FactionSettings);		
		loadContext.ReadValue("antiSpawnGridSize", settings.AntiSpawnGridSize);
		loadContext.ReadValue("defendWaypoint", settings.DefendWaypointPrefab);
		loadContext.ReadValue("attackWaypoint", settings.AttackWaypointPrefab);
		loadContext.ReadValue("cycleWaypoint", settings.CycleWaypointPrefab);
		loadContext.ReadValue("patrolWaypoint", settings.PatrolWaypointPrefab);
		loadContext.ReadValue("wanderIntervalInSeconds", settings.WanderIntervalInSeconds);
		
		MergeFactionsInGame(settings.FactionSettings);
		SaveToFile(settings);
		
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