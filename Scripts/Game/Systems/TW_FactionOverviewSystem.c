typedef TW_SettingsManager<ref TW_SpawnSettingsInterface> SpawnSettingsManager;

class TW_SpawnSettingsInterface : TW_SettingsInterface<SpawnSettingsBase>
{
	private ref SpawnSettingsBase _spawnSettings;
	private ref FactionSpawnSettings _selectedFactionSettings;
	private ref ScriptInvoker<ref FactionSpawnSettings> _onFactionSelectionChanged = new ScriptInvoker<ref FactionSpawnSettings>();
	
	ScriptInvoker<ref FactionSpawnSettings> GetOnFactionSelectionChanged() { return _onFactionSelectionChanged; }
	
	override void Initialize(SpawnSettingsBase settings)
	{
		if(settings)
			_spawnSettings = settings;
		else
			_spawnSettings = TW_SpawnManagerBase.GetSpawnSettings();
		
		if(!_spawnSettings)
		{
			Print("TrainWreck: TW_SpawnSettingsInterface -> Spawn settings are null", LogLevel.ERROR);
			return;
		}
		
		if(_selectedFactionSettings != null)
		{
			SelectFaction(_selectedFactionSettings.FactionName);
		}
		else if(_spawnSettings.FactionSettings.Count() > 0)
		{
			_selectedFactionSettings = _spawnSettings.FactionSettings.Get(0);
		}
		
		isInitialized = true;
	}
	
	void SelectFaction(string factionKey)
	{
		if(!_spawnSettings)
		{
			Print("TrainWreck: Unable to select faction. Spawn settings are null", LogLevel.ERROR);
			return;
		}
		
		foreach(FactionSpawnSettings settings : _spawnSettings.FactionSettings)
		{
			if(settings.FactionName == factionKey)
			{
				_selectedFactionSettings = settings;
				GetOnFactionSelectionChanged().Invoke(_selectedFactionSettings);
				return;
			}
		}
		
		PrintFormat("TrainWreck: TW_SpawnSettingsInterface unable to locate faction '%1'", factionKey, LogLevel.WARNING);
		_selectedFactionSettings = null;
		GetOnFactionSelectionChanged().Invoke(null);
	}
	
	FactionSpawnSettings GetFactionSpawnSettings() { return _selectedFactionSettings; }
	SpawnSettingsBase GetSpawnSettings() { return _spawnSettings; }
	
	override void SaveSettings()
	{
		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		player.UpdateFactionSpawnSettings(GetSpawnSettings());
	}
	
	override void ResetToDefault()
	{
		ref SpawnSettingsBase settings = SpawnSettingsBase.GetDefault();
		Initialize(settings);
		SaveSettings();
	}
};