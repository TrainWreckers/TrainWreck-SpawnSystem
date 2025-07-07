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
		#else
		if(Replication.IsServer())
			activeSpawnSettings = TW_SpawnManagerBase.GetInstance().GetSettings();
		else 
			activeSpawnSettings = SCR_BaseGameMode.TW_SpawnSettings;
		#endif
		
		if(activeSpawnSettings && activeSpawnSettings.FactionSettings.Count() > 0)
		{
			activeFactionSpawnSettings = activeSpawnSettings.FactionSettings.Get(0);
		}
		
		isInitialized = true;
	}
	
	static void SetSpawnSettings(SpawnSettingsBase settings) 
	{
		string factionName = string.Empty; 
		
		if(GetFactionSpawnSettings() != null)
		{
			factionName = GetFactionSpawnSettings().FactionName;
		}
		
		activeSpawnSettings = settings; 
		
		if(factionName != string.Empty)
		{
			SelectFaction(factionName);
		}
	}
	
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
	
	static void ResetToDefault()
	{
		ref SpawnSettingsBase settings = SpawnSettingsBase.GetDefault();
		SetSpawnSettings(settings);
		SaveSettings();
	}
	
	static void InitSystem()
	{
		if(!isInitialized) Initialize();
	}
};