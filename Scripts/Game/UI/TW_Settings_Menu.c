class TW_Settings_Menu: MenuBase
{
	const string s_FactionSettingsLayout = "{C9D1F6372109FC77}UI/FactionSettings.Layout.layout";
	
	private ref SpawnSettingsBase _settings;
	private Widget _contentArea;
	protected UniformGridLayoutWidget grid;
	
	private SCR_InputButtonComponent saveButton;
	private SCR_InputButtonComponent resetButton;
	
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		
		if(!rootWidget) return;
		_contentArea = rootWidget.FindAnyWidget("ContentArea");
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		_settings = interface.GetSpawnSettings();
		grid = UniformGridLayoutWidget.Cast(_contentArea);
		
		saveButton = SCR_InputButtonComponent.Cast(rootWidget.FindAnyWidget("SaveButton").FindHandler(SCR_InputButtonComponent));
		resetButton = SCR_InputButtonComponent.Cast(rootWidget.FindAnyWidget("ResetButton").FindHandler(SCR_InputButtonComponent));
		
		AddListeners();
		Initialize();
	}
	
	protected void Initialize()
	{
		if(!_settings)
		{
			Print("TrainWreck: Unable to initalize spawn settings menu -> Null reference to spawn settings", LogLevel.WARNING);
			return;
		}
		
		if(!_settings.FactionSettings || _settings.FactionSettings.IsEmpty())
		{
			Print("TrainWreck: Unable to initialize spawn settings menu -> No factions detected", LogLevel.WARNING);
			return;
		}
		
		int currentRow = 0;
		int currentCol = 0;
		int maxCols = 2;
		
		foreach(FactionSpawnSettings factionSettings : _settings.FactionSettings)
		{
			Widget factionWidget = GetGame().GetWorkspace().CreateWidgets(s_FactionSettingsLayout, _contentArea);
			TW_FactionSettings_MenuHandler handler = TW_FactionSettings_MenuHandler.Cast(factionWidget.FindHandler(TW_FactionSettings_MenuHandler));
			handler.Initialize(_settings, factionSettings);
			handler.GetFactionButtonWidget().m_OnClicked.Insert(OnFactionButtonClicked);
			
			UniformGridSlot.SetRow(factionWidget, currentRow);
			UniformGridSlot.SetColumn(factionWidget, currentCol);
			
			currentCol++;
			
			if(currentCol >= maxCols)
			{
				currentCol = 0;
				currentRow++;
			}
		}
	}
	
	protected void OnFactionButtonClicked(SCR_ButtonImageComponent comp)
	{
		string id = comp.GetId();
		
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		ref FactionSpawnSettings selectedSettings = interface.GetFactionSpawnSettings();
		if(!selectedSettings || selectedSettings.FactionName != id)
		{
			PrintFormat("TrainWreck: ID selected: %1", id);
			interface.SelectFaction(id);
		}
		
		if(!interface.GetFactionSpawnSettings())
		{
			Print("TrainWreck: Faction was not selected", LogLevel.ERROR);
			return;
		}
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.TW_FactionOverviewMenuUI);
	}
	
	private void Save()
	{
		Close();
	}
	
	private void Reset()
	{
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		interface.ResetToDefault();
		Close();
	}
	
	protected override void OnMenuClose()
	{		
		RemoveListeners();
	}
	
	private void AddListeners()
	{
		saveButton.m_OnClicked.Insert(Save);
		resetButton.m_OnClicked.Insert(Reset);
		
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.ActivateContext("TrainWreckSettings");
	}
	
	private void RemoveListeners()
	{
		saveButton.m_OnClicked.Remove(Save);
		resetButton.m_OnClicked.Remove(Reset);
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.ResetContext("TrainWreckSettings");
	}
};