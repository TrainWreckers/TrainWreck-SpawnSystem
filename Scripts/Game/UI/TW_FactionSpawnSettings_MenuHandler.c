/*!
	This component serves as a config for TrainWreck Spawn Settings menu elements.
	Sets the names of widgets where request handlers are attached to in the settings menu layout.
	Has to be attached at the root of the settings menu
*/
class TW_FactionSpawnSettings_MenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_FactionSettingsLayout = "{C9D1F6372109FC77}UI/FactionSettings.Layout.layout";
	const string s_ItemChanceLayout = "{95C8F2B268AEF55D}UI/ItemChance.layout";
	const string s_CheckboxLayout = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	const string s_SliderLayout = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	
	const string s_ContentArea = "ContentArea";
	const string s_SettingsArea = "SettingsPane";
	const string s_SaveButton = "SaveButton";
	const string s_ResetButton = "ResetButton";
	
	protected Widget m_Root;
	protected Widget m_ContentArea;
	protected Widget m_SettingsArea;
	protected string m_SelectedFactionId = string.Empty;
	
	protected ref SpawnSettingsBase settings = new SpawnSettingsBase();
	protected ref FactionSpawnSettings factionSpawnSettings;
	protected SCR_BaseGameMode gameMode;
	
	protected SCR_InputButtonComponent  saveButton;
	
	ref ScriptInvoker m_OnSaved = new ScriptInvoker(); 
	
	override void HandlerAttached(Widget w)
	{
		gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		super.HandlerAttached(w);
		m_Root = w;
		m_ContentArea = m_Root.FindAnyWidget(s_ContentArea);
		m_SettingsArea = m_Root.FindAnyWidget(s_SettingsArea);
		
		Widget save = m_Root.FindAnyWidget(s_SaveButton);
		if(save)
		{
			saveButton = SCR_InputButtonComponent .Cast(save.FindHandler(SCR_InputButtonComponent ));
			saveButton.m_OnClicked.Insert(SaveAll);
		}
		
		Widget reset = m_Root.FindAnyWidget(s_ResetButton);
		if(reset)
		{
			SCR_InputButtonComponent  resetButton = SCR_InputButtonComponent .Cast(reset.FindHandler(SCR_InputButtonComponent ));
			resetButton.m_OnClicked.Insert(Reset);
		}
		
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		settings = interface.GetSpawnSettings();
	}
	
	void Reset()
	{
		settings = SpawnSettingsBase.GetDefault();
		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		player.UpdateFactionSpawnSettings(settings);
	}
	
	void SaveAll()
	{
		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		player.UpdateFactionSpawnSettings(settings);
		GetGame().GetWorkspace().RemoveChild(m_Root);
		
		if(m_OnSaved)
			m_OnSaved.Invoke();
	}
	
	void AttachToSpawnSettings(SpawnSettingsBase settings)
	{
		if(!GetGame().InPlayMode()) return;
		this.settings = settings;
	}
	
};