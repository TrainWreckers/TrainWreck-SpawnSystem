class TW_FactionOverview_Menu: MenuBase
{
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		
		if(!rootWidget) return;
		Widget widget = GetGame().GetWorkspace().CreateWidgets("{76F362470F718CE5}UI/layouts/FactionSettings/FactionOverview.layout", rootWidget);
	}
};