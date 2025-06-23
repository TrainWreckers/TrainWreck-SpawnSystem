/*!
	This component serves as a config for TrainWreck Spawn Settings menu elements.
	Sets the names of widgets where request handlers are attached to in the settings menu layout.
	Has to be attached at the root of the settings menu
*/
class TW_SpawnSettingsMenu_Handler : SCR_ScriptedWidgetComponent
{
	const string s_ShowDebugCheckBox = "ShowDebugCheckBox";
	const string s_SpawnIntervalInSecondsSlider = "SpawnIntervalInSecondsSlider";
	const string s_SpawnDistanceInChunksSlider = "SpawnDistanceInChunksSlider";
	const string s_SpawnGridSizeInMetersSlider = "SpawnGridSizeInMetersSlider";
	const string s_AntiSpawnDistanceInChunksSlider = "AntiSpawnDistanceInChunksSlider";
	const string s_AntiSpawnGridSizeInMetersSlider = "AntiSpawnGridSizeInMetersSlider";
	const string s_GarbageCollectionTimerInSecondsSlider = "GarbageCollectionTimerInSecondsSlider";
	const string s_MaxGroupSizeSlider = "MaxGroupSizeSlider";
	const string s_WanderIntervalInSecondsSlider = "WanderIntervalInSecondsSlider";
	const string s_ShouldSpawnUSPlayerBaseCheckBox = "ShouldSpawnUSPlayerBaseCheckBox";
	const string s_ShouldSpawnUSSRPlayerBaseCheckBox = "ShouldSpawnUSSRPlayerBaseCheckBox";
	const string s_ShouldSpawnFIAPlayerBaseCheckBox = "ShouldSpawnFIAPlayerBaseCheckBox";
	
	protected Widget m_Root;
	protected ref SpawnSettingsBase settings = new SpawnSettingsBase();
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);		
		m_Root = w;
	}
	
	private void OnSliderChanged(SCR_SliderComponent comp, float value)
	{
		string name = comp.GetRootWidget().GetName();
		
		switch(name)
		{
			case s_SpawnIntervalInSecondsSlider:
			{
				settings.SetSpawnTimerInSeconds(value);
				break;
			}
			
			case s_SpawnDistanceInChunksSlider:
			{
				settings.SetSpawnDistanceInChunks(value);
				break;
			}
			
			case s_SpawnGridSizeInMetersSlider:
			{
				settings.SetSpawnGridSize(value);
				break;
			}
			
			case s_AntiSpawnGridSizeInMetersSlider:
			{
				settings.SetAntiSpawnGridSize(value);
				break;
			}
			
			case s_AntiSpawnDistanceInChunksSlider:
			{
				settings.SetAntiSpawnDistanceInChunks(value);
				break;
			}
			
			case s_WanderIntervalInSecondsSlider:
			{
				settings.SetWanderIntervalInSeconds(value);
				break;
			}
			
			case s_GarbageCollectionTimerInSecondsSlider:
			{
				settings.SetGarbageCollectionTimerInSeconds(value);
				break;
			}
			
			case s_MaxGroupSizeSlider:
			{
				settings.SetGroupSize(value);
				break;
			}
		}
	}
		
	private void OnCheckboxChanged(SCR_CheckboxComponent comp, bool state)
	{
		string name = comp.GetRootWidget().GetName();
		
		switch(name)
		{
			case s_ShowDebugCheckBox:
			{
				settings.SetShowDebug(state);
				break;
			}
			case s_ShouldSpawnUSPlayerBaseCheckBox:
			{
				settings.SetShouldSpawnUSPlayerBase(state);
				break;
			}
			case s_ShouldSpawnUSSRPlayerBaseCheckBox:
			{
				settings.SetShouldSpawnUSSRPlayerBase(state);
				break;
			}
			
			case s_ShouldSpawnFIAPlayerBaseCheckBox:
			{
				settings.SetShouldSpawnFIAPlayerBase(state);
				break;
			}						
		}
	}
	
	void AttachToSpawnSettings(SpawnSettingsBase settings)
	{
		if(!GetGame().InPlayMode()) return;
		this.settings = settings;
		GetCheckbox_ShowDebug().m_OnChanged.Insert(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnUSPlayerBase().m_OnChanged.Insert(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnUSSRPlayerBase().m_OnChanged.Insert(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnFIAPlayerBase().m_OnChanged.Insert(OnCheckboxChanged);
		
		GetSlider_MaxGroupSize().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_SpawnDistanceInChunks().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_SpawnIntervalInSeconds().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_WanderIntervalInSeconds().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_AntiSpawnGridSizeInMeters().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_AntiSpawnDistanceInChunks().m_OnChanged.Insert(OnSliderChanged);
		GetSlider_GarbageCollectionTimerInSeconds().m_OnChanged.Insert(OnSliderChanged);
		
		// Update UI to the latest values from SpawnSettings
		GetCheckbox_ShowDebug().SetChecked(settings.ShowDebug);
		GetCheckbox_ShouldSpawnUSPlayerBase().SetChecked(settings.ShouldSpawnUSPlayerBase);
		GetCheckbox_ShouldSpawnUSSRPlayerBase().SetChecked(settings.ShouldSpawnUSSRPlayerBase);
		GetCheckbox_ShouldSpawnFIAPlayerBase().SetChecked(settings.ShouldSpawnFIAPlayerBase);
		
		GetSlider_MaxGroupSize().SetValue(settings.GroupSize);
		GetSlider_SpawnDistanceInChunks().SetValue(settings.SpawnDistanceInChunks);
		GetSlider_SpawnIntervalInSeconds().SetValue(settings.SpawnTimerInSeconds);
		GetSlider_WanderIntervalInSeconds().SetValue(settings.WanderIntervalInSeconds);
		GetSlider_AntiSpawnGridSizeInMeters().SetValue(settings.AntiSpawnGridSize);
		GetSlider_AntiSpawnDistanceInChunks().SetValue(settings.AntiSpawnDistanceInChunks);
		GetSlider_GarbageCollectionTimerInSeconds().SetValue(settings.GarbageCollectionTimerInSeconds);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if(!GetGame().InPlayMode()) return;
		SpawnSettingsBase.SaveToFile(TW_SpawnManagerBase.GetInstance().GetSettings());
		
		GetCheckbox_ShowDebug().m_OnChanged.Remove(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnUSPlayerBase().m_OnChanged.Remove(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnUSSRPlayerBase().m_OnChanged.Remove(OnCheckboxChanged);
		GetCheckbox_ShouldSpawnFIAPlayerBase().m_OnChanged.Remove(OnCheckboxChanged);
		
		GetSlider_MaxGroupSize().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_SpawnDistanceInChunks().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_SpawnIntervalInSeconds().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_WanderIntervalInSeconds().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_AntiSpawnGridSizeInMeters().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_AntiSpawnDistanceInChunks().m_OnChanged.Remove(OnSliderChanged);
		GetSlider_GarbageCollectionTimerInSeconds().m_OnChanged.Remove(OnSliderChanged);
	}
	
	private SCR_CheckboxComponent GetCheckboxById(string id)
	{
		Widget temp = m_wRoot.FindAnyWidget(id);
			
		if(!temp) return null;
			
		return SCR_CheckboxComponent.Cast(temp.FindHandler(SCR_CheckboxComponent));
	}
	
	private SCR_SliderComponent GetSliderById(string id)
	{
		Widget temp = m_wRoot.FindAnyWidget(id);
		if(!temp) return null;
		return SCR_SliderComponent.Cast(temp.FindHandler(SCR_SliderComponent));
	}
	
	SCR_CheckboxComponent GetCheckbox_ShowDebug()
	{
		return GetCheckboxById(s_ShowDebugCheckBox);		
	}
		
	SCR_CheckboxComponent GetCheckbox_ShouldSpawnUSPlayerBase()
	{
		return GetCheckboxById(s_ShouldSpawnUSPlayerBaseCheckBox);
	}
	
	SCR_CheckboxComponent GetCheckbox_ShouldSpawnUSSRPlayerBase()
	{
		return GetCheckboxById(s_ShouldSpawnUSSRPlayerBaseCheckBox);
	}
	
	SCR_CheckboxComponent GetCheckbox_ShouldSpawnFIAPlayerBase()
	{
		return GetCheckboxById(s_ShouldSpawnFIAPlayerBaseCheckBox);
	}
	
	SCR_SliderComponent GetSlider_SpawnIntervalInSeconds()
	{
		return GetSliderById(s_SpawnIntervalInSecondsSlider);
	}
	
	SCR_SliderComponent GetSlider_SpawnDistanceInChunks()
	{
		return GetSliderById(s_SpawnDistanceInChunksSlider);
	}
	
	SCR_SliderComponent GetSlider_AntiSpawnDistanceInChunks()
	{
		return GetSliderById(s_AntiSpawnDistanceInChunksSlider);
	}
	
	SCR_SliderComponent GetSlider_AntiSpawnGridSizeInMeters()
	{
		return GetSliderById(s_AntiSpawnGridSizeInMetersSlider);
	}
	
	SCR_SliderComponent GetSlider_GarbageCollectionTimerInSeconds()
	{
		return GetSliderById(s_GarbageCollectionTimerInSecondsSlider);
	}
	
	SCR_SliderComponent GetSlider_MaxGroupSize()
	{
		return GetSliderById(s_MaxGroupSizeSlider);
	}
	
	SCR_SliderComponent GetSlider_WanderIntervalInSeconds()
	{
		return GetSliderById(s_WanderIntervalInSecondsSlider);
	}
};