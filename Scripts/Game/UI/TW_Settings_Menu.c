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
		
		handler.m_OnSaved.Insert(Close);
		
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
		
		handler.m_OnSaved.Remove(Close);
		RemoveListeners();
	}
	
	private void AddListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		
		if(!manager) return;
		
		manager.AddActionListener("MenuBack", EActionTrigger.DOWN, handler.SaveAll);
		manager.AddActionListener("DialogConfirm", EActionTrigger.DOWN, handler.Reset);
		manager.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, handler.NavigateLeft);
		manager.AddActionListener("MenuTabRight", EActionTrigger.DOWN, handler.NavigateRight);
	}
	
	private void RemoveListeners()
	{
		InputManager manager = GetGame().GetInputManager();
		if(!manager) return;
		
		manager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, handler.SaveAll);
		manager.RemoveActionListener("DialogConfirm", EActionTrigger.DOWN, handler.Reset);
		manager.AddActionListener("MenuTabLeft", EActionTrigger.DOWN, handler.NavigateLeft);
		manager.AddActionListener("MenuTabRight", EActionTrigger.DOWN, handler.NavigateRight);
	}
};