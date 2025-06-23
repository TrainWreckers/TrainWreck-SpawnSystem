class TW_SpawnSettings_BaseAttribute : SCR_BaseValueListEditorAttribute
{
	static SpawnSettingsBase GetSettings()
	{
		TW_SpawnManagerBase manager = TW_SpawnManagerBase.GetInstance();
		
		if(!manager) return null;
		
		return manager.GetSettings();
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SpawnIntervalInSecondsAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
		{
			Print("TrainWreck: Unable to read/set Spawn Interval in seconds. SpawnSettingsBase is null", LogLevel.ERROR);
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		}
		
		return SCR_BaseEditorAttributeVar.CreateInt(spawnSettings.SpawnTimerInSeconds);
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		int seconds = var.GetInt();
		
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return;
		
		spawnSettings.SetSpawnTimerInSeconds(seconds);
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_ShowDebugAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return SCR_BaseEditorAttributeVar.CreateBool(false);
		
		return SCR_BaseEditorAttributeVar.CreateBool(spawnSettings.ShowDebug);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		bool value = var.GetBool();
		
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return;
		
		spawnSettings.SetDebug(value);
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetSpawnDistanceInSecondsAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(spawnSettings.SpawnDistanceInChunks);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return;
		
		spawnSettings.SetSpawnDistanceInChunks(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetSpawnGridSizeAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(settings.SpawnGridSize);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings) return;
		settings.SetSpawnGridSize(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetAntiSpawnDistanceInChunksAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(spawnSettings.AntiSpawnDistanceInChunks);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase spawnSettings = TW_SpawnSettings_BaseAttribute.GetSettings();
		
		if(!spawnSettings)
			return;
		
		spawnSettings.SetAntiSpawnDistanceInChunks(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetAntiSpawnGridSizeAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(settings.AntiSpawnGridSize);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings) return;
		settings.SetAntiSpawnGridSize(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetGarbageCollectionTimerAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(settings.GarbageCollectionTimerInSeconds);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings) return;
		settings.SetGarbageCollectionTimerInSeconds(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_SetGroupSizeAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(settings.GroupSize);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings) return;
		settings.SetGroupSize(var.GetInt());
	}
}

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class TW_SpawnSettings_WanderIntervalInSecondsAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings)
			return SCR_BaseEditorAttributeVar.CreateInt(0);
		
		return SCR_BaseEditorAttributeVar.CreateInt(settings.WanderIntervalInSeconds);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SpawnSettingsBase settings = TW_SpawnSettings_BaseAttribute.GetSettings();
		if(!settings) return;
		settings.SetWanderIntervalInSeconds(var.GetInt());
	}
}