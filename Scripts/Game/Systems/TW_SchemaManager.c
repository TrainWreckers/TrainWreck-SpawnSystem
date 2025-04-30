class TW_Placeholder{};

class TW_SchemaBasic
{
	string name;
	string title;
	string description;
	string type;
	string defaultValue;
	
	ref TW_SchemaUI uiInfo;
};

class TW_SchemaUI
{
	string type;
};

class TW_SchemaUI_Slider : TW_SchemaUI
{
	int min;
	int max;
	int step;
	string format;
};


class TW_SchemaManager<Class T>
{
	private ref map<string, ref TW_SchemaBasic> classMap = new map<string, ref TW_SchemaBasic>();
	
	map<string, ref TW_SchemaBasic> GetSchemaClassMap()
	{
		return classMap;
	}
	
	int GetVariableNames(notnull out array<string> names)
	{
		int i = 0, count = classMap.Count();
		foreach(string key, TW_SchemaBasic _ : classMap)
			names.Insert(key);
		return count;
	}
	
	TW_SchemaBasic GetSchemaForVariable(string variableName)
	{
		if(!classMap.Contains(variableName))
			return null;
		
		return classMap.Get(variableName);
	}
	
	private void AnalyzeClass()
	{
		typename t = T;
		int varCount = t.GetVariableCount();
		
		PrintFormat("TrainWreck: T -> %1 -> %2", t, varCount);
		for(int i = 0; i < varCount; i++)
		{
			string varname = t.GetVariableName(i);
			classMap.Set(varname, null);
		}
	}
	
	void LoadSchemaFromPath(string path)
	{
		AnalyzeClass();
		
		V30_Json_FileDeserializer serializer = new V30_Json_FileDeserializer(path);
		serializer.Open(path);
		
		if(!serializer.IsOpen())
		{
			PrintFormat("TrainWreck: Failed to open %1", path, LogLevel.ERROR);
			return;
		}
		
		ref V30_Json_Value root = serializer.Deserialize();
		
		ref V30_Json_object rootObject = root.AsObject();
		ref V30_Json_string title = rootObject.GetAt("title").AsString();		
		ref V30_Json_string description = rootObject.GetAt("description").AsString();
		
		PrintFormat("TrainWreck: loading schema\n\tTitle: %1\n\tDesc: %2", title.Get(), description.Get());
		
		ref V30_Json_object propertiesValue = rootObject.GetAt("properties").AsObject();
		ref map<string, ref V30_Json_Value> properties = propertiesValue.Get();
		
		foreach(string key, V30_Json_Value value : properties)
		{
			V30_Json_object fieldObject = value.AsObject();
			
			ref map<string, ref V30_Json_Value> fieldProperties = fieldObject.Get();
			
			ref TW_SchemaBasic schema = new TW_SchemaBasic();
			
			schema.title = fieldProperties.Get("title").AsString().Get();
			schema.description = fieldProperties.Get("description").AsString().Get();
			schema.type = fieldProperties.Get("type").AsString().Get();
			
			if(fieldProperties.Contains("default"))
				schema.defaultValue = fieldProperties.Get("default").AsString().Get();
			
			if(fieldProperties.Contains("ui"))
			{
				V30_Json_object uiObject = fieldProperties.Get("ui").AsObject();
				ref map<string, ref V30_Json_Value> uiProps = uiObject.Get();
				
				string uiType = uiProps.Get("type").AsString().Get();
				uiType.ToLower();
				
				if(uiType == "slider")
				{
					ref TW_SchemaUI_Slider slider = new TW_SchemaUI_Slider();
					
					slider.type = uiType;
					
					if(uiProps.Contains("min"))
						slider.min = uiProps.Get("min").AsInt().Get();
					else 
						slider.min = 0;
					
					if(uiProps.Contains("max"))
						slider.max = uiProps.Get("max").AsInt().Get();
					else 
						slider.max = int.MAX;
					
					if(uiProps.Contains("step"))
						slider.step = uiProps.Get("step").AsInt().Get();
					else 
						slider.step = 1;
					
					if(uiProps.Contains("format"))
						slider.format = uiProps.Get("format").AsString().Get();
					else 
						slider.format = "%1";
					
					schema.uiInfo = slider;
				}
			}
			
			classMap.Set(key, schema);
		}		
	}
};