class TW_Layout_SpawnSettingsUI : ChimeraMenuBase
{
	private ref SpawnSettingsBase settings = new SpawnSettingsBase();
	private TW_SpawnSettingsMenu_Handler _handler;
	
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();
		_handler = TW_SpawnSettingsMenu_Handler.Cast(GetRootWidget().FindHandler(TW_SpawnSettingsMenu_Handler));
		
		SpawnSettingsBase settings;
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if(TW_Global.IsServer(gameMode))
		{
			settings = TW_SpawnManagerBase.GetInstance().GetSettings();
		}
		else
		{
			settings = SCR_BaseGameMode.TW_SpawnSettings;
		}
		
		_handler.AttachToSpawnSettings(settings);		
	}	
};