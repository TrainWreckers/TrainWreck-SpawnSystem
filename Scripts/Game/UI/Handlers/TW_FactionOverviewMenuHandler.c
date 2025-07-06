class TW_FactionOverviewMenuHandler : SCR_ScriptedWidgetComponent
{
	protected Widget m_Root;
	protected SCR_TabViewComponent tabs;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Root = w;
		
		TW_FactionOverviewSystem.InitSystem();
		
		tabs = SCR_TabViewComponent.Cast(m_Root.FindAnyWidget("Tabs").FindHandler(SCR_TabViewComponent));
		TextWidget text = TextWidget.Cast(m_Root.FindAnyWidget("FactionTitle"));
		
		ref FactionSpawnSettings settings = TW_FactionOverviewSystem.GetFactionSpawnSettings();
		
		if(settings)
		{
			SCR_FactionManager manager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			string name = manager.GetFactionByKey(settings.FactionName).GetFactionName();
			text.SetText(name);
		}
	}
	
	
};