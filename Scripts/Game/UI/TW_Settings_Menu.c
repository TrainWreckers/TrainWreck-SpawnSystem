class TW_Settings_Menu: MenuBase
{
	const string s_FactionSettingsLayout = "{C9D1F6372109FC77}UI/FactionSettings.Layout.layout";
	
	private TW_FactionSpawnSettings_MenuHandler handler;
	private ref SpawnSettingsBase _settings;
	private Widget _contentArea;
	protected UniformGridLayoutWidget grid;
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		
		if(!rootWidget) return;
		_contentArea = rootWidget.FindAnyWidget("ContentArea");
		_settings = TW_FactionOverviewSystem.GetSpawnSettings();
		grid = UniformGridLayoutWidget.Cast(_contentArea);
		
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
		
		ref FactionSpawnSettings selectedSettings = TW_FactionOverviewSystem.GetFactionSpawnSettings();
		if(!selectedSettings || selectedSettings.FactionName != id)
		{
			PrintFormat("TrainWreck: ID selected: %1", id);
			TW_FactionOverviewSystem.SelectFaction(id);
		}
		
		if(!TW_FactionOverviewSystem.GetFactionSpawnSettings())
		{
			Print("TrainWreck: Faction was not selected", LogLevel.ERROR);
			return;
		}
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.TW_FactionOverviewMenuUI);
	}
	
	
	protected override void OnMenuClose()
	{
		if(!handler) return;
		
		handler.m_OnSaved.Remove(Close);
		RemoveListeners();
	}
	
	private void AddListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.ActivateContext("TrainWreckSettings");
	}
	
	private void RemoveListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		manager.ResetContext("TrainWreckSettings");
	}
};