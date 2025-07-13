class TW_FactionUnitsMenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_ItemChanceLayout = "{95C8F2B268AEF55D}UI/ItemChance.layout";
	const string s_CheckboxLayout = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	const string s_SliderLayout = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	
	protected Widget m_Root;
	protected SCR_FactionManager factionManager;
	protected ref FactionSpawnSettings _factionSpawnSettings;
	protected Widget _contentArea;
	
	private ref array<SCR_ChangeableComponentBase> components = {};
	
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		_contentArea = m_Root.FindAnyWidget("ContentArea");
		
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		_factionSpawnSettings = interface.GetFactionSpawnSettings();
		SetupFactionEnabled();
		SetupFactionCount();
		SetupWanderChance();
		
		foreach(PrefabItemChance prefab : _factionSpawnSettings.Characters)
		{
			Widget chanceWidget = GetGame().GetWorkspace().CreateWidgets(s_ItemChanceLayout, _contentArea);
			TW_ItemChance_MenuHandler handler = TW_ItemChance_MenuHandler.Cast(chanceWidget.FindHandler(TW_ItemChance_MenuHandler));
			handler.LinkTo(prefab);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		if(components && !components.IsEmpty())
		{
			foreach(SCR_ChangeableComponentBase comp : components)
				if(comp)
					comp.m_OnChanged.Clear();
		}
	}
	
	private SCR_FactionManager GetFactionManager()
	{
		if(factionManager) return factionManager;
		factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		return factionManager;
	}
	
	private void SetupFactionEnabled()
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(s_CheckboxLayout, _contentArea);
		
		SCR_CheckboxComponent checkbox = SCR_CheckboxComponent.Cast(layout.FindHandler(SCR_CheckboxComponent));
		
		checkbox.SetLabel("Enabled");
		checkbox.m_OnChanged.Insert(OnFactionEnabledChanged);
		checkbox.SetChecked(_factionSpawnSettings.IsEnabled);
		components.Insert(checkbox);
	}
	
	private void SetupWanderChance()
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(s_SliderLayout, _contentArea);
		
		SCR_SliderComponent slider = SCR_SliderComponent.Cast(layout.FindHandler(SCR_SliderComponent));
		
		slider.SetLabel("AI Wander Chance");
		slider.SetMax(1);
		slider.SetStep(0.05);
		slider.SetMin(0);
		slider.SetValue(_factionSpawnSettings.AIWanderChance);
		slider.m_OnChanged.Insert(OnFactionWandererChanged);
		
		components.Insert(slider);
	}
	
	private void SetupFactionCount()
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(s_SliderLayout, _contentArea);
		
		SCR_SliderComponent slider = SCR_SliderComponent.Cast(layout.FindHandler(SCR_SliderComponent));
		
		slider.SetLabel("Amount to Spawn");
		slider.SetFormatText("%1");
		slider.SetMax(100);
		slider.SetStep(1);
		slider.SetMin(0);
		slider.SetValue(_factionSpawnSettings.MaxAmount);
		slider.m_OnChanged.Insert(OnFactionCountChanged);
		
		components.Insert(slider);
	}
	
	private void OnFactionWandererChanged(SCR_SliderComponent comp, float value)
	{
		if(!_factionSpawnSettings)
		{
			Print("TrainWreck: Unable to update faction wanderer changes... no ref to settings", LogLevel.WARNING);
			return;
		}
		
		_factionSpawnSettings.AIWanderChance = value;
	}
	
	private void OnFactionEnabledChanged(SCR_CheckboxComponent comp, bool value)
	{
		if(!_factionSpawnSettings)
		{
			Print("TrainWreck: Unable to update faction enablement... no ref to settings", LogLevel.WARNING);
			return;
		}
		
		_factionSpawnSettings.IsEnabled = value;
	}
	
	private void OnFactionCountChanged(SCR_SliderComponent comp, float value)
	{
		if(!_factionSpawnSettings)
		{
			Print("TrainWreck: Unable to update faction count... no ref to settings", LogLevel.WARNING);
			return;
		}
		
		_factionSpawnSettings.MaxAmount = (int)value;
	}
};