modded class SCR_BaseGameMode
{
	protected ref TW_FactionCompositions m_USCompositions;
	protected ref TW_FactionCompositions m_USSRCompositions;
	protected ref TW_FactionCompositions m_FIACompositions;
	
	protected ref TW_AOIConfig _usPlayerSpawnArea;
	protected ref TW_AOIConfig _ussrPlayerSpawnArea;
	protected ref TW_AOIConfig _fiaPlayerSpawnArea;
	
	TW_AOIConfig GetUSPlayerSpawnAreaConfig() 
	{ 
		if(!_usPlayerSpawnArea)
			InitializeCompositions();
		
		return _usPlayerSpawnArea; 
	}
	
	TW_AOIConfig GetUSSRPlayerSpawnAreaConfig() 
	{ 
		if(!_ussrPlayerSpawnArea)
			InitializeCompositions();
		
		return _ussrPlayerSpawnArea; 
	}
	
	TW_AOIConfig GetFIAPlayerSpawnAreaConfig() 
	{ 
		if(!_fiaPlayerSpawnArea)
			InitializeCompositions();
		
		return _fiaPlayerSpawnArea; 
	}
	
	TW_FactionCompositions GetFIACompositions() 
	{ 
		if(!m_FIACompositions)
			InitializeCompositions();
		
		return m_FIACompositions; 
	}
	
	TW_FactionCompositions GetUSSRCompositions() 
	{ 
		if(!m_USSRCompositions)
			InitializeCompositions();
		
		return m_USSRCompositions; 
	}
	
	TW_FactionCompositions GetUSCompositions() 
	{ 
		if(!m_USCompositions)
			InitializeCompositions();
		
		return m_USCompositions; 
	}
	
	override void OnPlayerConnected(int playerId)
	{
		vanilla.OnPlayerConnected(playerId);
		
		if(!Replication.IsServer()) return;
		
		PrintFormat("TrainWreck: Player %1 joined. Broadcasting faction spawn settings", playerId);
		
		// We will broadcast the faction settings to people upon joining
		// Afterwards players should receive updates whenever the settings update
		Rpc(Rpc_Broadcast_FactionSpawnSettings, TW_Util.ToJson(TW_SpawnManagerBase.GetSpawnSettings(), true));
	}
	
	void UpdateFactionSpawnSettings(SpawnSettingsBase settings)
	{
		if(Replication.IsClient())
		{
			Print("TrainWreck: Unable to update faction spawn settings - Invoked from Client");		
			return;
		}
		else
			Print("TrainWreck: Updating Faction Spawn Settings");
		
		if(!SpawnSettingsBase.SaveToFile(settings))
		{
			PrintFormat("TrainWreck: UpdateFactionSpawnSettings<SaveToFile> - Failed. %1", TW_Util.ToJson(settings, true));
			return;
		}

		TW_SpawnManagerBase.GetInstance().LoadSettingsFromFile(settings);
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		interface.Initialize(settings);
		
		string serialized = TW_Util.ToJson(settings, true);
		Print("TrainWreck: UpdateFactionSpawnSettings -> Broadcasting Updates");
		
		Rpc(Rpc_Broadcast_FactionSpawnSettings, serialized);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_Broadcast_FactionSpawnSettings(string settings)
	{
		PrintFormat("TrainWreck: Broadcast Received");
		SpawnSettingsBase spawnSettings = SpawnSettingsBase.LoadFromFile(settings);
		TW_SpawnManagerBase.s_SpawnSettings = spawnSettings;
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		interface.Initialize(spawnSettings);
	}
	
	private void InitializeCompositions()
	{
		m_USCompositions = LoadTWFactionComposition("{0C96E3FCC84E8CD4}Configs/Compositions/TW_US_Compositions.conf");	
		m_USSRCompositions = LoadTWFactionComposition("{B482220F45A754E6}Configs/Compositions/TW_USSR_Compositions.conf");	
		m_FIACompositions = LoadTWFactionComposition("{E37B578DE2724443}Configs/Compositions/TW_FIA_Compositions.conf");
		
		_fiaPlayerSpawnArea = LoadMustSpawnConfig("{44D46A068C64FC16}Configs/AOI/TW_FIA_PlayerSpawnArea.conf");
		_usPlayerSpawnArea = LoadMustSpawnConfig("{A85AFC349D1B80B9}Configs/AOI/TW_US_PlayerSpawnArea.conf");
		_ussrPlayerSpawnArea = LoadMustSpawnConfig("{5C3AA095E0DE6331}Configs/AOI/TW_USSR_PlayerSpawnArea.conf");
	}
	
	private TW_AOIConfig LoadMustSpawnConfig(ResourceName prefab)
	{
		Resource configContainer = BaseContainerTools.LoadContainer(prefab);
		if(!configContainer || !configContainer.IsValid())
			return null;
		
		ref TW_AOIConfig registry = TW_AOIConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configContainer.GetResource().ToBaseContainer()));
		
		if(!registry)
			PrintFormat("TrainWreck-SpawnSystem: Was unable to load config: %1", prefab, LogLevel.ERROR);
		
		return registry;
	}
	
	private TW_FactionCompositions LoadTWFactionComposition(ResourceName prefab)
	{		
		Resource configContainer = BaseContainerTools.LoadContainer(prefab);
		if (!configContainer || !configContainer.IsValid())
			return null;

		ref TW_FactionCompositions registry = TW_FactionCompositions.Cast(BaseContainerTools.CreateInstanceFromContainer(configContainer.GetResource().ToBaseContainer()));
		
		if(!registry)
			PrintFormat("TrainWreck-SpawnSystem: Was unable to load config: %1", prefab, LogLevel.ERROR);
		
		return registry;
	}
	
	protected ref TW_SpawnManagerBase m_SpawnManager;
	
	override void EOnInit(IEntity owner)
	{					
		super.EOnInit(owner);
		
		if(TW_Global.IsInRuntime() && Replication.IsServer())
		{
			Event_OnGamePluginsInitialized.Insert(LoadSpawnManager);
		}
	}
	
	private ref TW_AreaOfInterestHandler aoiHandler;
	private ref TW_RequiredAOIHandler requiredHandler;
	
	protected void LoadSpawnManager()
	{
		RplComponent rpl = TW<RplComponent>.Find(this);
		if(!(rpl.IsMaster() && rpl.Role() == RplRole.Authority))
			return;
		
		Print("TrainWreck: Loading Spawn Manager");	
		m_SpawnManager = new TW_SpawnManagerBase();
		m_SpawnManager.Init(this);
	}
	
	private void OnInvalidPlacement(TW_CompositionPlacementResult result)
	{
		ResourceName prefab = TW_AreaOfInterestHandler.SmallZone;
		
		switch(result.CompositionSize)
		{
			case TW_CompositionSize.LARGE:
			{
				prefab = TW_AreaOfInterestHandler.LargeZone;
				break;
			}
			
			case TW_CompositionSize.MEDIUM:
			{
				prefab = TW_AreaOfInterestHandler.MediumZone;
				break;
			}
		}
		
		result.Prefab = prefab;
		aoiHandler.SpawnFromResult(result);
		
		PrintFormat("TrainWreck-SpawnSystem: Callback - Failed comp placement at '%1'", result.Position, LogLevel.WARNING);				
	}
	
	private void OnSuccessPlacement(TW_CompositionPlacementResult result)
	{
		PrintFormat("TrainWreck_SpawnSystem: Callback - Success comp placement at '%1'", result.Position, LogLevel.WARNING);		
	}
		
	private void FailedReason(TW_AOIFailedReason reason)
	{
		PrintFormat("TrainWreck-SpawnSystem: callback - Failed to spawn AOI: '%1'", SCR_Enum.GetEnumName(TW_AOIFailedReason, reason), LogLevel.ERROR);
	}
	
	//private ref array<ref TW_AreaOfInterestHandler> _handlers = {};
	private void Test()
	{
		
		return;
		/*if(!TW_SpawnManagerBase.GetInstance().GetSettings().CompositionSettings.ShouldSpawnComposition)
			return;
		
		ref TW_CompositionSettings settings = TW_SpawnManagerBase.GetInstance().GetSettings().CompositionSettings;
		
		for(int i = 0; i < settings.CompositionAreaCount; i++)
		{
			ref TW_CompositionSpawnParameters params = new TW_CompositionSpawnParameters(GetUSSRCompositions());
			ref TW_MapLocation location = m_MapManager.GetRandomLocation();
			
			PrintFormat("TrainWreck: Spawning around: %1", WidgetManager.Translate(location.LocationName()));
			
			params.AreaCenter = location.GetPosition();
			params.AreaRadius = (m_MapManager.settings.GetConfigType(location.LocationType()).radius * 2 ) * m_MapManager.settings.gridSize;
			params.MinimumDistanceFromAreaCenter = Math.RandomFloatInclusive(20, 100);
			
			params.MinRadius = settings.AreaRadius.Min;
			params.MaxRadius = settings.AreaRadius.Max;
			
			params.RetriesPerPlacement = 5;
			
			params.SmallCompositions = settings.SmallCount.GetRandomPercentage();
			params.MediumCompositions = settings.MediumCount.GetRandomPercentage();
			params.LargeCompositions = settings.LargeCount.GetRandomPercentage();
			params.DefensiveWalls = settings.WallCount.GetRandomPercentage();
			params.DefensiveBunkers = settings.BunkerCount.GetRandomPercentage();
			params.NextSearchPosition = 1.25;
			
			aoiHandler = TW_AreaOfInterestHandler(params);
			aoiHandler.GetOnAOIFailed().Insert(FailedReason);
			aoiHandler.GetOnCompositionPlacementFailed().Insert(OnInvalidPlacement);
			aoiHandler.GetOnCompositionPlacementSuccess().Insert(OnSuccessPlacement);
			
			if(aoiHandler.StartSpawn())
				PrintFormat("TrainWreck: Successfully spawned something");			
			
			_handlers.Insert(aoiHandler);			
		}*/
	}
}