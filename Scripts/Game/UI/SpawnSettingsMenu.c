class TW_Layout_SpawnSettings : MenuBase
{
	protected static const string CONTENT_AREA = "ContentArea";
	
	protected static const string WIDGET_STRING_PREFAB = "";
	protected static const string WIDGET_INTEGER_PREFAB = "";
	protected static const string WIDGET_FLOAT_PREFAB = "";
	protected static const string WIDGET_ENUM_PREFAB = "";
	protected static const string WIDGET_BOOL_PREFAB = "";
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