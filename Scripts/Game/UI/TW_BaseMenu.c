class TW_BaseMenu_Handler
{
	void ConsumeValues(typename type, notnull map<string, string> values);
}

class TW_BaseMenu : MenuBase
{
	protected static const string CONTENT_AREA = "ContentArea";
	
	protected static const string WIDGET_STRING_PREFAB = "";
	protected static const string WIDGET_INTEGER_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_FLOAT_PREFAB = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	protected static const string WIDGET_ENUM_PREFAB = "";
	protected static const string WIDGET_BOOL_PREFAB = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	protected static const string WIDGET_FLAGS_PREFAB = "";
	
	protected Widget _contentAreaWidget;
	protected Widget _rootWidget;
	
		
}