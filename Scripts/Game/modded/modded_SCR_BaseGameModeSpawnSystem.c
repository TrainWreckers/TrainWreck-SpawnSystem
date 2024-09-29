modded class SCR_BaseGameMode
{
	protected override void InitializePositionMonitor()
	{
	 	SpawnSettingsBase settings = m_SpawnManager.GetSettings();
						
		positionMonitor = new TW_MonitorPositions(settings.SpawnGridSize, settings.SpawnGridRadius, settings.AntiSpawnGridSize, settings.AntiSpawnGridRadius);
		m_PositionMonitorUpdateInterval = 2.0;
	}
	
	protected ref TW_SpawnManagerBase m_SpawnManager;
	
	override void EOnInit(IEntity owner)
	{				
		if(!GetGame().InPlayMode() || !IsMaster())
			return;
		
		LoadSpawnManager();
		
		super.EOnInit(owner);					
	}
	
	protected void LoadSpawnManager()
	{
		m_SpawnManager = new TW_SpawnManagerBase();
		m_SpawnManager.Init();
	}
}