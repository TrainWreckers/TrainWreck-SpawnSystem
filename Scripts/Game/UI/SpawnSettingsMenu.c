class TW_Layout_SpawnSettings : MenuBase
{
	protected static const string CONTENT_AREA = "ContentArea";
	
	protected static const string WIDGET_STRING_PREFAB = "";
	protected static const string WIDGET_INTEGER_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_FLOAT_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_ENUM_PREFAB = "";
	protected static const string WIDGET_BOOL_PREFAB = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	protected static const string WIDGET_FLAGS_PREFAB = "";
	
	private Widget _contentAreaWidget;
	private Widget _rootWidget;
	
	private ref map<string, SCR_ChangeableComponentBase> _fieldMap = new map<string, SCR_ChangeableComponentBase>();
	
	protected override void OnMenuOpen()
	{
		_rootWidget = GetRootWidget();
		
		// Get content area
		_contentAreaWidget = _rootWidget.FindWidget(CONTENT_AREA);
		
		
	}
};