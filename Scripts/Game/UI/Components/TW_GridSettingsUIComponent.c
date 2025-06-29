class TW_GridSettings
{
	int DistanceInChunks
	int SizeInMeters;
}

class TW_GridSettingsUIComponent : ScriptedWidgetComponent
{
	[Attribute("DistanceInChunks")]
	protected string m_sDistanceInChunks;
	protected SCR_SliderComponent m_wDistanceInChunks;
	
	[Attribute("GridSizeInMeters")]
	protected string m_sGridSizeInMeters;
	protected SCR_SliderComponent m_wGridSizeInMeters;
	
	protected Widget m_wRoot;
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		Widget distWidget = m_wRoot.FindAnyWidget(m_sDistanceInChunks);
		
		if(distWidget)
			m_wDistanceInChunks = SCR_SliderComponent.Cast(distWidget.FindHandler(SCR_SliderComponent));
		
		Widget sizeWidget = m_wRoot.FindAnyWidget(m_sGridSizeInMeters);
		
		if(sizeWidget)
			m_wGridSizeInMeters = SCR_SliderComponent.Cast(sizeWidget.FindHandler(SCR_SliderComponent));		
	}		
	
	TW_GridSettings GetGridSettings()
	{
		ref TW_GridSettings settings = new TW_GridSettings();
		
		settings.SizeInMeters = (int) m_wGridSizeInMeters.GetValue();		
		settings.DistanceInChunks = (int) m_wDistanceInChunks.GetValue();
		
		return settings;
	}
};