class FactionSpawnInfo
{
	private ref array<ResourceName> m_Characters = {};
	private ref array<ResourceName> m_Groups = {};
	
	private ref array<float> m_CharacterChances = {};
	private ref array<float> m_GroupChances = {};
	
	private ref map<TW_VehicleType, ref array<ResourceName>> m_Vehicles = new map<TW_VehicleType, ref array<ResourceName>>();
	private ref map<TW_VehicleType, ref array<float>> m_VehicleChances = new map<TW_VehicleType, ref array<float>>();
	
	void AddCharacter(PrefabItemChance item) 
	{
		m_Characters.Insert(item.PrefabName); 
		m_CharacterChances.Insert(item.Chance);
	}
	
	void AddGroup(PrefabItemChance item) 
	{
		m_Groups.Insert(item.PrefabName); 
		m_GroupChances.Insert(item.Chance);
	}
	
	void AddVehicle(VehicleItemChance item) 
	{ 
		TW_VehicleType type = FromString(item.VehicleType);
		
		if(m_Vehicles.Contains(type))
		{
			m_Vehicles.Get(type).Insert(item.PrefabName);
			m_VehicleChances.Get(type).Insert(item.Chance);
		}
		else
		{
			m_Vehicles.Insert(type, {item.PrefabName});
			m_VehicleChances.Insert(type, {item.Chance});
		}
	}
	
	TW_VehicleType FromString(string type)
	{
		if(type == SCR_Enum.GetEnumName(TW_VehicleType, TW_VehicleType.Air))
			return TW_VehicleType.Air;
		if(type == SCR_Enum.GetEnumName(TW_VehicleType, TW_VehicleType.Small))
			return TW_VehicleType.Small;
		if(type == SCR_Enum.GetEnumName(TW_VehicleType, TW_VehicleType.Medium))
			return TW_VehicleType.Medium;
		return TW_VehicleType.Large;
	}
	
	private ref array<int> _vehicleValues = {};
	TW_VehicleType GetRandomTypeFromFlag(TW_VehicleType flags)
	{
		ref array<TW_VehicleType> selected = {};		
		if(_vehicleValues.IsEmpty())
		{
			SCR_Enum.GetEnumValues(TW_VehicleType, _vehicleValues);						
		}		
		
		foreach(TW_VehicleType val : _vehicleValues)
		{
			if(SCR_Enum.HasFlag(flags, val))
				selected.Insert(val);
		}
		
		if(selected.IsEmpty())
			return TW_VehicleType.Small;
		
		return selected.GetRandomElement();
	}
		
	ResourceName GetRandomCharacter() 
	{ 
		int index = SCR_ArrayHelper.GetWeightedIndex(m_CharacterChances, Math.RandomFloat01());
		return m_Characters.Get(index);
	}
	
	ResourceName GetRandomGroup() 
	{ 
		int index = SCR_ArrayHelper.GetWeightedIndex(m_GroupChances, Math.RandomFloat01());
		return m_Groups.Get(index);
	}
	
	ResourceName GetRandomVehicle(TW_VehicleType type) 
	{ 
		if(!m_Vehicles.Contains(type))
			return ResourceName.Empty;
		
		int index = SCR_ArrayHelper.GetWeightedIndex(m_VehicleChances.Get(type), Math.RandomFloat01());
		return m_Vehicles.Get(type).Get(index);
	}
};

class BehaviorWeights
{
	ref array<ref TW_AISpawnBehavior> Behaviors = {};
	ref array<float> Weights = {};
};

enum TW_AISpawnBehavior
{
	DefendLocal,
	DefendArea,
	Patrol,
	Attack,
	NoWaypoints
};


class TW_SpawnManagerBase
{
	private static ref TW_SpawnManagerBase s_Instance;
	static TW_SpawnManagerBase GetInstance()
	{
		return s_Instance;
	}
	
	protected ref SpawnSettingsBase m_SpawnSettings;
	protected ref map<string, ref array<SCR_AIGroup>> m_FactionGroups = new map<string, ref array<SCR_AIGroup>>();
	protected ref map<string, ref array<SCR_ChimeraAIAgent>> m_FactionAgents = new map<string, ref array<SCR_ChimeraAIAgent>>();
	
	protected ref map<string, ref FactionSpawnInfo> m_Spawnables = new map<string, ref FactionSpawnInfo>();
	
	protected ref set<string> m_PlayerChunks = new set<string>();
	protected ref set<string> m_AntiSpawnChunks = new set<string>();
	
	protected ref array<TW_AISpawnPoint> m_SpawnPointsNearPlayers = {};
	protected ref array<TW_VehicleSpawnPoint> m_VehicleSpawnPointsNearPlayers = {};
	
	protected ref map<string, ref FactionSpawnSettings> m_FactionSettings = new map<string, ref FactionSpawnSettings>();
	protected ref array<string> m_EnabledFactions = {};
	protected ref map<string, int> m_FactionCounts = new map<string, int>();
	protected ref map<string, float> m_FactionChances = new map<string, float>();
	
	protected ref map<string, ref BehaviorWeights> m_FactionBehaviorWeights = new map<string, ref BehaviorWeights>();
	
	protected SCR_BaseGameMode m_GameMode;
	
	ref SpawnSettingsBase GetSettings() { return m_SpawnSettings; }
	private bool isTrickleSpawning = false;
	private int m_SpawnQueueCount = 0;
	
	//! Should we render debug statements solely meant to troubleshoot specific problems
	protected bool IsDebug()
	{
		if(!m_SpawnSettings) 
			return false;
		
		return m_SpawnSettings.ShowDebug;
	}
	
	private ref set<string> __spawnGridKeys = new set<string>();
	private ref set<string> __antiGridKeys = new set<string>();
		
	protected void OnSettingsChanged(string key)
	{
		if(key == "SpawnGridSize")
		{
			TW_AISpawnPoint.ChangeSpawnGridSize(m_SpawnSettings.SpawnGridSize);
			TW_VehicleSpawnPoint.ChangeSpawnGridSize(m_SpawnSettings.SpawnGridSize);
		}	
	}
	
	private void PrintSettings()
	{
		if(!IsDebug()) 
			return;
		
		foreach(string factionName, FactionSpawnSettings settings : m_FactionSettings)
		{		
			PrintFormat("Faction: %1", factionName);
				
			foreach(PrefabItemChance item : settings.Characters)			
			{
				PrintFormat("\tPrefab: %1\n\tChance: %2\n\n", item.PrefabName, item.Chance);	
			}
		}
	}
	
	private bool hasInitialized = false;
	void LoadSettingsFromFile(SpawnSettingsBase overrideSettings = null)
	{
		m_FactionBehaviorWeights.Clear();
		m_FactionChances.Clear();
		m_Spawnables.Clear();
		m_FactionSettings.Clear();
		m_EnabledFactions.Clear();
		
		if(overrideSettings)
			m_SpawnSettings = overrideSettings;
		else
			m_SpawnSettings = SpawnSettingsBase.LoadFromFile();
		
		
		SCR_BaseGameMode.TW_SpawnSettings = m_SpawnSettings;
		
		if(!m_SpawnSettings)
		{
			PrintFormat("TrainWreck-SpawnSystem: Cannot find spawn file", LogLevel.WARNING);
			return;
		}
		
		m_SpawnSettings.GetOnChanged().Insert(OnSettingsChanged);
		
		foreach(FactionSpawnSettings settings : m_SpawnSettings.FactionSettings)
		{
			if(m_Spawnables.Contains(settings.FactionName))
			{
				PrintFormat("TrainWreck: TW_SpawnManagerBase -> Skipping '%1', spawnables already tracking it", settings.FactionName, LogLevel.WARNING);
				continue;
			}
			
			ref BehaviorWeights behaviorWeights = new BehaviorWeights();
			
			foreach(string name, float chance : settings.Behaviors)
			{
				PrintFormat("TrainWreck: TW_SpawnManagerBase '%1': %2", name, chance);
				
				if(name == SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.Patrol))
				{
					behaviorWeights.Behaviors.Insert(TW_AISpawnBehavior.Patrol);
					behaviorWeights.Weights.Insert(chance);
				}
				else if(name == SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.DefendLocal))
				{
					behaviorWeights.Behaviors.Insert(TW_AISpawnBehavior.DefendLocal);
					behaviorWeights.Weights.Insert(chance);
				}
				else if(name == SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.DefendArea))
				{
					behaviorWeights.Behaviors.Insert(TW_AISpawnBehavior.DefendArea);
					behaviorWeights.Weights.Insert(chance);
				}
				else if(name == SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.Attack))
				{
					behaviorWeights.Behaviors.Insert(TW_AISpawnBehavior.Attack);
					behaviorWeights.Weights.Insert(chance);
				}
				else if(name == SCR_Enum.GetEnumName(TW_AISpawnBehavior, TW_AISpawnBehavior.NoWaypoints))
				{
					behaviorWeights.Behaviors.Insert(TW_AISpawnBehavior.NoWaypoints);
					behaviorWeights.Weights.Insert(chance);
				}
			}
			
			m_FactionBehaviorWeights.Insert(settings.FactionName, behaviorWeights);
			
			FactionSpawnInfo spawnInfo = new FactionSpawnInfo();
			
			foreach(PrefabItemChance item : settings.Characters)
			{
				if(item.Chance > 0) 
					PrintFormat("TrainWreck:  CharacterChance(%1): %2 (Chance: %3)", settings.FactionName, item.PrefabName, item.Chance);
				
				spawnInfo.AddCharacter(item);
			}
			
			foreach(VehicleItemChance item : settings.Vehicles)
			{
				TW_Util.GetEntitySize(item.PrefabName);
				spawnInfo.AddVehicle(item);
			}
			
			foreach(PrefabItemChance item : settings.Groups)
				spawnInfo.AddGroup(item);
			
			if(!m_FactionSettings.Contains(settings.FactionName))
				m_FactionChances.Insert(settings.FactionName, settings.ChanceToSpawn);
			
			m_Spawnables.Insert(settings.FactionName, spawnInfo);	
			PrintFormat("TrainWreck: TW_SpawnManagerBase -> '%1' added to Spawnables", settings.FactionName);
			
			if(IsDebug())
			{
				GetGame().GetCallqueue().CallLater(PrintSettings, 30 * 1000, false);
			}
		}
		
		FactionManager manager = GetGame().GetFactionManager();
		
		if(!manager)
		{
			Print("TrainWreck-SpawnSystem: Unable to initialize. FactionManager not found", LogLevel.ERROR);
			return;
		}
		
		foreach(FactionSpawnSettings settings : m_SpawnSettings.FactionSettings)
			if(!m_FactionSettings.Contains(settings.FactionName) && settings.IsEnabled)
			{			
				if(!manager.GetFactionByKey(settings.FactionName))
				{
					PrintFormat("TrainWreck-SpawnSystem: %1 - either mod not loaded, or faction not available in this scenario", settings.FactionName, LogLevel.WARNING);
					continue;
				}	
			
				PrintFormat("TrainWreck: TW_SpawnManagerBase: '%1' Enabled with max amount of %2", settings.FactionName, settings.MaxAmount);
			
				m_FactionSettings.Insert(settings.FactionName, settings);
				m_EnabledFactions.Insert(settings.FactionName);
			}
			else 
			{
				PrintFormat("TrainWreck: TW_SpawnManagerBase: '%1' Enabled: %2", settings.FactionName, settings.IsEnabled);	
			}
		
		if(!hasInitialized)
		{
			m_GameMode.GetOnPositionMonitorChanged().Insert(OnMonitorChanged);
			hasInitialized = true;
		}
	}
	
	void Init(SCR_BaseGameMode gameMode)
	{
		s_Instance = this;
		m_GameMode = gameMode;
		LoadSettingsFromFile();
		SpawnPlayerBases();
	}
	
	private ref TW_RequiredAOIHandler _usPlayerAOI;
	private ref TW_RequiredAOIHandler _ussrPlayerAOI;
	private ref TW_RequiredAOIHandler _fiaPlayerAOI;
	
	private void SpawnPlayerBases()
	{		
		if(GetSettings().ShouldSpawnUSPlayerBase)
		{
			_usPlayerAOI = new TW_RequiredAOIHandler(m_GameMode.GetUSPlayerSpawnAreaConfig());
			
			if(_usPlayerAOI.StartSpawn())
				PrintFormat("TrainWreck: Spawn started");
			else 
				PrintFormat("TrainWreck: Spawn for required AOI failed", LogLevel.ERROR);		
		}	
		
		if(GetSettings().ShouldSpawnUSSRPlayerBase)
		{
			_ussrPlayerAOI = new TW_RequiredAOIHandler(m_GameMode.GetUSSRPlayerSpawnAreaConfig());
			
			if(_ussrPlayerAOI.StartSpawn())
				PrintFormat("TrainWreck: Spawn started");
			else 
				PrintFormat("TrainWreck: Spawn for required AOI failed", LogLevel.ERROR);		
		}
		
		if(GetSettings().ShouldSpawnFIAPlayerBase)
		{
			_fiaPlayerAOI = new TW_RequiredAOIHandler(m_GameMode.GetFIAPlayerSpawnAreaConfig());	
			
			if(_fiaPlayerAOI.StartSpawn())
				PrintFormat("TrainWreck: Spawn started");
			else 
				PrintFormat("TrainWreck: Spawn for required AOI failed", LogLevel.ERROR);		
		}
	}		
	
	//! Need to make sure the spawn grid matches settings file
	private void RegisterBackgroundTasks()
	{	
		PrintFormat("TrainWreck: TW_SpawnManagerBase -> Background tasks registered");
		GetGame().GetCallqueue().CallLater(WanderLoop, m_SpawnSettings.WanderIntervalInSeconds * 1000, true);
		GetGame().GetCallqueue().CallLater(SpawnLoop, m_SpawnSettings.SpawnTimerInSeconds * 1000, true);
		GetGame().GetCallqueue().CallLater(GarbageCollection, m_SpawnSettings.GarbageCollectionTimerInSeconds * 1000, true);
		
		if(m_SpawnSettings.VehicleSpawnSettings && m_SpawnSettings.VehicleSpawnSettings.ShouldSpawnVehicles)
			GetGame().GetCallqueue().CallLater(VehicleLoop, m_SpawnSettings.SpawnTimerInSeconds * 1000, true);
	}
	
	private static ref map<string, TW_VehicleType> _vehicleTypeMap = new map<string, TW_VehicleType>();
	private static ref array<string> _vehicleTypeNames = {};
	
	void VehicleLoop()
	{		
		if(GetGame().GetPlayerManager().GetPlayerCount() <= 0)
			return;
		
		TW_VehicleSpawnPoint.ManageVehicles(m_PlayerChunks, m_SpawnSettings.VehicleSpawnSettings.DeleteIfMoreThanChunksAway);
		
		if(m_VehicleSpawnPointsNearPlayers.IsEmpty())
			return;				
		
		if(_vehicleTypeMap.Count() == 0)
		{
			ref array<int> values = {};
			SCR_Enum.GetEnumValues(TW_VehicleType, values);
			
			foreach(int val : values)
			{
				string name = SCR_Enum.GetEnumName(TW_VehicleType, val);
				_vehicleTypeMap.Insert(name, val);
				_vehicleTypeNames.Insert(name);
			}
		}
				
		int amount = Math.RandomIntInclusive(2, 10);
		
		for(int i = 0; i < amount; i++)
		{
			// Pick a random vehicle spawn point
			string typeName = _vehicleTypeNames.GetRandomElement();
			TW_VehicleType type = _vehicleTypeMap.Get(typeName);
			
			if(m_VehicleSpawnPointsNearPlayers.IsEmpty())
				break;
			
			TW_VehicleSpawnPoint spawnPoint = m_VehicleSpawnPointsNearPlayers.GetRandomElement();
			m_VehicleSpawnPointsNearPlayers.RemoveItem(spawnPoint);
			
			if(!spawnPoint.IsActive())
				continue;
			
			TW_VehicleType vehicleFlags = spawnPoint.GetAllowedVehicleTypes();			
			string factionName = m_EnabledFactions.GetRandomElement();
			FactionSpawnInfo settings = m_Spawnables.Get(factionName);
			
			TW_VehicleType randomType = settings.GetRandomTypeFromFlag(vehicleFlags);
			ResourceName vehiclePrefab = settings.GetRandomVehicle(randomType);
			
			IEntity vehicle;
			if(!spawnPoint.SpawnVehicle(vehiclePrefab, vehicle, m_SpawnSettings.VehicleSpawnSettings.VehicleChanceToSpawn))
				continue;
			
			TW_VehicleSpawnPoint.s_VehicleGrid.RemoveCoord(spawnPoint.GetOrigin());			
			TW_Util.ApplyRandomDamageToEntity(vehicle);		
		}
		
		m_VehicleSpawnPointsNearPlayers.Clear();
	}	
	
	void SpawnLoop()
	{				
		if(GetGame().GetPlayerManager().GetPlayerCount() <= 0)
		{
			if(IsDebug())
				Print("TrainWreck: No Players active -> Skipping spawn logic");
		}
		
		if(m_SpawnPointsNearPlayers.IsEmpty())
		{
			if(IsDebug())
			{
				Print("TrainWreck: No spawn points near players? Doubtful... something may have broke", LogLevel.WARNING);
			}
		}
		
		if(isTrickleSpawning)
		{
			if(IsDebug())
				Print("TrainWreck: Is Trickle Spawning...");
			return;
		}
		
		ref array<SCR_AIGroup> groups = {};
		int agentCount = GetAgentCount(groups);
		
		int spawnCount = Math.RandomIntInclusive(0, 10);
		m_SpawnQueueCount += spawnCount;
		
		if(spawnCount > 0)
			isTrickleSpawning = true;
		
		GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, spawnCount);
	}
	
	
	private void OnMonitorChanged(TW_MonitorPositions monitor)
	{
		PrintFormat("TrainWreck: Spawn System - Received new Monitor");
		m_GameMode.GetOnPlayerPositionsUpdated().Insert(OnPlayerChunksUpdated);
		TW_AISpawnPoint.ChangeSpawnGridSize(m_SpawnSettings.SpawnGridSize);
		TW_VehicleSpawnPoint.ChangeSpawnGridSize(m_SpawnSettings.SpawnGridSize);
		
		RegisterBackgroundTasks();
	}
	
	protected void WanderLoop()
	{		
		foreach(string faction, ref array<SCR_AIGroup> groups : m_FactionGroups)
		{
			if(!groups) continue;
			
			int count = groups.Count();
			
			for(int i = 0; i < count; i++)						
			{				
				if(i < 0 || groups.IsEmpty())
					break;
				
				SCR_AIGroup group = groups.Get(i);
				
				if(!group || !group.IsWanderer())
					continue;

				TW_AISpawnPoint point = m_SpawnPointsNearPlayers.GetRandomElement();
				GetGame().GetCallqueue().CallLater(CreateNewWaypointForGroup, i * 250, false, group, point);				
			}
		}
	}
	
	private void CreateNewWaypointForGroup(SCR_AIGroup group, TW_AISpawnPoint point)
	{
		if(!group)
			return;
		
		ref array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		foreach(AIWaypoint waypoint : waypoints)
			group.RemoveWaypoint(waypoint);
		
		AIWaypoint waypoint = TW_Util.CreateWaypointAt(m_SpawnSettings.DefendWaypointPrefab, point.GetOrigin());				
		group.AddWaypoint(waypoint);
	}
	
	protected bool IsValidSpawn(TW_AISpawnPoint spawnPoint)
	{
		if(!spawnPoint || !spawnPoint.IsActive()) return false;
		
		string position = TW_Util.ToGridText(spawnPoint.GetOrigin(), m_SpawnSettings.AntiSpawnGridSize);
		return !m_AntiSpawnChunks.Contains(position);
	}
	
	protected FactionKey GetRandomFactionToSpawn()
	{
		if(m_FactionCounts.IsEmpty())
		{
			if(IsDebug())
				PrintFormat("TrainWreck: Faction counts are empty. Cannot pick faction", LogLevel.WARNING);
			return FactionKey.Empty;
		}
			
		ref array<string> factions = new array<string>();
		ref array<float> factionWeights = new array<float>();
		
		FactionSpawnSettings info;				
		
		foreach(string key, int amount : m_FactionCounts)
		{
			if(!m_FactionSettings.Contains(key))
			{
				if(IsDebug())
				{
					PrintFormat("TrainWreck: Faction settings for '%1' not found. Skipping for spawn eligibility", key);
				}
				continue;
			}
			
			info = m_FactionSettings.Get(key);
			
			if(IsDebug())
			{
				PrintFormat("TrainWreck: Faction '%1' should spawn around %2 units: has %3 right now", key, info.MaxAmount, amount);
			}
			
			if(amount < info.MaxAmount)
			{
				factions.Insert(key);
				factionWeights.Insert(m_FactionChances.Get(key));	
				
				if(IsDebug())
				{
					PrintFormat("TrainWreck: %1 has less than max amount of %2", key, info.MaxAmount);
				}
			}
		}
		
		int count = factions.Count();
		if(count > 0)
		{
			if(IsDebug())
			{
				PrintFormat("TrainWreck: Factions eligible to spawn: %1", SCR_StringHelper.Join(", ", factions));
			}
			
			int index = SCR_ArrayHelper.GetWeightedIndex(factionWeights, Math.RandomFloat01());
			return factions.Get(index);
		}
		
		return FactionKey.Empty;		
	}
	
	protected void InvokeSpawnOn(int remainingCount)
	{
		if(remainingCount <= 0)
		{
			isTrickleSpawning = false;
			return;
		}
		else 
			remainingCount -= 1;
		
		if(m_SpawnPointsNearPlayers.IsEmpty())
		{
			isTrickleSpawning = false;
			return;
		}
		
		TW_AISpawnPoint spawnPoint = m_SpawnPointsNearPlayers.GetRandomElement();
		
		ResourceName waypointOverride = ResourceName.Empty;
		IEntity positionOverride = null;
		
		string selectedFaction = GetRandomFactionToSpawn();
		
		if(selectedFaction == FactionKey.Empty || !selectedFaction)
		{
			isTrickleSpawning = false;
			return;
		}
		
		if(IsDebug())
			PrintFormat("TrainWreck: Selected faction to spawn -> '%1'", selectedFaction);
				
		if(IsValidSpawn(spawnPoint))
		{			
			ref FactionSpawnSettings spawnSettings = m_FactionSettings.Get(selectedFaction);
			
			if(!spawnSettings)
			{
				GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
				return;
			}
			
			float tagAsWanderer = Math.RandomFloat01();
			bool tag = tagAsWanderer < spawnSettings.AIWanderChance;
			
			ref FactionSpawnInfo factionSettings = m_Spawnables.Get(selectedFaction);
			int groupSize = Math.RandomIntInclusive(1, m_SpawnSettings.GroupSize);
			ResourceName characterPrefab = factionSettings.GetRandomCharacter();
			
			if(IsDebug())
			{
				PrintFormat("TrainWreck: Tag(%1), GroupSize(%2), Prefab(%3), Faction(%4)", tag, groupSize, characterPrefab, selectedFaction);
			}
			
			SCR_AIGroup group = TW_Util.CreateNewGroup(spawnPoint, selectedFaction, characterPrefab, groupSize);
			
			if(!group)
			{
				PrintFormat("TrainWreck: Was unable to spawn group('%1') for faction %2", characterPrefab, selectedFaction);		
				GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
				return;		
			}
			
			group.SetWanderer(tag);
			
			// This unit is managed by our spawn system
			group.SetIsManagedBySpawnSystem(true);
			
			GetGame().GetCallqueue().CallLater(SetGroupWaypoint, 500, false, group);			
		}
		
		GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
	}
	
	protected void SetGroupWaypoint(SCR_AIGroup group)
	{
		if(!group) return;
		FactionKey selectedFaction = group.GetFaction().GetFactionKey();
		
		if(!m_FactionBehaviorWeights.Contains(selectedFaction))
			return;
		
		ref BehaviorWeights weights = m_FactionBehaviorWeights.Get(selectedFaction);
		
		int behaviorIndex = SCR_ArrayHelper.GetWeightedIndex(weights.Weights, Math.RandomFloat01());
		TW_AISpawnBehavior behavior = weights.Behaviors.Get(behaviorIndex);
		
		// Do not assign a waypoint if no waypoints is selected
		if(TW_AISpawnBehavior.NoWaypoints == behavior)
			return;
		
		ResourceName waypoint = ResourceName.Empty;
		vector position = group.GetOrigin();
		
		switch(behavior)
		{
			case TW_AISpawnBehavior.Patrol:	
			{
				int waypointCount = Math.RandomIntInclusive(5, 15);
				TW_Util.CreatePatrolPathFor(group, m_SpawnSettings.PatrolWaypointPrefab, m_SpawnSettings.CycleWaypointPrefab, waypointCount, Math.RandomFloat(150, 600));
				break;
			}
			
			case TW_AISpawnBehavior.Attack:
			{
				waypoint = m_SpawnSettings.AttackWaypointPrefab;
				position = m_SpawnPointsNearPlayers.GetRandomElement().GetOrigin();
				break;
			}
			
			case TW_AISpawnBehavior.DefendLocal:
			{
				waypoint = m_SpawnSettings.DefendWaypointPrefab;					
				break;
			}
			
			case TW_AISpawnBehavior.DefendArea:
			{
				waypoint = m_SpawnSettings.DefendWaypointPrefab;
				position = m_SpawnPointsNearPlayers.GetRandomElement().GetOrigin();
				break;
			}
		}
		
		if(waypoint)
		{
			AIWaypoint point = TW_Util.CreateWaypointAt(waypoint, position);
			group.AddWaypoint(point);
		}
		
		if(!m_FactionCounts.Contains(selectedFaction))
			m_FactionCounts.Insert(selectedFaction, group.GetAgentsCount());
		else
			m_FactionCounts.Set(selectedFaction, m_FactionCounts.Get(selectedFaction) + group.GetAgentsCount());
	}
	
	protected void OnPlayerChunksUpdated(GridUpdateEvent gridEvent)
	{
		m_PlayerChunks.Clear();
		m_AntiSpawnChunks.Clear();
		
		m_PlayerChunks.Copy(gridEvent.GetPlayerChunks());
		m_AntiSpawnChunks.Copy(gridEvent.GetAntiChunks());
		
		int before = m_SpawnPointsNearPlayers.Count();
		
		m_SpawnPointsNearPlayers.Clear();
		m_VehicleSpawnPointsNearPlayers.Clear();
		
		TW_AISpawnPoint.GetSpawnPointsInChunks(m_PlayerChunks, m_SpawnPointsNearPlayers);
		TW_VehicleSpawnPoint.GetSpawnPointsInChunks(m_PlayerChunks, m_VehicleSpawnPointsNearPlayers);
		
		if(IsDebug())
		{
			PrintFormat("TrainWreck: Updating player chunks. Count Before (%1) -> After (%2)", before, m_SpawnPointsNearPlayers.Count());
		}
	}
	
	protected void ProcessForGC(AIAgent agent)
	{
		if(!agent) return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		
		if(group)
		{
			// We should not delete units that are not managed by our spawn system
			if(!group.IsManagedBySpawnSystem())
				return;
			
			auto characters = group.GetAIMembers();
			FactionKey key = group.GetFaction().GetFactionKey();
			
			m_FactionCounts.Set(key, m_FactionCounts.Get(key) - characters.Count());
			
			foreach(SCR_ChimeraCharacter character : characters)
				SCR_EntityHelper.DeleteEntityAndChildren(character);
			
			SCR_EntityHelper.DeleteEntityAndChildren(agent);
			return;
		}
		
		SCR_ChimeraAIAgent ai = SCR_ChimeraAIAgent.Cast(agent);		
		FactionKey key = TW_Util.GetFactionKeyFromAgent(ai);
		
		m_FactionCounts.Set(key, m_FactionCounts.Get(key) - 1);
		
		if(ai)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(agent);
		}
	}
	
	/*!
		Rather than doing distance checks per entity, we're going to do a grid square check. If they're
		not in an active chunk, we'll delete
	*/
	void GarbageCollection()
	{
		ref array<SCR_AIGroup> agents = {};
		int currentAgents = GetAgentCount(agents, false);
		int queuedForGC = 0;
		int vehiclesQueuedForGC = 0;
		
		if(currentAgents <= 0)
			return;
		
		int x, y;
		string positionText;
		
		foreach(int agentN, SCR_AIGroup agent : agents)
		{
			if(!agent)
				continue;
			
			if(!agent.IsManagedBySpawnSystem())
				continue;
			
			positionText = TW_Util.ToGridText(agent.GetOrigin(), m_SpawnSettings.SpawnGridSize);
			
			// If the chunk is not loaded --> delete
			if(!m_PlayerChunks.Contains(positionText))
			{
				GetGame().GetCallqueue().CallLater(ProcessForGC, agentN * 150, false, agent);
				queuedForGC++;
			}
		}
	}
	
	void OnAIDeath(SCR_AIGroup group, AIAgent deadAgent)
	{
		if(!group) return;
		
		string faction = group.GetFaction().GetFactionKey();
		
		if(!m_FactionCounts.Contains(faction))
			return;
		
		m_FactionCounts.Set(faction, Math.ClampInt(m_FactionCounts.Get(faction) - 1, 0, 500));
	}
	
	//! Populate FactionGroups and FactionAgents, while outputting groups
	int GetAgentCount(out notnull array<SCR_AIGroup> groups, bool includeAll = false)
	{
		m_FactionGroups.Clear();
		m_FactionAgents.Clear();
		m_FactionCounts.Clear();
		
		foreach(string key : m_EnabledFactions)
			if(m_FactionCounts.Contains(key))
				m_FactionCounts.Set(key, 0);
			else
				m_FactionCounts.Insert(key, 0);
		
		ref array<AIAgent> worldAgents = {};
		
		// We want to ensure we're only getting AI we care about... specifically NOT BIRDS
		GetGame().GetAIWorld().GetAIAgents(worldAgents);
		
		int count = 0;
		foreach(auto agent : worldAgents)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(agent);
			
			if(group)
			{
				if(group.GetPlayerCount() > 0)
					continue;
				
				// If this group is NOT managed by our spawn system -> ignore
				if(!group.IsManagedBySpawnSystem())
					continue;
				
				FactionKey key = group.GetFaction().GetFactionKey();
				
				if(!m_FactionGroups.Contains(key))
					m_FactionGroups.Insert(key, {});							
				
				if(includeAll || !group.IgnoreGlobalCount())
				{
					m_FactionGroups.Get(key).Insert(group);	
					groups.Insert(group);
					
					if(!m_FactionCounts.Contains(key))
						m_FactionCounts.Set(key, group.GetAgentsCount());
					else
						m_FactionCounts.Set(key, m_FactionCounts.Get(key) + group.GetAgentsCount());
					
					count += group.GetAgentsCount();
				}
			}
			/*else
			{
				SCR_ChimeraAIAgent aiAgent = SCR_ChimeraAIAgent.Cast(agent);
				
				if(!aiAgent)
					continue;
				
				FactionKey key= TW_Util.GetFactionKeyFromAgent(aiAgent);
				
				if(key == FactionKey.Empty)
					continue;
				
				if(!m_FactionAgents.Contains(key))
					m_FactionAgents.Insert(key, {});
				
				if(!m_FactionCounts.Contains(key))
					m_FactionCounts.Insert(key, 1);
				else
					m_FactionCounts.Set(key, m_FactionCounts.Get(key) + 1);
				
				m_FactionAgents.Get(key).Insert(aiAgent);
				count++;
			}*/
		}
		
		if(IsDebug())
			foreach(string key, int amount : m_FactionCounts)
				PrintFormat("TrainWreck: Current count for %1 is %2", key, amount);
		
		return count;
	}
	
};