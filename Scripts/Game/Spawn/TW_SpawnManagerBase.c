class FactionSpawnInfo
{
	private ref array<ResourceName> m_Characters = {};
	private ref array<ResourceName> m_Groups = {};
	private ref array<ResourceName> m_Vehicles = {};
	
	void AddCharacter(ResourceName character) { m_Characters.Insert(character); }
	void AddGroup(ResourceName group) { m_Groups.Insert(group); }
	void AddVehicle(ResourceName vehicle) { m_Vehicles.Insert(vehicle); }
		
	ResourceName GetRandomCharacter() { return m_Characters.GetRandomElement(); }
	ResourceName GetRandomGroup() { return m_Groups.GetRandomElement(); }
	ResourceName GetRandomVehicle() { return m_Vehicles.GetRandomElement(); }
	
	array<ResourceName> GetCharacters() { return m_Characters; }
	array<ResourceName> GetGroups() { return m_Groups; }
	array<ResourceName> GetVehicles() { return m_Vehicles; }
};

enum TW_AISpawnBehavior
{
	DefendLocal,
	DefendArea,
	Patrol,
	Attack
};


class TW_SpawnManagerBase
{
	private static TW_SpawnManagerBase s_Instance;
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
	
	protected SCR_BaseGameMode m_GameMode;
	
	ref SpawnSettingsBase GetSettings() { return m_SpawnSettings; }
	private bool isTrickleSpawning = false;
	private int m_MaxTotalAgents = 0;
	private int m_SpawnQueueCount = 0;
	private ref array<TW_AISpawnBehavior> m_Behaviors = {TW_AISpawnBehavior.DefendLocal, TW_AISpawnBehavior.DefendArea, TW_AISpawnBehavior.Attack, TW_AISpawnBehavior.Patrol};

	void Init()
	{
		s_Instance = this;
		m_SpawnSettings = SpawnSettingsBase.LoadFromFile();
		m_MaxTotalAgents = 0;
		
		foreach(FactionSpawnSettings settings : m_SpawnSettings.FactionSettings)
			if(!m_FactionSettings.Contains(settings.FactionName) && settings.IsEnabled)
			{
				m_FactionSettings.Insert(settings.FactionName, settings);
				m_EnabledFactions.Insert(settings.FactionName);
				m_MaxTotalAgents += settings.MaxAmount;
			}
		
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		LoadFactionCharacters();
		GetGame().GetCallqueue().CallLater(ReRegisterSpawnPointGrid, 2500, false);
	}
	
	private void LoadFactionCharacters()
	{
		FactionManager manager = GetGame().GetFactionManager();
		
		ref array<Faction> entries = {};
		manager.GetFactionsList(entries);
		
		foreach(Faction fac : entries)
		{
			SCR_Faction faction = SCR_Faction.Cast(fac);
			
			if(!faction)
				continue;
			
			if(m_Spawnables.Contains(faction.GetFactionKey()))
				continue;
			
			ref FactionSpawnInfo info = new FactionSpawnInfo();
			
			SCR_EntityCatalog characterCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.CHARACTER);
			ref array<SCR_EntityCatalogEntry> items = {};
			
			characterCatalog.GetEntityList(items);
			
			foreach(SCR_EntityCatalogEntry entry : items)
				info.AddCharacter(entry.GetPrefab());
			
			SCR_EntityCatalog groupCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
			items.Clear();
			groupCatalog.GetEntityList(items);
			
			foreach(SCR_EntityCatalogEntry entry : items)
				info.AddGroup(entry.GetPrefab());
			
			SCR_EntityCatalog vehicleCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
			items.Clear();
			vehicleCatalog.GetEntityList(items);
			
			foreach(SCR_EntityCatalogEntry entry : items)
				info.AddVehicle(entry.GetPrefab());
			
			m_Spawnables.Insert(faction.GetFactionKey(), info);
		}
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
	
	//! Need to make sure the spawn grid matches settings file
	private void ReRegisterSpawnPointGrid()
	{
		m_GameMode.GetOnPlayerPositionsUpdated().Insert(OnPlayerChunksUpdated);
		TW_AISpawnPoint.ChangeSpawnGridSize(m_SpawnSettings.SpawnGridSize);
		
		GetGame().GetCallqueue().CallLater(SpawnLoop, m_SpawnSettings.SpawnTimerInSeconds * 1000, true);
		GetGame().GetCallqueue().CallLater(GarbageCollection, m_SpawnSettings.GarbageCollectionTimerInSeconds * 1000, true);
	}
	
	protected bool IsValidSpawn(TW_AISpawnPoint spawnPoint)
	{
		if(!spawnPoint) return false;
		string position = TW_Util.ToGridText(spawnPoint.GetOrigin(), m_SpawnSettings.AntiSpawnGridSize);
		return !m_AntiSpawnChunks.Contains(position);
	}
	
	protected string GetRandomFactionToSpawn()
	{
		ref array<string> factions = new array<string>();
		FactionSpawnSettings info;
		
		if(m_FactionCounts.IsEmpty())
		{
			foreach(string faction : m_EnabledFactions)
				m_FactionCounts.Insert(faction, 0);
			return m_EnabledFactions.GetRandomElement();
		}
			
		foreach(string key, int amount : m_FactionCounts)
		{
			if(!m_FactionSettings.Contains(key))
				continue;
			
			info = m_FactionSettings.Get(key);
			if(amount < info.MaxAmount)
			{
				factions.Insert(key);
			}
		}
		
		int count = factions.Count();
		
		if(count > 0)
			return factions.GetRandomElement();
		
		return string.Empty;		
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
		{
			PrintFormat("TrainWreck-SpawnSettings: '%1' - Not valid faction", selectedFaction, LogLevel.ERROR);
			GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
			return;
		}
		
		TW_AISpawnBehavior behavior = m_Behaviors.GetRandomElement();
		
		if(IsValidSpawn(spawnPoint))
		{			
			ref FactionSpawnSettings spawnSettings = m_FactionSettings.Get(selectedFaction);
			
			if(!spawnSettings)
			{
				PrintFormat("TrainWreck-SpawnSettings: %1 - Not valid faction", selectedFaction, LogLevel.ERROR);
				GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
				return;
			}
			
			float chanceToSpawn = Math.RandomIntInclusive(0, 100);
			
			// Unsuccessful roll to spawn.
			if(chanceToSpawn > spawnSettings.ChanceToSpawn)
			{
				GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
				return;
			}
				
			float tagAsWanderer = Math.RandomFloat01();
			bool tag = tagAsWanderer < spawnSettings.AIWanderChance;
			
			ref FactionSpawnInfo factionSettings = m_Spawnables.Get(selectedFaction);
				
			SCR_AIGroup group = TW_Util.CreateNewGroup(spawnPoint, selectedFaction, factionSettings.GetCharacters(), Math.RandomIntInclusive(1, 3));
			
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
		
		GetGame().GetCallqueue().CallLater(InvokeSpawnOn, 0.15, false, remainingCount);
	}
	
	protected void OnPlayerChunksUpdated(GridUpdateEvent gridEvent)
	{
		m_PlayerChunks.Clear();
		m_AntiSpawnChunks.Clear();
		
		m_PlayerChunks.Copy(gridEvent.GetPlayerChunks());
		m_AntiSpawnChunks.Copy(gridEvent.GetAntiChunks());
		
		TW_AISpawnPoint.GetSpawnPointsInChunks(m_PlayerChunks, m_SpawnPointsNearPlayers);
	}
	
	protected void ProcessForGC(AIAgent agent)
	{
		if(!agent) return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		
		if(group)
		{
			auto characters = group.GetAIMembers();
			FactionKey key = group.GetFaction().GetFactionKey();
			
			m_FactionCounts.Set(key, m_FactionCounts.Get(key) - characters.Count());
			
			foreach(SCR_ChimeraCharacter character : characters)
				SCR_EntityHelper.DeleteEntityAndChildren(character);
			
			SCR_EntityHelper.DeleteEntityAndChildren(agent);
			return;
		}
		
		SCR_ChimeraAIAgent ai = SCR_ChimeraAIAgent.Cast(agent);
		FactionKey key = ai.GetFaction(agent).GetFactionKey();
		
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
		
		PrintFormat("TrainWreck-SpawnSystem: An agent has been removed from group: %1", faction);
		
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
				
				Faction aiFaction = aiAgent.GetFaction(aiAgent);
				
				if(!aiFaction)					 
					continue;
				
				FactionKey key = aiFaction.GetFactionKey();
				
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