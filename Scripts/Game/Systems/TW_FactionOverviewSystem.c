class TW_FactionOverviewSystem : GameSystem
{
	protected static ref SpawnSettingsBase activeSpawnSettings;
	protected static ref FactionSpawnSettings activeFactionSpawnSettings; 
	protected static ref ScriptInvoker<ref FactionSpawnSettings> onFactionSelectionChanged = new ScriptInvoker<ref FactionSpawnSettings>();
	protected static bool isInitialized = false;
	
	private static void Initialize()
	{
		Print("TrainWreck: Initialized Faction Overview System");
		#ifdef WORKBENCH 
		activeSpawnSettings = SpawnSettingsBase.LoadFromFile();
		
		if(activeSpawnSettings.FactionSettings.Count() > 0)
		{
			activeFactionSpawnSettings = activeSpawnSettings.FactionSettings.Get(0);
		}
		#else
		if(Replication.IsServer())
			settings = TW_SpawnManagerBase.GetInstance().GetSettings();
		else 
			settings = SCR_BaseGameMode.TW_SpawnSettings;
		#endif
		
		isInitialized = true;
	}
	
	static void SetSpawnSettings(SpawnSettingsBase settings) { activeSpawnSettings = settings; }
	static void SetFactionspawnSettings(FactionSpawnSettings settings) { activeFactionSpawnSettings = settings; }
	
	static void SelectFaction(string factionKey)
	{
		if(!activeSpawnSettings)
		{
			Print("TrainWreck: Unable to select faction. Spawn Settings are null", LogLevel.WARNING);
			return;
		}
		
		foreach(FactionSpawnSettings settings : activeSpawnSettings.FactionSettings)
		{
			if(settings.FactionName == factionKey)
			{
				activeFactionSpawnSettings = settings;
				onFactionSelectionChanged.Invoke(activeFactionSpawnSettings);
				return;
			}
		}
		
		
		PrintFormat("TrainWreck: Unable to locate faction settings for %1", factionKey);
		activeFactionSpawnSettings = null;
	}
	
	static FactionSpawnSettings GetFactionSpawnSettings() { return activeFactionSpawnSettings; }
	static SpawnSettingsBase GetSpawnSettings() { return activeSpawnSettings; }
	
	static void SaveSettings()
	{
		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		player.UpdateFactionSpawnSettings(activeSpawnSettings);
	}
	
	static void InitSystem()
	{
		if(!isInitialized) Initialize();
	}
};