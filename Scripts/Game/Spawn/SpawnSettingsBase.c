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
	ResourceName DefendWaypointPrefab;
	
	//! Waypoint to use when groups are set to attack
	ResourceName AttackWaypointPrefab;
	
	//! Waypoint to use when creating patrol waypoints
	ResourceName PatrolWaypointPrefab;
	
	//! Waypoint to use when creating patrols
	ResourceName CycleWaypointPrefab;
	
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
				
		return prettyContainer.SaveToFile(FILENAME);
	}
	
	private static FactionSpawnSettings GetDefaultFactionSettings(SCR_Faction faction)
	{
		ref FactionSpawnSettings factionSettings = new FactionSpawnSettings();
				
		factionSettings.FactionName = faction.GetFactionKey();
		factionSettings.MaxAmount = 30;
		factionSettings.IsEnabled = true;
		factionSettings.AIWanderChance = 0.25;
		factionSettings.AIWanderMaximumPercent = 0.2;
		factionSettings.AIWanderMinimumPercent = 0.01;
		factionSettings.ChanceToSpawn = 50;
		
		return factionSettings;
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
		
		settings.SetData("default", 2, 250, 5, 2, 150, 5.0);
		
		settings.CycleWaypointPrefab = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
		settings.PatrolWaypointPrefab = "{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et";
		settings.DefendWaypointPrefab = "{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et";
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
		
		MergeFactionsInGame(settings.FactionSettings);
		SaveToFile(settings);
		
		return settings;
	}
};

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
	
	//! Minimum percentage of units from faction that'll be assigned wander
	float AIWanderMinimumPercent;
	
	//! Maximum percentage of units from faction that'll be assigned wander
	float AIWanderMaximumPercent;		
};