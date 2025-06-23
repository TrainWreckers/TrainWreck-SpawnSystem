class TW_Layout_SpawnSettingsUI : ChimeraMenuBase
{
	private ref SpawnSettingsBase settings = new SpawnSettingsBase();
	private TW_SpawnSettingsMenu_Handler _handler;
	
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();
		_handler = TW_SpawnSettingsMenu_Handler.Cast(GetRootWidget().FindHandler(TW_SpawnSettingsMenu_Handler));
		_handler.AttachToSpawnSettings(TW_SpawnManagerBase.GetInstance().GetSettings());		
	}	
};