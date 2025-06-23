/*!
	This component serves as a config for TrainWreck Spawn Settings menu elements.
	Sets the names of widgets where request handlers are attached to in the settings menu layout.
	Has to be attached at the root of the settings menu
*/
class TW_FactionSpawnSettings_MenuHandler : SCR_ScriptedWidgetComponent
{
	const string s_FactionSettingsLayout = "{C9D1F6372109FC77}UI/FactionSettings.Layout.layout";
	const string s_ItemChanceLayout = "{95C8F2B268AEF55D}UI/ItemChance.layout";
	const string s_CheckboxLayout = "{5D5055E10FD00549}UI/layouts/WidgetLibrary/ToolBoxes/WLib_Checkbox.layout";
	const string s_SliderLayout = "{4A41296C0E9A889F}UI/layouts/WidgetLibrary/WLib_Slider.layout";
	
	const string s_ContentArea = "ContentArea";
	const string s_SettingsArea = "SettingsPane";
	const string s_SaveButton = "SaveButton";
	const string s_ResetButton = "ResetButton";
	
	protected Widget m_Root;
	protected Widget m_ContentArea;
	protected Widget m_SettingsArea;
	protected string m_SelectedFactionId = string.Empty;
	
	protected ref SpawnSettingsBase settings = new SpawnSettingsBase();
	protected ref FactionSpawnSettings factionSpawnSettings;
	
	protected SCR_InputButtonComponent saveButton;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Root = w;
		m_ContentArea = m_Root.FindAnyWidget(s_ContentArea);
		m_SettingsArea = m_Root.FindAnyWidget(s_SettingsArea);
		
		Widget save = m_Root.FindAnyWidget(s_SaveButton);
		if(save)
		{
			saveButton = SCR_InputButtonComponent.Cast(save.FindHandler(SCR_InputButtonComponent));
			saveButton.m_OnClicked.Insert(SaveAll);
		}
		
		Widget reset = m_Root.FindAnyWidget(s_ResetButton);
		if(reset)
		{
			SCR_InputButtonComponent resetButton = SCR_InputButtonComponent.Cast(reset.FindHandler(SCR_InputButtonComponent));
			resetButton.m_OnClicked.Insert(Reset);
		}
	}
	
	private void Reset()
	{
		Print("TrainWreck: Resetting Spawn Settings");
		settings = SpawnSettingsBase.GetDefault();
		SpawnSettingsBase.SaveToFile(settings);
		TW_SpawnManagerBase.GetInstance().LoadSettingsFromFile(settings);
		Initialize(settings);
	}
	
	private void SaveAll()
	{
		Print("TrainWreck: Saving Train Wreck Settings");
		SpawnSettingsBase.SaveToFile(settings);
		
		TW_SpawnManagerBase.GetInstance().LoadSettingsFromFile();
		
		GetGame().GetWorkspace().RemoveChild(m_Root);
	}
	
	private void SetupFactionEnabled()
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(s_CheckboxLayout, m_SettingsArea);
		
		SCR_CheckboxComponent checkbox = SCR_CheckboxComponent.Cast(layout.FindHandler(SCR_CheckboxComponent));
		
		checkbox.SetLabel("Enabled");
		checkbox.m_OnChanged.Insert(OnFactionEnabledChange);
		checkbox.SetChecked(factionSpawnSettings.IsEnabled);
	}
	
	private void SetupFactionCount()
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(s_SliderLayout, m_SettingsArea);
		
		SCR_SliderComponent slider = SCR_SliderComponent.Cast(layout.FindHandler(SCR_SliderComponent));
		
		slider.SetLabel("Amount to Spawn");
		slider.SetFormatText("%1");
		
		slider.SetMax(100);
		slider.SetStep(1);
		slider.SetMin(0);
		slider.SetValue(factionSpawnSettings.MaxAmount);
		
		slider.m_OnChanged.Insert(OnFactionCountChanged);
	}
	
	private void OnFactionCountChanged(SCR_SliderComponent comp, float value)
	{
		if(!factionSpawnSettings)
		{
			Print("TrainWreck: Unable to update faction count... no ref to settings", LogLevel.WARNING);
			return;
		}
		
		factionSpawnSettings.MaxAmount = (int)value;
	}
	
	private void OnFactionEnabledChange(SCR_CheckboxComponent comp, bool value)
	{
		if(!factionSpawnSettings)
		{
			Print("TrainWreck: Unable to update faction enablement... no ref to settings", LogLevel.WARNING);
			return;
		}
		factionSpawnSettings.IsEnabled = value;
	}
	
	void Initialize(SpawnSettingsBase settings)
	{
		this.settings = settings;
		
		ClearSettingsPane();
		Widget child = m_ContentArea.GetChildren();
		
		while(child)
		{
			m_ContentArea.RemoveChild(child);
			child = m_ContentArea.GetChildren();
		}
		
		foreach(FactionSpawnSettings factionSettings : settings.FactionSettings)
		{
			Widget factionWidget =  GetGame().GetWorkspace().CreateWidgets(s_FactionSettingsLayout);
			m_ContentArea.AddChild(factionWidget);
			
			TW_FactionSettings_MenuHandler handler = TW_FactionSettings_MenuHandler.Cast(factionWidget.FindHandler(TW_FactionSettings_MenuHandler));
			
			handler.Initialize(settings, factionSettings);
			handler.GetFactionButtonWidget().m_OnClicked.Insert(OnFactionButtonClicked);
		}
	}
	
	void OnFactionButtonClicked(SCR_ButtonImageComponent comp)
	{
		string id = comp.GetId();
		
		if(id == m_SelectedFactionId) return;
		ClearSettingsPane();
		
		FactionSpawnSettings faction = SelectFaction(id);
		
		if(!faction || !faction.Characters) return;
		
		SetupFactionEnabled();
		SetupFactionCount();
		
		foreach(PrefabItemChance chance : faction.Characters)
		{
			Widget chanceWidget = GetGame().GetWorkspace().CreateWidgets(s_ItemChanceLayout);
			m_SettingsArea.AddChild(chanceWidget);
			
			TW_ItemChance_MenuHandler handler = TW_ItemChance_MenuHandler.Cast(chanceWidget.FindHandler(TW_ItemChance_MenuHandler));
			handler.LinkTo(chance);
		}
		
	}
	
	private ref FactionSpawnSettings SelectFaction(string id)
	{
		m_SelectedFactionId = id;
		
		foreach(FactionSpawnSettings factionSettings : settings.FactionSettings)
		{
			if(factionSettings.FactionName != id) continue;
			factionSpawnSettings = factionSettings;
			return factionSettings;
		}
		
		return null;
	}
	
	private void ClearSettingsPane()
	{
		Widget child = m_SettingsArea.GetChildren();
		
		while(child)
		{
			m_SettingsArea.RemoveChild(child);
			child = m_SettingsArea.GetChildren();
		}
	}
	
	void AttachToSpawnSettings(SpawnSettingsBase settings)
	{
		if(!GetGame().InPlayMode()) return;
		this.settings = settings;
	}
	
	override void HandlerDeattached(Widget w)
	{
		if(!GetGame().InPlayMode()) return;
		SpawnSettingsBase.SaveToFile(TW_SpawnManagerBase.GetInstance().GetSettings());
	}
};