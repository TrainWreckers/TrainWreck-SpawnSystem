class TW_FactionVehiclesMenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_ItemChanceLayout = "{95C8F2B268AEF55D}UI/ItemChance.layout";
	const string s_CheckboxLayout = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	const string s_SliderLayout = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	
	protected Widget m_Root;
	protected Widget m_ContentArea;
	protected SCR_FactionManager _factionManager;
	protected ref FactionSpawnSettings _factionSpawnSettings;

	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		m_ContentArea = m_Root.FindAnyWidget("ContentArea");
		
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		_factionSpawnSettings = interface.GetFactionSpawnSettings();
		
		foreach(VehicleItemChance prefab : _factionSpawnSettings.Vehicles)
		{			
			Widget chanceWidget = GetGame().GetWorkspace().CreateWidgets(s_ItemChanceLayout, m_ContentArea);
			TW_ItemChance_MenuHandler handler = TW_ItemChance_MenuHandler.Cast(chanceWidget.FindHandler(TW_ItemChance_MenuHandler));
			handler.LinkTo(prefab);
		}
	}
};