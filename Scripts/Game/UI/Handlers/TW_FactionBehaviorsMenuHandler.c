class TW_FactionBehaviorsMenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_SliderLayout = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected Widget m_Root;
	protected Widget m_Content;
	
	protected ref FactionSpawnSettings _factionSpawnSettings;
	protected ref array<SCR_ChangeableComponentBase> components = {};
	
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		m_Content = w.FindAnyWidget("ContentArea");
		
		ref TW_SpawnSettingsInterface interface = SpawnSettingsManager.GetInstance().GetInterface();
		_factionSpawnSettings = interface.GetFactionSpawnSettings();
		
		Initialize();
	}
	
	private void Initialize()
	{
		if(!_factionSpawnSettings)
		{
			Print("TrainWreck: No faction settings", LogLevel.WARNING);
			return;
		}
		
		if(_factionSpawnSettings.Behaviors.IsEmpty())
		{
			PrintFormat("TrainWreck: No behaviors defined for %1", _factionSpawnSettings.FactionName, LogLevel.WARNING);
			return;
		}
		
		foreach(string name, float value : _factionSpawnSettings.Behaviors)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(s_SliderLayout, m_Content);
			SCR_SliderComponent slider = SCR_SliderComponent.Cast(w.FindHandler(SCR_SliderComponent));
			
			slider.SetLabel(name);
			slider.SetFormatText("%1 (Weight)");
			slider.SetMax(100);
			slider.SetMin(0);
			slider.SetStep(5);		
			slider.SetValue(value);
			slider.m_OnChanged.Insert(OnBehaviorChanged);
		}
	}
	
	private void OnBehaviorChanged(SCR_SliderComponent slider, float value)
	{
		if(_factionSpawnSettings.Behaviors.Contains(slider.GetLabel().GetText()))
		{
			_factionSpawnSettings.Behaviors.Set(slider.GetLabel().GetText(), value);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		if(components && !components.IsEmpty())
		{
			foreach(SCR_ChangeableComponentBase comp : components)
				comp.m_OnChanged.Clear();
		}
	}
};