class TW_Settings_Menu: MenuBase
{
	private TW_FactionSpawnSettings_MenuHandler handler;
	
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		
		if(!rootWidget) return;
		
		Widget widget = GetGame().GetWorkspace().CreateWidgets("{11DFF8493E5F6F83}UI/SpawnSettingsLayout.layout"); //.OpenMenu(ChimeraMenuPreset.TW_SpawnSettings);
		handler = TW_FactionSpawnSettings_MenuHandler.Cast(widget.FindHandler(TW_FactionSpawnSettings_MenuHandler));
		widget.SetName("TrainWreckSettings");
		
		
		rootWidget.AddChild(widget);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		SpawnSettingsBase settings;
		
		if(Replication.IsServer())
		{
			settings = TW_SpawnManagerBase.GetInstance().GetSettings();
		}
		else 
			settings = SCR_BaseGameMode.TW_SpawnSettings;
		
		handler.Initialize(settings);
		AddListeners();
	}
	
	
	protected override void OnMenuClose()
	{
		if(!handler) return;
		
		RemoveListeners();
	}
	
	private void AddListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		
		if(!manager) return;
		
		manager.AddActionListener("MenuBack", EActionTrigger.DOWN, SaveAll);
		manager.AddActionListener("DialogConfirm", EActionTrigger.DOWN, Reset);
	}
	
	private void RemoveListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		
		manager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, SaveAll);
		manager.RemoveActionListener("DialogConfirm", EActionTrigger.DOWN, Reset);
	}
	
	private void SaveAll()
	{
		handler.SaveAll();
		Close();
	}
	
	private void Reset()
	{
		handler.Reset();
	}
};