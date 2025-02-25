modded class SCR_BaseGameMode
{
	protected ref TW_FactionCompositions m_USCompositions;
	protected ref TW_FactionCompositions m_USSRCompositions;
	protected ref TW_FactionCompositions m_FIACompositions;
	
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
	
	private void InitializeCompositions()
	{
		m_USCompositions = LoadTWFactionComposition("{0C96E3FCC84E8CD4}Configs/Compositions/TW_US_Compositions.conf");	
		m_USSRCompositions = LoadTWFactionComposition("{B482220F45A754E6}Configs/Compositions/TW_USSR_Compositions.conf");	
		m_FIACompositions = LoadTWFactionComposition("{E37B578DE2724443}Configs/Compositions/TW_FIA_Compositions.conf");
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
	
	protected override void InitializePositionMonitor()
	{
	 	if(!TW_Global.IsServer(this))
			return;
		
		SpawnSettingsBase settings = m_SpawnManager.GetSettings();
		
		positionMonitor = new TW_MonitorPositions(settings.SpawnGridSize, settings.SpawnDistanceInChunks, settings.AntiSpawnGridSize, settings.AntiSpawnDistanceInChunks);
		m_PositionMonitorUpdateInterval = 2.0;
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
	
	protected void LoadSpawnManager()
	{
		RplComponent rpl = TW<RplComponent>.Find(this);
		if(!(rpl.IsMaster() && rpl.Role() == RplRole.Authority))
			return;
		
		Print("TrainWreck: Loading Spawn Manager");	
		m_SpawnManager = new TW_SpawnManagerBase();
		m_SpawnManager.Init(this);
		
		if(!m_USSRCompositions) 
			InitializeCompositions();
		
		GetGame().GetCallqueue().CallLater(Test, 1000 * 5, false);
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
	
	private void Test()
	{
		int count = 15;
		for(int i = 0; i < count; i++)
		{
			ref TW_CompositionSpawnParameters params = new TW_CompositionSpawnParameters(GetUSSRCompositions());
			ref TW_MapLocation location = m_MapManager.GetRandomLocation();
			
			PrintFormat("TrainWreck: Spawning around: %1", WidgetManager.Translate(location.LocationName()));
			
			params.AreaCenter = location.GetPosition();
			params.AreaRadius = (m_MapManager.settings.GetConfigType(location.LocationType()).radius * 2) * m_MapManager.settings.gridSize;
			params.RetriesPerPlacement = 5;
			
			params.SmallCompositions = Math.RandomIntInclusive(1, 5);
			params.MediumCompositions = Math.RandomIntInclusive(1, 5);
			params.LargeCompositions = Math.RandomIntInclusive(0, 3);
			params.DefensiveWalls = Math.RandomIntInclusive(1, 6);
			params.DefensiveBunkers = Math.RandomIntInclusive(1, 5);
			params.MinimumDistanceFromAreaCenter = 25;
			params.NextSearchPosition = 3.5;				
			
			aoiHandler = new TW_AreaOfInterestHandler(params);
			aoiHandler.GetOnAOIFailed().Insert(FailedReason);
			aoiHandler.GetOnCompositionPlacementFailed().Insert(OnInvalidPlacement);
			aoiHandler.GetOnCompositionPlacementSuccess().Insert(OnSuccessPlacement);
			
			if(aoiHandler.StartSpawn())
			{
				PrintFormat("TrainWreck: Successfully spawned something");
				break;
			}
			else 
				PrintFormat("TrainWreck: Iteration %1 of composition spawning", i, LogLevel.ERROR);
			
			aoiHandler.Delete();
			delete aoiHandler;
		}
	}
}