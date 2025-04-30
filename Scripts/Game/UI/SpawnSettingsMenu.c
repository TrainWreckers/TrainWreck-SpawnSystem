class TW_Layout_SpawnSettings : TW_BaseMenu
{
	private ref map<string, SCR_ChangeableComponentBase> _fieldMap = new map<string, SCR_ChangeableComponentBase>();
	
	/*!
		Should only ever need to instantiate this once during the engine's lifetime.
		Saves a bit of performance as well, since we won't have to parse the spec-file each time
		the menu is opened.
	*/
	private static ref TW_SchemaManager<SpawnSettingsBase> _spawnSettings;
	private ref SpawnSettingsBase settings = new SpawnSettingsBase();
	
	override string GetSchemaPath() { return "Schemas/TrainWreckSchema.json"; }	
	
	protected override void OnMenuOpen()
	{
		_rootWidget = GetRootWidget();
		
		// Get content area
		_contentAreaWidget = _rootWidget.FindWidget(CONTENT_AREA);
		
		if(!_spawnSettings)
		{
			_spawnSettings = new TW_SchemaManager<SpawnSettingsBase>();
			_spawnSettings.LoadSchemaFromPath(GetSchemaPath());
		}
		
		ref map<string, ref TW_SchemaBasic> classMap = _spawnSettings.GetSchemaClassMap();
		
		foreach(string varName, ref TW_SchemaBasic schema : classMap)
		{
			string prefabFor = GetWidgetTypeFor(schema.type);
			
			if(!prefabFor)
			{
				PrintFormat("TrainWreck: Prefab for type '%1' was not found. Skipping var %2", schema.type, varName, LogLevel.WARNING);
				continue;
			}
			
			Widget widgetPrefab = GetGame().GetWorkspace().CreateWidgets(prefabFor, _contentAreaWidget);
			
			if(!widgetPrefab)
			{
				PrintFormat("TrainWreck: Was unable to create widget for type '%1', %2'", schema.type, varName, LogLevel.ERROR);
				continue;
			}
			
			
		}
	}
};