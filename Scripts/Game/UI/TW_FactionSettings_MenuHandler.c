/*!
	This component serves as a config for TrainWreck Spawn Settings menu elements.
	Sets the names of widgets where request handlers are attached to in the settings menu layout.
	Has to be attached at the root of the settings menu
*/
class TW_FactionSettings_MenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_FactionName = "FactionName";
	const string s_Button = "Button";
	
	protected Widget m_Root;
	protected SCR_FactionManager factionManager;
	protected ref FactionSpawnSettings _factionSpawnSettings;
	protected ref SpawnSettingsBase _settings;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);		
		m_Root = w;
		GetGame().GetInputManager().ActivateContext("TrainWreckSettings");
	}
	
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetInputManager().ResetContext("TrainWreckSettings");
	}
	
	private SCR_FactionManager GetFactionManager()
	{
		if(factionManager) return factionManager;
		
		factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		return factionManager;
	}
	
	void Initialize(SpawnSettingsBase settings, FactionSpawnSettings spawnSettings)
	{
		_factionSpawnSettings = spawnSettings;
		_settings = settings;
		
		SCR_FactionManager manager = GetFactionManager();
		
		SCR_Faction faction = SCR_Faction.Cast(manager.GetFactionByKey(spawnSettings.FactionName));
		
		if(!faction)
		{
			PrintFormat("TrainWreck: Faction '%1' not found. Unable to add tab for faction settings", spawnSettings.FactionName);
			return;
		}
		
		ResourceName iconPath = faction.GetUIInfo().GetIconPath();
		
		GetFactionNameWidget().SetText(faction.GetFactionName());
		GetFactionButtonWidget().SetImage(iconPath);
		GetFactionButtonWidget().SetId(spawnSettings.FactionName);
	}
	
	private TextWidget GetFactionNameWidget()
	{
		Widget temp = m_wRoot.FindAnyWidget(s_FactionName);
		
		if(!temp) return null;
		return TextWidget.Cast(temp);
	}
	
	SCR_ButtonImageComponent GetFactionButtonWidget()
	{
		Widget temp = m_wRoot.FindAnyWidget(s_Button);
		if(!temp) return null;
		return SCR_ButtonImageComponent.Cast(temp.FindHandler(SCR_ButtonImageComponent));
	}
	
};