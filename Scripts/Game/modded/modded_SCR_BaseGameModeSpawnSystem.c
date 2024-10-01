modded class SCR_BaseGameMode
{
	protected override void InitializePositionMonitor()
	{
	 	if(!TW_Global.IsServer(this))
			return;
		
		SpawnSettingsBase settings = m_SpawnManager.GetSettings();
		
		positionMonitor = new TW_MonitorPositions(settings.SpawnGridSize, settings.SpawnGridRadius, settings.AntiSpawnGridSize, settings.AntiSpawnGridRadius);
		m_PositionMonitorUpdateInterval = 2.0;
	}
	
	protected ref TW_SpawnManagerBase m_SpawnManager;
	
	override void EOnInit(IEntity owner)
	{	
		if(TW_Global.IsInRuntime())
			LoadSpawnManager();
		
		super.EOnInit(owner);					
	}
	
	protected void LoadSpawnManager()
	{
		RplComponent rpl = TW<RplComponent>.Find(this);
		if(!(rpl.IsMaster() && rpl.Role() == RplRole.Authority))
			return;
		
		m_SpawnManager = new TW_SpawnManagerBase();
		m_SpawnManager.Init();
	}
}