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
	
	protected ref map<string, ref FactionSpawnSettings> m_FactionSettings = new map<string, ref FactionSpawnSettings>();
	protected ref array<string> m_EnabledFactions = {};
	protected ref map<string, int> m_FactionCounts = new map<string, int>();
	protected ref map<string, float> m_FactionChances = new map<string, float>();
	
	protected ref map<string, ref BehaviorWeights> m_FactionBehaviorWeights = new map<string, ref BehaviorWeights>();
	
	protected SCR_BaseGameMode m_GameMode;
	
	ref SpawnSettingsBase GetSettings() { return m_SpawnSettings; }
	private bool isTrickleSpawning = false;
	private int m_MaxTotalAgents = 0;
	private int m_SpawnQueueCount = 0;
	
	//! Should we render debug statements solely meant to troubleshoot specific problems
	protected bool IsDebug()
	{
		if(!m_SpawnSettings) 
			return false;
		
		return m_SpawnSettings.ShowDebug;
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
	
	void Init(SCR_BaseGameMode gameMode)
	{
		s_Instance = this;
		m_SpawnSettings = SpawnSettingsBase.LoadFromFile();
		m_GameMode = gameMode;
		
		if(!m_SpawnSettings)
		{
			PrintFormat("TrainWreck-SpawnSystem: Cannot find spawn file", LogLevel.WARNING);
			return;
		}
		
		m_MaxTotalAgents = 0;
		
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
				spawnInfo.AddCharacter(item);
			
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
				m_MaxTotalAgents += settings.MaxAmount;
			}
			else 
			{
				PrintFormat("TrainWreck: TW_SpawnManagerBase: '%1' Enabled: %2", settings.FactionName, settings.IsEnabled);	
			}
		
		gameMode.GetOnPositionMonitorChanged().Insert(OnMonitorChanged);
	}
	
	//! Need to make sure the spawn grid matches settings file
	private void RegisterBackgroundTasks()
	{	
		PrintFormat("TrainWreck: TW_SpawnManagerBase -> Background tasks registered");
		GetGame().GetCallqueue().CallLater(WanderLoop, m_SpawnSettings.WanderIntervalInSeconds * 1000, true);
		GetGame().GetCallqueue().CallLater(SpawnLoop, m_SpawnSettings.SpawnTimerInSeconds * 1000, true);
		GetGame().GetCallqueue().CallLater(GarbageCollection, m_SpawnSettings.GarbageCollectionTimerInSeconds * 1000, true);
	}
	
	void SpawnLoop()
	{
		if(GetGame().GetPlayerManager().GetPlayerCount() <= 0)
		{
			Print("TrainWreck-SpawnSystem: No players on map. Skipping spawn...");
			return;
		}
		
		if(isTrickleSpawning)
			return;
		
		ref array<SCR_AIGroup> groups = {};
		int agentCount = GetAgentCount(groups);
		
		if(agentCount >= m_MaxTotalAgents)
			return;
		
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
		if(!spawnPoint) return false;
		string position = TW_Util.ToGridText(spawnPoint.GetOrigin(), m_SpawnSettings.AntiSpawnGridSize);
		return !m_AntiSpawnChunks.Contains(position);
	}
	
	protected FactionKey GetRandomFactionToSpawn()
	{
		ref array<string> factions = new array<string>();
		ref array<float> factionWeights = new array<float>();
		
		FactionSpawnSettings info;
		
		if(m_FactionCounts.IsEmpty())
		{
			foreach(string faction, float weight : m_FactionChances)
			{
				// Should ignore any faction whose weights are set to 0
				if(weight <= 0) 
					continue;
				
				m_FactionCounts.Insert(faction, 0);
				factionWeights.Insert(weight);
				factions.Insert(faction);
			}
			
			int index = SCR_ArrayHelper.GetWeightedIndex(factionWeights, Math.RandomFloat01());
			return factions.Get(index);
		}
		
		foreach(string key, int amount : m_FactionCounts)
		{
			if(!m_FactionSettings.Contains(key))
			{
				continue;
			}
			
			info = m_FactionSettings.Get(key);
			
			if(amount < info.MaxAmount)
			{
				factions.Insert(key);
				factionWeights.Insert(m_FactionChances.Get(key));	
			}
		}
		
		int count = factions.Count();
		if(count > 0)
		{
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
		
		if(!selectedFaction)
			return;
		
		if(IsDebug())
		{
			PrintFormat("TrainWreck: Selected faction to spawn -> '%1'", selectedFaction);
		}
				
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
		
		m_SpawnPointsNearPlayers.Clear();
		
		TW_AISpawnPoint.GetSpawnPointsInChunks(m_PlayerChunks, m_SpawnPointsNearPlayers);
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
		int currentAgents = GetAgentCount(agents, true);
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
			else
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
			}
		}
		
		return count;
	}
	
};